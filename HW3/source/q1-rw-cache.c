#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <string.h>
#include <linux/fs.h>



#define GIGA 1000000000l
#define MEGA 1000000l
#define SEGMENTS 10000

int main()
{
    int blksize;
    struct timeval start_time;
    struct timeval finish_time;
    off_t file_size = 10 * GIGA;
    char* buffer = NULL;
    

    int fd = open("/dev/nvme0n1", O_RDONLY | O_NONBLOCK);
    if (fd < 0)
    {
        perror("open dev: ");
    }
    if (ioctl(fd, BLKSSZGET, &blksize) == -1)
    {
        perror("ioctl: ");
    }
    printf("size: %d\n", blksize);


    //USE blksize AS THE 2ND ARGUMENT
    if (posix_memalign((void**)&buffer, 512, file_size) < 0)
    {
        perror("memalign: ");
        exit(EXIT_FAILURE);
    }
    
    
    for (size_t i = 0; i < (10 * GIGA); i++)
    {
        buffer[i] = 'a';
    }
    

    int file_dsc = open("Q2_FILE.txt", O_RDWR | O_CREAT | O_TRUNC | O_DIRECT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (file_dsc < 0)
    {
        perror("open: ");
        exit(EXIT_FAILURE);
    }
    if (fallocate(file_dsc, FALLOC_FL_ZERO_RANGE, 0 , file_size) != 0)
    {
        perror("fallocate: ");
        exit(EXIT_FAILURE);
    }

   //Writing
    gettimeofday(&start_time, NULL);
    for (size_t i = 0; i < 10; i++)
    {
        if (write(file_dsc, buffer, (10 * GIGA)) == -1)
        {
            perror("write: ");
        }
        lseek(file_dsc, 0 , SEEK_SET);
    }
    gettimeofday(&finish_time, NULL);

    printf("Write, seconds : %ld -- micro seconds : %ld \n", start_time.tv_sec, start_time.tv_usec);
    printf("Write, seconds : %ld -- micro seconds : %ld \n", finish_time.tv_sec, finish_time.tv_usec);
    

    //Reading
    /*
    gettimeofday(&start_time, NULL);
    for (size_t i = 0; i < 10; i++)
    {
        if (read(file_dsc, buffer, 10*GIGA) == -1)
        {
            perror("read: ");
        }
        lseek(file_dsc, 0 , SEEK_SET);
    }
    gettimeofday(&finish_time, NULL);

    printf("Read, seconds : %ld -- micro seconds : %ld \n", start_time.tv_sec, start_time.tv_usec);
    printf("Read, seconds : %ld -- micro seconds : %ld \n", finish_time.tv_sec, finish_time.tv_usec);
    */


    free(buffer);
    close(file_dsc);
    exit(EXIT_SUCCESS);
}


























































