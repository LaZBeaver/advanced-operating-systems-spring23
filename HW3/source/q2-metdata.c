#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <string.h>
#include <linux/fs.h>
#include <time.h>


#define FILES 1000
#define FILE_SIZE 1024

void random_strnig(char* holder);
int random_id();

int main()
{
    struct timeval start_time;
    struct timeval finish_time;
    size_t sum = 0;
    size_t sum_u = 0;
    srand(time(NULL));

    char* write_buffer_align_1 = NULL;
    if (posix_memalign((void**)&write_buffer_align_1, 512, FILE_SIZE) < 0)
    {
        perror("memalign: ");
        exit(EXIT_FAILURE);
    }

    char* write_buffer_align_2 = NULL;
    if (posix_memalign((void**)&write_buffer_align_2, 512, FILE_SIZE) < 0)
    {
        perror("memalign: ");
        exit(EXIT_FAILURE);
    }

    char* read_buffer = (char*) malloc(20* sizeof(char));
    if (read_buffer == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    

    char* holder = (char*) malloc(10 * sizeof(char));
    if (holder == NULL)
    {
        perror("malloc");
        exit(EXIT_SUCCESS);
    }

    char* id = (char*) malloc(10 *sizeof(char));
    if (id == NULL)
    {
        perror("malloc");
        exit(EXIT_SUCCESS);
    }
    
    char* names[FILES];
    for (size_t i = 0; i < FILES; i++)
    {
        names[i] = (char*) malloc(10 * sizeof(char));
        if (names[i] == NULL)
        {
            perror("malloc");
            exit(EXIT_SUCCESS);
        }
        
    }
    

    //CREATING FILES
    for (size_t i = 0; i < FILES; i++)
    {
        memset(holder, 0, strlen(holder));
        random_strnig(holder);
        strcpy(names[i], holder);
        //printf("%d--", random_id());
        //sprintf(id, "%lu", i);
        //printf("%s\n", holder);

        
        int file_dsc = open(holder, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (file_dsc < 0)
        {
            perror("open: ");
            exit(EXIT_FAILURE);
        }
        if (fallocate(file_dsc, FALLOC_FL_ZERO_RANGE, 0 , FILE_SIZE) != 0)
        {
            perror("fallocate: ");
            exit(EXIT_FAILURE);
        }
        if (close(file_dsc) < 0)
        {
            perror("close");
        }
        
    }
    

    //WRITING 
    
    for (size_t i = 0; i < FILES; i++)
    {
        sprintf(id, "%d", random_id());
        strcpy(write_buffer_align_1, names[i]);
        strcpy(write_buffer_align_2, id);
        
        
        int file_dsc = open(names[i], O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (file_dsc < 0)
        {
            perror("open: ");
            exit(EXIT_FAILURE);
        }

        gettimeofday(&start_time, NULL);
        if (write(file_dsc, write_buffer_align_1, FILE_SIZE) == -1)
        {
            perror("write: ");
            exit(EXIT_FAILURE);
        }
        if (write(file_dsc, write_buffer_align_2, FILE_SIZE) == -1)
        {
            perror("write: ");
            exit(EXIT_FAILURE);
        }
        gettimeofday(&finish_time, NULL);

        if (close(file_dsc) < 0)
        {
            perror("close");
            exit(EXIT_FAILURE);
        }

        sum += (finish_time.tv_sec - start_time.tv_sec);
        sum_u += (finish_time.tv_usec - start_time.tv_usec);
    }
    
    printf("Write time secs: %lu\n", sum);
    printf("Write time micro secs: %lu\n", sum_u);
    


    //READING
    /*
    for (size_t i = 0; i < FILES; i++)
    {   
        int file_dsc = open(names[i], O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (file_dsc < 0)
        {
            perror("open: ");
            exit(EXIT_FAILURE);
        }
        gettimeofday(&start_time, NULL);
        if (read(file_dsc, read_buffer, 20) == -1)
        {          
            perror("read: ");
            exit(EXIT_FAILURE);
        }
        gettimeofday(&finish_time, NULL);
        //printf("%s\n", read_buffer);


        if (close(file_dsc) < 0)
        {
            perror("close");
            exit(EXIT_FAILURE);
        }

        sum += (finish_time.tv_sec - start_time.tv_sec);
        sum_u += (finish_time.tv_usec - start_time.tv_usec);
    }

    printf("Read time secs: %lu\n", sum);
    printf("Read time micro secs: %lu\n", sum_u);
    */

    for (size_t i = 0; i < FILES; i++)
    {
        free(names[i]);
    }
    free(holder);
    free(id);
    free(read_buffer);
    

    exit(EXIT_SUCCESS);
}


void random_strnig(char* holder)
{
    const char* append = ".txt";
    char alphabet[] = "abcdefghijklmnopqrstuvwxyz";
    int i, index;

    for (i = 0; i < 6; i++)
    {
        index = rand() % 26;
        holder[i] = alphabet[index];
    }

    strcat(holder, append);
}


int random_id()
{
    int id = rand() % 1000000;
    return id;
}