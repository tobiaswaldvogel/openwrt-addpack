#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <asm/fcntl.h>
//#include <fcntl.h>
#include <sys/mman.h>

#define FILEPATH "./bigfile.bin"
#define FILESIZE (1024*1024*1024)
#define BUFFER_SIZE (1024*1024)
// #define BUFFER_SIZE (64*1024)


int main(int argc, char *argv[])
{
    int i;
    int fd;
    int result;
	unsigned int len = BUFFER_SIZE;
	unsigned char large_buffer[BUFFER_SIZE+512], *buffer;

	if (argc <2){
		printf("Use: %s <filename>\n", argv[0]);
		exit(0);
	}

	buffer = (unsigned char*) ((((unsigned int) large_buffer) + 512) & ~0x1ff);

    /* Open a file for writing.
     *  - Creating the file if it doesn't exist.
     *  - Truncating it to 0 size if it already exists. (not really needed)
     *
     * Note: "O_WRONLY" mode is not sufficient when mmaping.
     */
    fd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC | O_DIRECT, (mode_t)0600);
    if (fd == -1) {
		perror("Error opening file for writing");
		exit(EXIT_FAILURE);
    }
	printf("File creato OK\n");

	if (posix_fallocate(fd, 0, FILESIZE)<0){
		perror("Error calling posix_fallocate()");
		exit(EXIT_FAILURE);
	}
	printf("posix_fallocate OK\n");

#if 0
    /* Stretch the file size to the size of the (mmapped) array of ints
     */
    result = lseek(fd, FILESIZE, SEEK_SET);
    if (result == -1) {
		close(fd);
		perror("Error calling lseek() to 'stretch' the file");
		exit(EXIT_FAILURE);
    }
	printf("seek ok\n");
    
    /* Something needs to be written at the end of the file to
     * have the file actually have the new size.
     * Just writing an empty string at the current file position will do.
     *
     * Note:
     *  - The current position in the file is at the end of the stretched 
     *    file due to the call to lseek().
     *  - An empty string is actually a single '\0' character, so a zero-byte
     *    will be written at the last byte of the file.
     */
    result = write(fd, buffer, 512);
    if (result<0) {
		perror("Error writing last byte of the file");
		exit(EXIT_FAILURE);
    }
	printf("File esteso alla dimensione\n");

	/* ritorna all'inizio del file */
    result = lseek(fd, 0, SEEK_SET);
    if (result == -1) {
		close(fd);
		perror("Error calling lseek() to rewind the file");
		exit(EXIT_FAILURE);
    }
#endif

	bzero(buffer, sizeof(buffer));

	for (i=0; i<(FILESIZE/BUFFER_SIZE); i++){
		buffer[0] = i;
		if (write(fd, buffer, BUFFER_SIZE)!=BUFFER_SIZE){
			perror("Error calling write()");
			exit(EXIT_FAILURE);
		}
	}

	munmap(buffer, len);
    close(fd);
    return 0;
}
