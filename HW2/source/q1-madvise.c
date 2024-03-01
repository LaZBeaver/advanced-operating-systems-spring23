#define _GNU_SOURCE

#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <signal.h>
//#include <string.h>


#define PAGE_SIZE (4*1024)
#define THREAD_NUMBER 2
#define NOT_A_MILLION 1000000

char *page = NULL;
char *page_aligned = NULL;


void *thread_allocator()
{
    page = (char*) malloc(PAGE_SIZE);
    if (page == NULL)
    {
        perror("malloc: ");
        exit(EXIT_FAILURE);
    }
    page_aligned = (char*) (((uintptr_t)page) & ~ (PAGE_SIZE - 1));
}

void *thread_advisor()
{
    for (size_t i = 0; i < NOT_A_MILLION; i++)
    {
        if (madvise(page_aligned, PAGE_SIZE, MADV_SEQUENTIAL) < 0)
        {
            perror("madvise: ");
            exit(EXIT_FAILURE);
        }
    }
}


int main()
{
    pthread_t tid_allocator;
    pthread_t tid_advisor[THREAD_NUMBER];

    if (pthread_create(&tid_allocator, NULL, thread_allocator, NULL) != 0)
    {
        perror("create thread allocator: ");
        exit(EXIT_FAILURE);
    }
    
    if (pthread_join(tid_allocator, NULL))
    {
        perror("join thread allocator: ");
        exit(EXIT_FAILURE);
    }
    
    for (size_t i = 0; i < THREAD_NUMBER; i++)
    {
        if (pthread_create(&tid_advisor[i], NULL, thread_advisor, NULL) != 0)
        {
            perror("create thread advisor: ");
            exit(EXIT_FAILURE);
        }
    }

    for (size_t i = 0; i < THREAD_NUMBER; i++)
    {
        if (pthread_join(tid_advisor[i], NULL) != 0)
        {
            perror("join thread advisor: ");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}





































