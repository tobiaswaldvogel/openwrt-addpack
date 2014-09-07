/*
 * Another way to manage AIO writes with Samba. To maximize write throughput
 * VFS function recvfile must be working for SMB2. See bug #9412 for more 
 * info about how samba4 is progressing on this.
 *
 * Jose M. Prieto <priejos@gmail.com>
 * 
 * Based on work done by:
 * - Giuseppe De Robertis for ProFTPD
 * - Tobias Waldvogel for vsftpd
 */

#include "includes.h"
#include "system/filesys.h"
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <libaio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

/*
 * defaults
 */
#define DFL_BLOCK_SIZE			(64 * 1024)
#define DFL_BLOCK_NUMS			128
#define DFL_MAX_EVENTS			16

/*
 * general macros
 */
#define DAIO_SECTOR_SIZE			getpagesize()
#define DAIO_MIN(x,y)				((x) < (y) ? (x) : (y))
#define DAIO_ROUNDDOWN_SECTOR(x,p)	((x) & ~((p)-1))
#define DAIO_ROUNDUP_SECTOR(x,p)	((x) + ((p)-1)) & ~((p)-1)

/*
 * debug macros
 */
#define DAIO_MSG_PREFIX			"daio_linux: "
#define DAIO_ERROR(s...)		DEBUG(0,(DAIO_MSG_PREFIX "error: " s))
#define DAIO_WARN(s...)			DEBUG(0,(DAIO_MSG_PREFIX "warning: " s))
#define DAIO_DEBUG1(s...)		DEBUG(1,(DAIO_MSG_PREFIX "debug1: " s))
#define DAIO_DEBUG2(s...)		DEBUG(2,(DAIO_MSG_PREFIX "debug2: " s))
#if 1 
#define DAIO_DEBUG5(s...)		DEBUG(5,(DAIO_MSG_PREFIX "debug5: " s))
#else
#define DAIO_DEBUG5(s...)		 
#endif

/*
 * block control macros
 */
#define BLKCTL_FLD(r)			((r)->blk_ctrl)
#define BLKCTL_ITEM(r,i)		(BLKCTL_FLD(r)[(i)])
#define BLKCTL_INDEX(r,i)		(BLKCTL_ITEM(r,i).index)
#define BLKCTL_FILE(r,i)		(BLKCTL_ITEM(r,i).file)
#define BLKCTL_OFS(r,i)			(BLKCTL_ITEM(r,i).offset)
#define BLKCTL_SIZE(r,i)		(BLKCTL_ITEM(r,i).size)
#define BLKCTL_IO(r,i)			(BLKCTL_ITEM(r,i).iocb_ctrl)
#define BLKCTL_IO_BLKCTL(r,i)	(BLKCTL_IO(r,i).blk_ctrl)
#define BLKCTL_IO_MEMPTR(r,i)	(BLKCTL_IO(r,i).mem_start)
#define BLKCTL_IO_IOCB(r,i)		(BLKCTL_IO(r,i).iocb)

/* 
 * block bitmap macros
 */
#define BLKMAP_SIZE				256
#define BLKMAP_VECTORS			8
typedef uint32_t				daio_bm_t;
#define BLKMAP_VC_SIZE			32
#define BLKMAP_VC_SET(m,n)		((m) |= ((daio_bm_t)1 << (n)))
#define BLKMAP_VC_GET(m,n)		((bool)(((m) >> (n)) & 1))
#define BLKMAP_VC_CLR(m,n)		((m) &= ~((daio_bm_t)1 << (n)))
#define BLKMAP_VC_FFZ(m)		(__builtin_ffs(~(m))-1)
#define BLKMAP_IDX(n)			((int)(n)/BLKMAP_VC_SIZE)
#define BLKMAP_FLD(r)			((r)->blk_map)
#define BLKMAP_SET(r,n) \
			BLKMAP_VC_SET(BLKMAP_FLD(r)[BLKMAP_IDX(n)],(n)%BLKMAP_VC_SIZE)
