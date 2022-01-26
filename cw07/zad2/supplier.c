#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <time.h>
#include <sys/time.h>
#include "header.h"

union semun {
    int val;
};

pid_t main_pid;

int n = -1, m = -1;

int oven_common;
int table_common;
int* oven;
int* table;
sem_t* sem0;
sem_t* sem1;
sem_t* sem2;
sem_t* sem3;
sem_t* sem4;

typedef struct {
    int pizza_type;
    int pizza_no;
} pizza_from_table;

long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    return milliseconds;
}

int pizzas_on_table_no(){
    int* sem_val = calloc(1, sizeof(int));
    sem_getvalue(sem3, sem_val);
    return TABLE_CAPACITY - *sem_val;
}

pizza_from_table take_out_the_table(int* table){

    sem_wait(sem4);
    sem_post(sem1);

    int pizza;

    int pos_on_table = table[TABLE_CAPACITY];

    for(int i = 0; i < TABLE_CAPACITY; i++){
        printf(" \n");
        if(table[(pos_on_table + i)%TABLE_CAPACITY] != -1){
            pizza = table[(pos_on_table + i)%TABLE_CAPACITY];
            table[(pos_on_table + i)%TABLE_CAPACITY] = -1;
            table[TABLE_CAPACITY] = (pos_on_table + i+1)%TABLE_CAPACITY;
            break;
        }
    }
    pizza_from_table result;
    result.pizza_type = pizza;
    result.pizza_no = pizzas_on_table_no() - 1;

    sem_post(sem3);
    sem_post(sem1);

    return result;
}
void close_sems(){
    sem_close(sem0);
    sem_close(sem1);
    sem_close(sem2);
    sem_close(sem3);
    sem_close(sem4);
}

void notify(int sig){
    close_sems();
    kill(main_pid, SIGINT);
}

void quit(int sig){
    close_sems();
    exit(0);
}

void notify_wrapper(void){
    notify(0);
}

int main(int argc, char** argv)
{
    main_pid = atoi(argv[1]);

    signal(SIGINT, notify);
    atexit(notify_wrapper);
    signal(SIGUSR1, quit);

    struct timespec sleep_time;

    table_common = shm_open(TABLE, O_RDWR, 0666);
    table = (int*)mmap(NULL, (TABLE_CAPACITY+1)*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, table_common, 0);

    sem0 = sem_open(SEM0, O_RDWR, 0666);
    sem1 = sem_open(SEM1, O_RDWR, 0666);
    sem2 = sem_open(SEM2, O_RDWR, 0666);
    sem3 = sem_open(SEM3, O_RDWR, 0666);
    sem4 = sem_open(SEM4, O_RDWR, 0666);


//supplier

    pizza_from_table pizza_info;
    int pizza_type;
    int k;
    while(1){
        //todo zabrać ze stołu
        pizza_info = take_out_the_table(table);
        pizza_type = pizza_info.pizza_type;
        k = pizza_info.pizza_no;
        printf("%d %lld Pobieram pizze %d. Liczba pizz na stole: %d.\n", getpid(), current_timestamp(), pizza_type, k);
        sleep_time.tv_sec = 4;
        sleep_time.tv_nsec = rand() % 999999999;
        nanosleep(&sleep_time, NULL);

        printf("%d %lld Dostarczam pizze %d.\n", getpid(), current_timestamp(), pizza_type);
        sleep_time.tv_sec = 4;
        sleep_time.tv_nsec = rand() % 999999999;
        nanosleep(&sleep_time, NULL);
    }

//supplier
    return 0;
}
