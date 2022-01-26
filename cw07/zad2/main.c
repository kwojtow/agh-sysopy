#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include "header.h"

union semun {
    int val;
};

int* cooks;
int* suppliers ;

int n = -1, m = -1;

int oven_common;
int table_common;
int* oven;
int* table;
int sems;
sem_t* sem0;
sem_t* sem1;
sem_t* sem2;
sem_t* sem3;
sem_t* sem4;

long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    return milliseconds;
}

void quit(int sig){
    for(int i = 0; i < n; i++){
        kill(cooks[i], SIGUSR1);
    }
    for(int i = 0; i < m; i++){
        kill(suppliers[i], SIGUSR1);
    }
    free(cooks);
    free(suppliers);

    munmap(oven, sizeof(oven_common));
    shm_unlink(OVEN);

    sem_close(sem0);
    sem_close(sem1);
    sem_close(sem2);
    sem_close(sem3);
    sem_close(sem4);

    sleep(1);

    sem_unlink(SEM0);
    sem_unlink(SEM1);
    sem_unlink(SEM2);
    sem_unlink(SEM3);
    sem_unlink(SEM4);


    exit(0);
}

void quit_wrapper(void){
    quit(0);
}

int main(int argc, char** argv)
{
    if(argc != 3){
        exit(1);
    }

    if((n = atoi(argv[1])) < 0){
        exit(2);
    }
    if((m = atoi(argv[2])) < 0){
        exit(3);
    }

    cooks = calloc(n, sizeof(int));
    suppliers = calloc(m, sizeof(int));

    signal(SIGINT, quit);
    atexit(quit_wrapper);


    oven_common = shm_open(OVEN, O_CREAT | O_RDWR, 0666);
    ftruncate(oven_common, (OVEN_CAPACITY + 1) * sizeof(int)); //LAST PLACE STORES THE INDEX OF THE LAST EDITED PLACE
    oven = (int* )mmap(NULL, (OVEN_CAPACITY + 1) * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, oven_common, 0);

    for(int i = 0; i < OVEN_CAPACITY; i++){
        oven[i] = -1;
    }
    oven[OVEN_CAPACITY] = 0;


    table_common = shm_open(TABLE, O_CREAT | O_RDWR, 0666);
    ftruncate(table_common, (TABLE_CAPACITY+1)*sizeof(int)); //LAST PLACE STORES THE INDEX OF THE LAST EDITED PLACE
    table = (int*)mmap(NULL, (TABLE_CAPACITY+1)*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, table_common, 0);

    for(int i = 0; i < TABLE_CAPACITY; i++){
        table[i] = -1;
    }
    table[TABLE_CAPACITY] = 0;



    sem0 = sem_open(SEM0, O_CREAT, 0666, 1);
    sem1 = sem_open(SEM1, O_CREAT, 0666, 1);
    sem2 = sem_open(SEM2, O_CREAT, 0666, OVEN_CAPACITY);
    sem3 = sem_open(SEM3, O_CREAT, 0666, TABLE_CAPACITY);
    sem4 = sem_open(SEM4, O_CREAT, 0666, 0);

    char buf[20];
    sprintf(buf, "%d", getpid());

    for(int i = 0; i < n; i++){
        if((cooks[i] = fork()) == 0){
            execl("./cook", "cook", buf);
        }
    }


    for(int i = 0; i < m; i++){
        if((suppliers[i] = fork()) == 0){
            execl("./supplier", "supplier", buf);
        }
    }


    while(wait(NULL));



    return 0;
}