#define BLKMAP_GET(r,n) \
			BLKMAP_VC_GET(BLKMAP_FLD(r)[BLKMAP_IDX(n)],(n)%BLKMAP_VC_SIZE)
#define BLKMAP_CLR(r,n) \
			BLKMAP_VC_CLR(BLKMAP_FLD(r)[BLKMAP_IDX(n)],(n)%BLKMAP_VC_SIZE)
static inline __blkmap_ffz(daio_bm_t *map)
{
	int i, ffz;

	for (i = 0; i < BLKMAP_VECTORS; i++) {
		ffz = BLKMAP_VC_FFZ(map[i]);
		if (ffz != -1)
			return ffz + BLKMAP_VC_SIZE*i;
	}

	return -1;
}
#define BLKMAP_FFZ(r)			__blkmap_ffz(BLKMAP_FLD(r))


/*
 * Direct AIO (daio) structs and types
 */
typedef struct iocb_ctrl_struct iocb_ctrl_t;
typedef struct daio_blk_ctrl_struct daio_blk_ctrl_t;
typedef struct daio_file_struct daio_file_t;
typedef struct daio_struct daio_t;

struct daio_file_struct {
	daio_file_t		*prev, *next;
	files_struct	*fsp;		/* file_struct */
	int				requests;	/* # of ongoing AIO requests */
	int				new_flags;	/* new status flags */
	int				old_flags;	/* old status flags */
};

struct iocb_ctrl_struct {
	daio_blk_ctrl_t		*blk_ctrl;
	unsigned char		*mem_start;
	struct iocb			iocb;		
};

struct daio_blk_ctrl_struct {
	int				index;		/* block index in ring */
	daio_file_t		*file;		/* target file */
	off_t			offset;		/* offset at data must be written to */
	size_t			size;		/* size of filled space in block */
	iocb_ctrl_t		iocb_ctrl;	/* AIO control struct */
};

struct daio_struct {
	unsigned char	*buffer;	/* allocate memory for buffer ring */
	size_t			size;		/* total ring size */
	size_t			pageSize;	/* page size */
	size_t			blockSize;	/* size of one block */
	int				blockNum;	/* # of blocks in ring */
	int				maxEvents;	/* AIO max events */
	daio_blk_ctrl_t	*blk_ctrl;	/* block control array */
	daio_bm_t		blk_map[BLKMAP_VECTORS];
								/* free/used block 256-bits bitmap */
	io_context_t	ctx;		/* AIO context */
	daio_file_t		*files;		/* daio file list */
};


/*
 * internal module functions
 */
static int daio_init(vfs_handle_struct *handle,
				size_t blockSize, int blockNum, int maxEvents)
{
	daio_t *daioData;
	daio_blk_ctrl_t *blk_ctrl;
	int i, ret;
		
	/* init buffer ring struct */
	daioData = talloc_zero(handle->conn, daio_t);
	if (daioData == NULL) {
		errno = ENOMEM;
		DAIO_ERROR("daio_init: talloc_zero failed "
			"for struct daio_t [errno=%d]\n", errno);
		goto daio_init_error;
	}

	daioData->pageSize = DAIO_SECTOR_SIZE;
	daioData->blockSize = DAIO_ROUNDUP_SECTOR(blockSize, daioData->pageSize);
	daioData->blockNum = blockNum;
	daioData->maxEvents = maxEvents;
	daioData->size = blockSize*blockNum;
	daioData->files = NULL;
	
	/* initialize block bitmap: all blocks free */
	memset(daioData->blk_map, 0, sizeof(daio_bm_t)*BLKMAP_VECTORS);
	
	/*
	 * buffer ring itself must be allocated via posix_memalign 
	 * so that memory space is aligned to pages (requirement for
	 * using O_DIRECT)
	 */
	ret = posix_memalign((void **) &daioData->buffer, DAIO_SECTOR_SIZE, 
							blockSize*blockNum);
	if (ret!=0){
		errno = ret;
		DAIO_ERROR("daio_init: posix_memalign failed [errno=%d]\n",errno);
		goto daio_init_error;
	}

	/* alloc the daio block control array */
	daioData->blk_ctrl = talloc_zero_array(handle->conn,
							daio_blk_ctrl_t, blockNum);
	if (daioData->blk_ctrl == NULL){
		errno = ENOMEM;
		DAIO_ERROR("daio_init: talloc_zero_array failed "
			"for the block control array [errno=%d]\n", errno);
		goto daio_init_error;
	}

	/* init daio block control array */
	for (i=0; i<blockNum; i++) {
		BLKCTL_INDEX(daioData, i) = i;
		BLKCTL_IO_MEMPTR(daioData, i) = daioData->buffer + blockSize*i;
		BLKCTL_IO_BLKCTL(daioData, i) = &BLKCTL_ITEM(daioData, i);
	}
	
	/* initialize AIO state machine */
	ret = io_queue_init(blockNum, &daioData->ctx);
	if (ret < 0) {
		errno = ret*-1;
		DAIO_ERROR("io_queue_init failed [errno=%d]\n", errno);
		goto daio_init_error;
	}
	
	/* store context data into private handle section */
	SMB_VFS_HANDLE_SET_DATA(handle, daioData, NULL, daio_t, return -1);
	
	return 0;

daio_init_error:
	if (daioData) {
		if (daioData->buffer)
			free(daioData->buffer);
		if (daioData->blk_ctrl)
			TALLOC_FREE(daioData->blk_ctrl);
		TALLOC_FREE(daioData);
	}
	return -1;
}

