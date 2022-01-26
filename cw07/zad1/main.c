#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <errno.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include "header.h"

union semun {
    int val;
};

int* cooks;
int* suppliers ;

int n = -1, m = -1;

int oven_common;
int table_common;
int sems;
int deleted = 0;

long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    return milliseconds;
}

void quit(int sig){
    for(int i = 0; i < n; i++){
        kill(cooks[i], SIGUSR2);
    }
    for(int i = 0; i < m; i++){
        kill(suppliers[i], SIGUSR2);
    }
    if(deleted == 0){
        free(cooks);
        free(suppliers);
        deleted = 1;
    }

    shmctl(oven_common, IPC_RMID, NULL);
    shmctl(table_common, IPC_RMID, NULL);

    semctl(sems, 0, IPC_RMID, NULL);
    semctl(sems, 1, IPC_RMID, NULL);
    semctl(sems, 2, IPC_RMID, NULL);
    semctl(sems, 3, IPC_RMID, NULL);
    semctl(sems, 4, IPC_RMID, NULL);

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
    signal(SIGUSR1, quit);
    atexit(quit_wrapper);


    key_t oven_key = ftok("/home", 'A');
    oven_common = shmget(oven_key, (OVEN_CAPACITY + 1) * sizeof(int), 0666 | IPC_CREAT ); //LAST PLACE STORES THE INDEX OF THE LAST EDITED PLACE

    int* oven = (int*) shmat(oven_common, NULL, 0);
    for(int i = 0; i < OVEN_CAPACITY; i++){
        oven[i] = -1;
    }
    oven[OVEN_CAPACITY] = 0;


    key_t table_key = ftok("/home", 'B');
    table_common = shmget(table_key, (TABLE_CAPACITY + 1) * sizeof(int), IPC_CREAT | 0666); //LAST PLACE STORES THE INDEX OF THE LAST EDITED PLACE

    int* table = (int*) shmat(table_common, NULL, 0);
    for(int i = 0; i < TABLE_CAPACITY; i++){
        table[i] = -1;
    }
    table[TABLE_CAPACITY] = 0;

    key_t semset_key = ftok("/home", 'C');
    int nsems = 5;
    sems = semget(semset_key, nsems, IPC_CREAT | 0666);

    union semun arg;
    arg.val = 1;
    semctl(sems, 0, SETVAL, arg);
    semctl(sems, 1, SETVAL, arg);
    arg.val = OVEN_CAPACITY;
    semctl(sems, 2, SETVAL, arg);
    arg.val = TABLE_CAPACITY;
    semctl(sems, 3, SETVAL, arg);
    arg.val = 0;
    semctl(sems, 4, SETVAL, arg);
    //system("ipcs");

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


