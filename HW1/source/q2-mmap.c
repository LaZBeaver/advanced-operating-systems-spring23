#define _GNU_SOURCE

#include <pthread.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sched.h>

#define PAGE_SIZE (4*1024)
#define THREAD_WRITER_NUMBER 2
#define THREAD_READER_NUMBER 0

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
        write(STDERR_FILENO, "failed to open the specified file", 
        strlen("failed to open the specified file"));
        exit(EXIT_FAILURE);
    }

    if (fstat(file_dsc, &sb) < 0)
    {
        write(STDERR_FILENO, "failed to get the file stats", 
        strlen("failed to get the file stats"));
        exit(EXIT_FAILURE);
    }

    //sb.st_size should increase from 0 to page size after fallocate
    //it may require using fstat again to see the increase
    if (fallocate(file_dsc, FALLOC_FL_ZERO_RANGE, 0 , sysconf(_SC_PAGE_SIZE)) != 0)
    {
        write(STDERR_FILENO, "failed to allocate space for the file", 
        strlen("failed to allocate space for the file"));
        exit(EXIT_FAILURE);
    }
    
    
    EXCHANGE = mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, file_dsc, 0);
    if (EXCHANGE == MAP_FAILED)
    {
        write(STDERR_FILENO, "failed to map the specified descriptor", 
        strlen("failed to map the specified descriptor"));
        exit(EXIT_FAILURE);
    }
}

void *thread_remapper()
{
    int file_dsc = open("Q2_FILE2.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (file_dsc < 0)
    {
        write(STDERR_FILENO, "failed to open the specified file", 
        strlen("failed to open the specified file"));
        exit(EXIT_FAILURE);
    }

    if (fstat(file_dsc, &sb) < 0)
    {
        write(STDERR_FILENO, "failed to get the file stats", 
        strlen("failed to get the file stats"));
        exit(EXIT_FAILURE);
    }

    //sb.st_size should increase from 0 to page size after this
    //it may require using fstat again to see the increase
    if (fallocate(file_dsc, FALLOC_FL_ZERO_RANGE, 0 , sysconf(_SC_PAGE_SIZE)) != 0)
    {
        write(STDERR_FILENO, "failed to allocate space for the file", 
        strlen("failed to allocate space for the file"));
        exit(EXIT_FAILURE);
    }
    
    pthread_barrier_wait(&barrier);
    
    EXCHANGE = mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, file_dsc, 0);
    if (EXCHANGE == MAP_FAILED)
    {
        write(STDERR_FILENO, "failed to map the specified descriptor", 
        strlen("failed to map the specified descriptor"));
        exit(EXIT_FAILURE);
    }

    /*
    if (munmap(EXCHANGE, sysconf(_SC_PAGE_SIZE)) < 0)
    {
        write(STDERR_FILENO, "failed to unmap the specified region", 
        strlen("failed to unmap the specified region"));
        exit(EXIT_FAILURE);        
    }
    */
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
       
    pthread_barrier_wait(&barrier);

    for (size_t i = 0; i < PAGE_SIZE; i++)
    {
        EXCHANGE[i] = wr;
    }
}

void handler(int sig)
{
    write(2, "SIGSEGV Occured!\n", strlen("SIGSEGV Occured!\n"));
    exit(EXIT_FAILURE);
}

int main()
{
    /*
    cpu_set_t sets [8];
    for (size_t i = 0; i < 8; i++)
    {
        CPU_ZERO(&sets[i]);
        CPU_SET(i, &sets[i]);
    }
    */

    //printf("Number of cpu cores %lu\n", sysconf(_SC_NPROCESSORS_CONF));
    //printf("Number of online cpu cores %lu\n", sysconf(_SC_NPROCESSORS_ONLN));

    struct sigaction sa;
    memset (&sa, '\0', sizeof(sa));
    sa.sa_sigaction = (void*)handler;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &sa, NULL);
    
    if (pthread_barrier_init(&barrier, NULL, THREAD_READER_NUMBER+THREAD_WRITER_NUMBER+1) != 0)
    {
        write(STDERR_FILENO, "failed to initiate the barrier", 
        strlen("failed to initiate the barrier"));
        exit(EXIT_FAILURE);
    }

    pthread_t tid_mapper;
    pthread_t tid_remapper;

    pthread_t tid_readers [THREAD_READER_NUMBER];
    pthread_t tid_writers [THREAD_WRITER_NUMBER];

    //---------------------------------------------------------

    if (pthread_create(&tid_mapper, NULL, thread_mapper, NULL) != 0)
    {
        write(STDERR_FILENO, "failed to create the thread", 
        strlen("failed to create the thread"));
        exit(EXIT_FAILURE);
    }

    if (pthread_join(tid_mapper, NULL) != 0)
    {
        write(STDERR_FILENO, "failed to join the thread", 
        strlen("failed to join the thread"));
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&tid_remapper, NULL, thread_remapper, NULL) != 0)
    {
        write(STDERR_FILENO, "failed to create the thread", 
        strlen("failed to create the thread"));
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < THREAD_READER_NUMBER; i++)
    {
        if (pthread_create(&tid_readers[i], NULL, thread_reader, NULL) != 0)
        {
            write(STDERR_FILENO, "failed to create the thread", 
            strlen("failed to create the thread"));
            exit(EXIT_FAILURE);
        }

        //sched_setaffinity(tid_readers[i], sizeof(sets[i]), &sets[i]);
    }

    for (size_t i = 0; i < THREAD_WRITER_NUMBER; i++)
    {
        if (pthread_create(&tid_writers[i], NULL, thread_writer, NULL) != 0)
        {
            write(STDERR_FILENO, "failed to create the thread", 
            strlen("failed to create the thread"));
            exit(EXIT_FAILURE);
        }

        //sched_setaffinity(tid_readers[i+THREAD_READER_NUMBER], sizeof(sets[i+THREAD_READER_NUMBER]), &sets[i+THREAD_READER_NUMBER]);
    }
    
    //---------------------------------------------------------

    if (pthread_join(tid_remapper, NULL) != 0)
    {
        write(STDERR_FILENO, "failed to join the thread", 
        strlen("failed to join the thread"));
        exit(EXIT_FAILURE);
    }


    for (size_t i = 0; i < THREAD_READER_NUMBER; i++)
    {
        if (pthread_join(tid_readers[i], NULL) != 0)
        {
            write(STDERR_FILENO, "failed to join the thread", 
            strlen("failed to join the thread"));
            exit(EXIT_FAILURE);
        }
    }

    for (size_t i = 0; i < THREAD_WRITER_NUMBER; i++)
    {
        if (pthread_join(tid_writers[i], NULL) != 0)
        {
            write(STDERR_FILENO, "failed to join the thread", 
            strlen("failed to join the thread"));
            exit(EXIT_FAILURE);
        }
        
    }
    
    //---------------------------------------------------------

    pthread_barrier_destroy(&barrier);
    exit(EXIT_SUCCESS);
}