static void daio_deinit(vfs_handle_struct *handle)
{
	int ret;
	daio_t *daioData;
	
	/* retrieve private data */
	SMB_VFS_HANDLE_GET_DATA(handle, daioData, daio_t, return);
	
	/* release the AIO queue */
	if ((ret = io_queue_release(daioData->ctx)) < 0) {
		DAIO_WARN("daio_deinit: io_queue_release failed [errno=%d]",(-1*ret));
	}
	
	/* release allocated resources */
	free(daioData->buffer);
	TALLOC_FREE(daioData->blk_ctrl);
	TALLOC_FREE(daioData);
	
	return;
}

static daio_file_t *daio_file_search_fsp(daio_t *daioData, 
			const files_struct *fsp)
{
	daio_file_t *file_p = NULL;
	
	if (!daioData->files || !fsp)
		return NULL;

	/* 
	 * try to find out whether access flags were already 
	 * set for this file descriptor
	 */
	do {
		file_p = file_p?file_p->next:daioData->files;
		if (file_p->fsp == fsp) {
			/* found! */
			return file_p;
		}
	} while (file_p->next);

	return NULL;
}

static daio_file_t *daio_file_init(daio_t *daioData, files_struct *fsp)
{
	daio_file_t *file_p;
	int ret;

	if (daioData->files) {
		file_p = daio_file_search_fsp(daioData, fsp);
		if (file_p) {
			/* file already initialized */
			DAIO_DEBUG5("daio_file_init: found file in daio list [%p]\n", file_p);
			return file_p;
		}
	}

	/*
	 * this is a new file, so initialize a new daio_file_t 
	 * struct and rebuilt list of daio files
	 */
	file_p = talloc_zero(fsp, daio_file_t);
	if (file_p == NULL) {
		errno = ENOMEM;
		DAIO_ERROR("daio_file_init: talloc_zero failed "
			"for struct daio_file_t [errno=%d]\n", errno);
		return NULL;
	}

	DAIO_DEBUG5("daio_file_init: new file into daio list [%p]\n", file_p);
	
	file_p->fsp = fsp;
	file_p->requests = 0;
	file_p->old_flags = fcntl(fsp->fh->fd, F_GETFL) & ~O_ACCMODE;
	file_p->new_flags = file_p->old_flags | O_DIRECT;
	ret = fcntl(fsp->fh->fd, F_SETFL, file_p->new_flags);
	if (ret < 0) {
		DAIO_ERROR("daio_file_init: fcntl failed [errno=%d]\n", errno);
		TALLOC_FREE(file_p);
		goto daio_file_init_end;
	}

	DAIO_DEBUG5("daio_file_init: file descriptor flags changed from "
		"%d (old) to %d (new)\n", file_p->old_flags, file_p->new_flags);

	/* set list of target files using this module */
	file_p->prev = NULL;
	file_p->next = daioData->files;
	if (daioData->files)
		daioData->files->prev = file_p;
	daioData->files = file_p;
	
daio_file_init_end:
	return file_p;
}

