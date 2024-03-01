#define _GNU_SOURCE

#include <pthread.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
//#include <sched.h>
#include <stdio.h>

#define PAGE_SIZE (4*1024)
#define THREAD_WRITER_NUMBER 1
#define THREAD_READER_NUMBER 1

char *EXCHANGE = NULL;
char rd;
char wr = 'G';
pthread_barrier_t barrier;
struct stat sb;


void *thread_mapper()
{
    int file_dsc = open("Q2_FILE.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (file_dsc < 0)
    {
        perror("open: ");
        exit(EXIT_FAILURE);
    }

    if (fstat(file_dsc, &sb) < 0)
    {
        perror("fstat: ");
        exit(EXIT_FAILURE);
    }

    //sb.st_size should increase from 0 to page size after fallocate
    //it may require using fstat again to see the increase
    if (fallocate(file_dsc, FALLOC_FL_ZERO_RANGE, 0 , PAGE_SIZE) != 0)
    {
        perror("fallocate: ");
        exit(EXIT_FAILURE);
    }
    
    
    EXCHANGE = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, file_dsc, 0);
    if (EXCHANGE == MAP_FAILED)
    {
        perror("mmap: ");
        exit(EXIT_FAILURE);
    }
}


void *thread_reader()
{
    for (size_t i = 0; i < PAGE_SIZE; i++)
    {
        rd = EXCHANGE[i];
    }    

    pthread_barrier_wait(&barrier);

    for (size_t i = 0; i < PAGE_SIZE; i++)
    {
        rd = EXCHANGE[i];
    }
}

void *thread_writer()
{
    for (size_t i = 0; i < PAGE_SIZE; i++)
    {
        EXCHANGE[i] = wr;
    }
       
    if (msync(EXCHANGE, PAGE_SIZE, MS_ASYNC) < 0)
    {
        perror("msync: ");
    }
    

    pthread_barrier_wait(&barrier);

    for (size_t i = 0; i < PAGE_SIZE; i++)
    {
        EXCHANGE[i] = wr;
    }
}


int main()
{
    if (pthread_barrier_init(&barrier, NULL, THREAD_READER_NUMBER+THREAD_WRITER_NUMBER))
    {
        perror("barrier init: ");
        exit(EXIT_FAILURE);
    }

    pthread_t tid_mapper;

    pthread_t tid_readers [THREAD_READER_NUMBER];
    pthread_t tid_writers [THREAD_WRITER_NUMBER];

    //---------------------------------------------------------

    if (pthread_create(&tid_mapper, NULL, thread_mapper, NULL) != 0)
    {
        perror("create mapper: ");
        exit(EXIT_FAILURE);
    }

    if (pthread_join(tid_mapper, NULL) != 0)
    {
        perror("join mapper: ");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < THREAD_READER_NUMBER; i++)
    {
        if (pthread_create(&tid_readers[i], NULL, thread_reader, NULL) != 0)
        {
            perror("create reader: ");
            exit(EXIT_FAILURE);
        }
    }

    for (size_t i = 0; i < THREAD_WRITER_NUMBER; i++)
    {
        if (pthread_create(&tid_writers[i], NULL, thread_writer, NULL) != 0)
        {
            perror("create writer: ");
            exit(EXIT_FAILURE);
        }
    }
    
    //---------------------------------------------------------

    for (size_t i = 0; i < THREAD_READER_NUMBER; i++)
    {
        if (pthread_join(tid_readers[i], NULL) != 0)
        {
            perror("reader join: ");
            exit(EXIT_FAILURE);
        }
    }

    for (size_t i = 0; i < THREAD_WRITER_NUMBER; i++)
    {
        if (pthread_join(tid_writers[i], NULL) != 0)
        {
            perror("writer join: ");
            exit(EXIT_FAILURE);
        }
        
    }
    
    //---------------------------------------------------------

    pthread_barrier_destroy(&barrier);
    exit(EXIT_SUCCESS);
}

