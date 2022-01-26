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
    int pizza_pos;
    int pizza_no;
} pizza_in_oven;

typedef struct {
    int pizza_type;
    int pizza_no;
} pizza_from_oven;

long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    return milliseconds;
}


int pizzas_in_oven_no(){
    int* sem_val = calloc(1, sizeof(int));
    sem_getvalue(sem2, sem_val);
    return OVEN_CAPACITY - *sem_val;
}

int pizzas_on_table_no(){
    int* sem_val = calloc(1, sizeof(int));
    sem_getvalue(sem3, sem_val);
    return TABLE_CAPACITY - *sem_val;
}

pizza_in_oven put_in_the_oven(int pizza, int* oven){

    sem_wait(sem2);
    sem_wait(sem0);

    int pos_in_oven = oven[OVEN_CAPACITY];
    int new_pizza_pos = 0;

    for(int i = 0; i < OVEN_CAPACITY; i++){
        if(oven[(pos_in_oven + i)%OVEN_CAPACITY] == -1){
            printf(" \n");
            oven[(pos_in_oven + i)%OVEN_CAPACITY] = pizza;
            new_pizza_pos = (pos_in_oven + i)%OVEN_CAPACITY;
            oven[OVEN_CAPACITY] = new_pizza_pos;
            break;
        }
    }
    pizza_in_oven result;
    result.pizza_pos = new_pizza_pos;
    result.pizza_no = pizzas_in_oven_no();

    sem_post(sem0);

    return result;
}
pizza_from_oven take_out_the_oven(int pos, int* oven){

    sem_wait(sem0);

    int pizza;

    pizza = oven[pos];
    oven[pos] = -1;

    printf(" \n");

    pizza_from_oven result;
    result.pizza_type = pizza;
    result.pizza_no = pizzas_in_oven_no() - 1;



    sem_post(sem2);
    sem_post(sem0);

    return result;
}

int put_on_the_table(int pizza, int* table){

    sem_wait(sem1);
    sem_wait(sem3);

    int pos_on_table = table[TABLE_CAPACITY];
    int new_pizza_pos = 0;

    for(int i = 0; i < TABLE_CAPACITY; i++){
        printf(" \n");
        if(table[(pos_on_table + i)%TABLE_CAPACITY] == -1){
            table[(pos_on_table + i)%TABLE_CAPACITY] = pizza;
            new_pizza_pos = (pos_on_table + i)%TABLE_CAPACITY;
            table[TABLE_CAPACITY] = new_pizza_pos;
            break;
        }
    }

    int result = pizzas_on_table_no();

    sem_post(sem1);
    sem_post(sem4);

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

    oven_common = shm_open(OVEN, O_RDWR, 0666);
    oven = (int* )mmap(NULL, (OVEN_CAPACITY + 1) * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, oven_common, 0);


    table_common = shm_open(TABLE, O_RDWR, 0666);
    table = (int*)mmap(NULL, (TABLE_CAPACITY+1)*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, table_common, 0);

    sem0 = sem_open(SEM0, O_RDWR, 0666);
    sem1 = sem_open(SEM1, O_RDWR, 0666);
    sem2 = sem_open(SEM2, O_RDWR, 0666);
    sem3 = sem_open(SEM3, O_RDWR, 0666);
    sem4 = sem_open(SEM4, O_RDWR, 0666);
//coock
    srand(time(NULL) ^ (getpid()<<16));
    struct timespec sleep_time;


    int pizza_type;
    pizza_in_oven pizza_info;
    pizza_from_oven pizza_info2;
    int pos;
    int m;
    int k;
    while(1){
        pizza_type = rand() % 10;

        printf("%d %lld Przygotowuje pizze %d.\n", getpid(), current_timestamp(), pizza_type);
        sleep_time.tv_sec = 1;
        sleep_time.tv_nsec = rand() % 999999999;
        nanosleep(&sleep_time, NULL);

        pizza_info = put_in_the_oven(pizza_type, oven);
        pos = pizza_info.pizza_pos;
        m = pizza_info.pizza_no;

        printf("%d %lld Dodałem pizze %d. Liczba pizz w piecu: %d.\n", getpid(), current_timestamp(), pizza_type, m);
        sleep_time.tv_sec = 4;
        sleep_time.tv_nsec = rand() % 999999999;
        nanosleep(&sleep_time, NULL);


        pizza_info2 = take_out_the_oven(pos, oven);
        pizza_type = pizza_info2.pizza_type;
        m = pizza_info2.pizza_no;

        k = put_on_the_table(pizza_type, table);
        printf("%d %lld Wyjmuje pizze %d. Liczba pizz w piecu: %d. Liczba pizz na stole: %d\n", getpid(), current_timestamp(), pizza_type, m, k);
    }

//cook

    return 0;
}