static int daio_wait_freeBlock(daio_t *daioData);

static int daio_file_flush(daio_t *daioData, files_struct *fsp) 
{
	int ret;
	daio_file_t *file_p;

	file_p = daio_file_search_fsp(daioData, fsp);
	if (file_p == NULL)
		return 0;

	while (file_p->requests > 0) {
		DAIO_DEBUG5("daio_file_flush: %d pending requests in AIO queue "
			"for file %s\n", file_p->requests, smb_fname_str_dbg(fsp->fsp_name));
		if ((ret = daio_wait_freeBlock(daioData)) < 0) {
			DAIO_ERROR("daio_file_flush: daio_wait_freeBlock failed "
				"[rc=%d]\n", ret);
			return ret;
		}
	}

	return 0;
}

static void daio_file_destroy(daio_t *daioData, files_struct *fsp)
{
	daio_file_t *file_p;

	file_p = daio_file_search_fsp(daioData, fsp);
	if (!file_p)
		return;

	/* rebuilt list of daio files before destruction */
	if (file_p == daioData->files) {
		/* first item */
		daioData->files = file_p->next;
		if (file_p->next)
			file_p->next->prev = NULL;
	}
	else if (!file_p->next && file_p->prev)
		/* last item */
		file_p->prev->next = NULL;
	else {
		/* any other item */
		file_p->prev->next = file_p->next;
		file_p->next->prev = file_p->prev;
	}
	
	DAIO_DEBUG5("daio_file_destroy: file destroyed [%p]\n", file_p);

	TALLOC_FREE(file_p);
	return;
}

static inline int daio_write_done(daio_t *daioData, 
				iocb_ctrl_t *iocb_ctrl, long res)
{
	int blk_index;
	daio_file_t *file_p;

	/* get block index to be released */
	blk_index = iocb_ctrl->blk_ctrl->index;
	DAIO_DEBUG5("daio_write_done: AIO write processed for block #%d\n", 
		blk_index);

	if (res != iocb_ctrl->blk_ctrl->size){
		/* return value from the write function */
		DAIO_ERROR("daio_write_done: wrote only %ld bytes but "
			"expected %lu\n", res, iocb_ctrl->blk_ctrl->size);
		return -1;
	}
	
	file_p = BLKCTL_FILE(daioData, blk_index);

	/* one AIO request off */
	file_p->requests--;
	/* release block in bitmap */
	BLKMAP_CLR(daioData, blk_index);
	
	DAIO_DEBUG5("daio_write_done: block #%d has been released and "
		"number of pending requests for %s is now %d\n",
		blk_index, smb_fname_str_dbg(file_p->fsp->fsp_name),
		file_p->requests);

	return 0;
}

static inline int daio_get_freeBlock(daio_t *daioData)
{
	int blk_index;

	blk_index = BLKMAP_FFZ(daioData);
	if (blk_index >= daioData->blockNum)
		/* no more free blocks in ring */
		return -1;

	return blk_index;
}

