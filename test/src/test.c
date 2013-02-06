#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE 1
#define _LARGEFILE64_SOURCE 1
#define _LARGE_FILES 1

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <asm/fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
	printf("blkcnt %d\n", sizeof(blkcnt_t));
	printf("samba_cv_SIZEOF_DEV_T %d\n", sizeof(dev_t));
	printf("SIZEOF_INO_T %d\n", sizeof(ino_t));
	printf("SIZEOF_OFF_T %d\n", sizeof(off_t));
	printf("SIZEOF_TIME_T %d\n", sizeof(time_t));

    return 0;
}