static int daio_wait_freeBlock(daio_t *daioData) {
	size_t n_events;
	struct io_event *event, *events;
	iocb_ctrl_t *iocb_ctrl;
	int io_ret, ret, i;

	n_events = sizeof(struct io_event)*daioData->maxEvents;
	events = (struct io_event *)malloc(n_events);
	if (events == NULL) {
		errno = ENOMEM;
		DAIO_ERROR("daio_wait_freeBlock: malloc failed for array "
			"of struct io_event [errno=%d]\n", errno);
		return -1;
	}
	
	for (;;){
		/* wait for an event */
		io_ret = io_getevents(daioData->ctx, 1, daioData->maxEvents, 
					events, NULL);
		if (io_ret < 0) {
			errno = EIO;
			ret = io_ret;
			DAIO_ERROR("daio_wait_freeBlock: io_getevents failed "
				"[rc=%d]\n", io_ret);
			goto daio_wait_freeBlock_end;
		}
		
		DAIO_DEBUG5("daio_wait_freeBlock: %d io_events\n", io_ret);

		for (i=0; i<io_ret; i++){
			event = events+i;
			iocb_ctrl = (iocb_ctrl_t *)event->data;

			if ((int)event->res < 0) {
				/* return value from the write function */
				errno = event->res * -1;
				ret = event->res;
				DAIO_ERROR("daio_wait_freeBlock: io_getevents event.res %ld\n",
					event->res);
				goto daio_wait_freeBlock_end;
			}

			if ((int)event->res2 < 0) {	
				/* aio error */
				errno = EIO;
				ret = event->res2;
				DAIO_ERROR("daio_wait_freeBlock: io_getevents event.res2 %ld\n", 
					event->res2);
				goto daio_wait_freeBlock_end;
			}

			if ((ret = daio_write_done(daioData, iocb_ctrl, event->res)) < 0)
				goto daio_wait_freeBlock_end;

			DAIO_DEBUG5("daio_wait_freeBlock: io_event #%d processed\n", i);
		}

		if ((ret = daio_get_freeBlock(daioData)) >= 0) {
			break;
		}
	}

daio_wait_freeBlock_end:
	free(events);
	return ret;
}

static int daio_submit_write(daio_t *daioData, int blk_index)
{
	struct iocb *iocb; 
	daio_file_t *file_p;
	int fd, ret;

	if (blk_index >= daioData->blockNum){
		errno = EINVAL;
		DAIO_ERROR("daio_submit_write: block index %d out of range\n", 
			blk_index);
		return -1;
	}

	file_p = BLKCTL_FILE(daioData, blk_index);

	/* file descriptor */
	fd = file_p->fsp->fh->fd;
	/* prepare AIO write */
	iocb = &BLKCTL_IO_IOCB(daioData, blk_index);
	io_prep_pwrite(iocb, fd, BLKCTL_IO_MEMPTR(daioData, blk_index), 
		BLKCTL_SIZE(daioData, blk_index), BLKCTL_OFS(daioData, blk_index));
	iocb->data = (void *) &BLKCTL_IO(daioData, blk_index);

	DAIO_DEBUG5("daio_submit_write: file descriptor is %d\n", fd);
	DAIO_DEBUG5("daio_submit_write: memory buffer is %p\n", 
		BLKCTL_IO_MEMPTR(daioData, blk_index));
	DAIO_DEBUG5("daio_submit_write: size is %lu\n", BLKCTL_SIZE(daioData, blk_index));
	DAIO_DEBUG5("daio_submit_write: offset is %ld\n", BLKCTL_OFS(daioData, blk_index));

	/* submit the write request */
	if ((ret = io_submit(daioData->ctx, 1, &iocb)) != 1) {		
		errno = ret*-1;
		DAIO_ERROR("daio_submit_write: io_submit failed [errno=%d]\n", errno);
		return -1;
	}
	
	/* increment AIO request counter on file */
	file_p->requests++;
	
	DAIO_DEBUG5("daio_submit_write: AIO write submitted for block #%d "
		"and number of pending requests for file %s is now %d\n",
		blk_index, smb_fname_str_dbg(file_p->fsp->fsp_name),
		file_p->requests);
	
	return 0;
}

static int daio_sync_write(daio_t *daioData, int blk_index) 
{
	daio_file_t *file_p;
	int fd, ret;
	unsigned char *buf;
	ssize_t wret;
	size_t n;
	off_t ofs;

	file_p = BLKCTL_FILE(daioData, blk_index);
	buf = BLKCTL_IO_MEMPTR(daioData, blk_index);
	n = BLKCTL_SIZE(daioData, blk_index);
	ofs = BLKCTL_OFS(daioData, blk_index);
	fd = file_p->fsp->fh->fd;

	DAIO_DEBUG5("daio_sync_write: file descriptor is %d\n", fd);
	DAIO_DEBUG5("daio_sync_write: memory buffer is %p\n", buf);
	DAIO_DEBUG5("daio_sync_write: size is %lu\n", n);
	DAIO_DEBUG5("daio_sync_write: offset is %ld\n", ofs);

	/* reset O_DIRECT temporarily */
	if ((ret = fcntl(fd, F_SETFL, (file_p->new_flags & ~O_DIRECT))) < 0) {
		DAIO_ERROR("daio_sync_write: can't reset O_DIRECT flag\n");
		return ret;
	}

	DAIO_DEBUG5("daio_sync_write: O_DIRECT reset for file %s\n",
		smb_fname_str_dbg(file_p->fsp->fsp_name));

	/* write block */
	do {
		wret = pwrite(fd, buf, n, ofs);
		
		if (wret == -1 && (errno == EINTR || errno == EAGAIN))
			continue;

		if (wret < 0) {
			DAIO_ERROR("daio_sync_write: pwrite failed [errno=%d]\n", errno);
			return wret;
		}

		n -= wret;
		ofs += wret;
		buf += wret;
	} while (n > 0);

	/* set O_DIRECT back */
	if ((ret = fcntl(fd, F_SETFL, file_p->new_flags)) < 0) {
		DAIO_ERROR("daio_sync_write: can't set O_DIRECT flag\n");
		return ret;
	}

	DAIO_DEBUG5("daio_sync_write: O_DIRECT set back for file %s\n",
		smb_fname_str_dbg(file_p->fsp->fsp_name));

	DAIO_DEBUG5("daio_sync_write: %ld bytes written into file %s\n",
		BLKCTL_SIZE(daioData, blk_index), 
		smb_fname_str_dbg(file_p->fsp->fsp_name));

	return 0;
}

static ssize_t daio_read_socket(int fd, unsigned char *buf, size_t n)
{
	ssize_t one_read;
	size_t tot_read;

	for (tot_read = 0; tot_read < n; ) {
		one_read = sys_read(fd, buf+tot_read, n-tot_read);
		if (one_read == 0)
			return tot_read;
		if (one_read == -1) {
			return -1;
		}
		tot_read += one_read;
	}

	return tot_read;
}


/*
 * VFS module routines
 */
static int daio_linux_connect(vfs_handle_struct *handle, const char *service,
				const char *user)
{
	size_t blockSize;	/* param "daio block size" */
	int blockNum;		/* param "daio block num" */ 
	int maxEvents;		/* param "daio max events */
	int ret;
	
	DAIO_DEBUG5("daio_linux_connect: call entry\n");
	
	/*********************************************************************
	 * Parameters:
	 *	 - "daio block size" determines the size of one block in DAIO
	 *	   buffer ring. Default: 64 KiB
	 *	 - "daio block num" determines the number of blocks in DAIO
	 *	   buffer ring. Default: 128. Max: 256.
	 *	 - "daio max events" determines max number of events to be processed
	 *	   by libaio. Default: as many as blocks.
	 *********************************************************************/
	
	blockSize = (size_t) lp_parm_int(SNUM(handle->conn), "daio_linux", 
		"daio block size", DFL_BLOCK_SIZE);
	DAIO_DEBUG5("daio_linux_connect: daio block size = %lu\n", blockSize);
	
	blockNum = lp_parm_int(SNUM(handle->conn), "daio_linux", 
		"daio block num", DFL_BLOCK_NUMS);
	DAIO_DEBUG5("daio_linux_connect: daio block num = %d\n", blockNum);
	if (blockNum < 1 || blockNum > BLKMAP_SIZE) {
		errno = EINVAL;
		DAIO_ERROR("daio_linux_connect: parameter \"daio block num\" "
			"out of range\n");
		DAIO_DEBUG5("daio_linux_connect: call exit\n");
		return -1;
	}
	
	maxEvents = lp_parm_int(SNUM(handle->conn), "daio_linux", 
		"daio max events", blockNum);
	DAIO_DEBUG5("daio_linux_connect: daio max events = %d\n", maxEvents);
		
	/* init buffer ring */
	ret = daio_init(handle, blockSize, blockNum, maxEvents); 
	if (ret < 0) {
		DAIO_ERROR("daio_linux_connect: can't initialize daio buffer ring "
			"with with %d blocks of %lu bytes each\n",
			blockNum, blockSize);
		DAIO_DEBUG5("daio_linux_connect: call exit\n");
		return ret;
	}

	DAIO_DEBUG5("daio_linux_connect: call exit\n");
	
	return SMB_VFS_NEXT_CONNECT(handle, service, user);
}

static void daio_linux_disconnect(vfs_handle_struct *handle)
{
	DAIO_DEBUG5("daio_linux_disconnect: call entry\n");
	
	SMB_VFS_NEXT_DISCONNECT(handle);
		
	/* deinitialize daio data structs */
	daio_deinit(handle);

	DAIO_DEBUG5("daio_linux_disconnect: call exit\n");
	
	return;
}

static int daio_linux_close(vfs_handle_struct *handle, files_struct *fsp)
{
	daio_t *daioData;
	
	DAIO_DEBUG5("daio_linux_close: call entry\n");
	
	/* retrieve private data */
	SMB_VFS_HANDLE_GET_DATA(handle, daioData, daio_t, 
		do {
			errno = ENOMEM;
			DAIO_ERROR("daio_linux_close: failure while getting daio data\n");
			DAIO_DEBUG5("daio_linux_close: call exit\n");
			return -1;
		} while(0));
	
	/* flush daio ring for this file */
	if (daio_file_flush(daioData, fsp) < 0) {
		DAIO_ERROR("daio_linux_close: daio_file_flush failed "
			"for filename %s\n", smb_fname_str_dbg(fsp->fsp_name));
		DAIO_DEBUG5("daio_linux_close: call exit\n");
		return -1;
	}
	
	/* remove fsp from file list if found */
	daio_file_destroy(daioData, fsp);
	
	DAIO_DEBUG5("daio_linux_close: call exit\n");
	
	return SMB_VFS_NEXT_CLOSE(handle, fsp);
}

static ssize_t daio_linux_recvfile(vfs_handle_struct *handle, int fromfd,
				files_struct *tofsp, off_t offset, size_t n)
{
	daio_t *daioData;
	daio_file_t *file;
	int blk_index, ret;
	size_t chunk_size;
	ssize_t read_size, write_size = 0;
	bool sync_write;

	DAIO_DEBUG5("daio_linux_recvfile: call entry\n");
	DAIO_DEBUG5("daio_linux_recvfile: input file is %s\n",
		smb_fname_str_dbg(tofsp->fsp_name));
	DAIO_DEBUG5("daio_linux_recvfile: input offset is %ld\n", (off_t)offset);
	DAIO_DEBUG5("daio_linux_recvfile: input size is %lu\n", n);
	
	/* retrieve private data */
	SMB_VFS_HANDLE_GET_DATA(handle, daioData, daio_t, 
		do {
			errno = ENOMEM;
			DAIO_ERROR("daio_linux_recvfile: failure while getting daio data\n");
			DAIO_DEBUG5("daio_linux_recvfile: call exit\n");
			return -1;
		} while(0));
	
	/* set file descriptor flag O_DIRECT if not done yet */	
	file = daio_file_init(daioData, tofsp);
	if (!file) {
		DAIO_ERROR("daio_linux_recvfile: daio_file_init failed for "
			"filename %s\n", smb_fname_str_dbg(tofsp->fsp_name));
		DAIO_DEBUG5("daio_linux_recvfile: call exit\n");
		return -1;
	}

	while (write_size < n) {
		/* determine next chunk size */
		chunk_size = MIN(daioData->blockSize, (n-write_size));
		/* align to platform page size */
		chunk_size = DAIO_ROUNDDOWN_SECTOR(chunk_size, daioData->pageSize);

		if (chunk_size == 0) {
			/* 
			 * can't align remaining size to page size
			 * so we need to process the write synchronously
			 */
			chunk_size = n-write_size;
			sync_write = true;
		} else {
			sync_write = false;
		}

		DAIO_DEBUG5("daio_linux_recvfile: chunk size to be processed next is "
			"%lu in %s mode\n", chunk_size, (sync_write?"sync":"async"));

		blk_index = daio_get_freeBlock(daioData);
		if (blk_index == -1) {
			/* 
			 * no free block available in ring 
			 * wait until some have been released
			 */
			blk_index = daio_wait_freeBlock(daioData);
			if (blk_index == -1) {
				/* error: no free block could be allocated */
				DAIO_ERROR("daio_linux_recvfile: no free block could be "
					"allocated after waiting for it\n");
				DAIO_DEBUG5("daio_linux_recvfile: call exit\n");
				return -1;
			}
		}
		
		DAIO_DEBUG5("daio_linux_recvfile: found free block #%d\n",
			blk_index);

		/* take block in ring --> update block bitmap */
		BLKMAP_SET(daioData, blk_index);
		DAIO_DEBUG5("daio_linux_recvfile: block #%d marked "
			"as taken\n", blk_index);
		
		/* read data from socket */
		read_size = daio_read_socket(fromfd, BLKCTL_IO_MEMPTR(daioData, blk_index), 
						chunk_size);
		if (read_size < 0) {
			/* error while reading socket */
			BLKMAP_CLR(daioData, blk_index);
			DAIO_ERROR("daio_linux_recvfile: can't read %lu bytes from "
				"socket %d [errno=%d]\n", chunk_size, fromfd, errno);
			DAIO_DEBUG5("daio_linux_recvfile: call exit\n");
			return -1;
		}

		DAIO_DEBUG5("daio_linux_recvfile: read %ld bytes from socket %d "
			"out of %lu\n", read_size, fromfd, chunk_size);

		/* update block control before AIO submit */
		BLKCTL_FILE(daioData, blk_index) = file;
		BLKCTL_OFS(daioData, blk_index) = (off_t)offset+write_size;
		BLKCTL_SIZE(daioData, blk_index) = read_size;
		
		DAIO_DEBUG5("daio_linux_recvfile: block control file sets to %s\n",
			smb_fname_str_dbg(BLKCTL_FILE(daioData, blk_index)->fsp->fsp_name));
		DAIO_DEBUG5("daio_linux_recvfile: block control offset sets to %ld\n",
			BLKCTL_OFS(daioData, blk_index));
		DAIO_DEBUG5("daio_linux_recvfile: block_control size sets to %lu\n",
			BLKCTL_SIZE(daioData, blk_index));

		if (sync_write) {
			/* sync write */
			ret = daio_sync_write(daioData, blk_index);
			BLKMAP_CLR(daioData, blk_index);
			if (ret < 0) {
				DAIO_ERROR("daio_linux_recvfile: daio_sync_write failed "
					"for block #%d [rc=%d]\n", blk_index, ret);
				DAIO_DEBUG5("daio_linux_recvfile: call exit\n");
				return -1;
			}
		} else {
			/* submit AIO write */
			if ((ret = daio_submit_write(daioData, blk_index)) < 0) {
				/* error: daio submit failed */
				BLKMAP_CLR(daioData, blk_index);
				DAIO_ERROR("daio_linux_recvfile: daio_submit_write failed "
					"for block #%d [rc=%d]\n", blk_index, ret);
				DAIO_DEBUG5("daio_linux_recvfile: call exit\n");
				return -1;
			}
		}

		/* move on loop control variables */
		write_size += read_size;
	}
	
	DAIO_DEBUG5("daio_linux_recvfile: total written bytes %ld\n", write_size);
	DAIO_DEBUG5("daio_linux_recvfile: call exit\n");
	
	return write_size;
}

static struct vfs_fn_pointers vfs_daio_linux_fns = {
	.connect_fn = daio_linux_connect,
	.disconnect_fn = daio_linux_disconnect,
	.close_fn = daio_linux_close,
	.recvfile_fn = daio_linux_recvfile,
};

NTSTATUS vfs_daio_linux_init(void)
{
	return smb_register_vfs(SMB_VFS_INTERFACE_VERSION,
				"daio_linux", &vfs_daio_linux_fns);
}

