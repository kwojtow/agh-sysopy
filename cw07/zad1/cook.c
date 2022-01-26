#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include "header.h"

union semun {
    int val;
};

pid_t main_pid;

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


int pizzas_in_oven_no(int sems){
    return OVEN_CAPACITY - semctl(sems, 2, GETVAL, NULL);
}

int pizzas_on_table_no(int sems){
    return TABLE_CAPACITY - semctl(sems, 3, GETVAL, NULL);
}

pizza_in_oven put_in_the_oven(int pizza, int* oven, int sems){
    struct sembuf buf1[2];
    buf1[0].sem_num = 0;
    buf1[0].sem_op = -1;
    buf1[0].sem_flg = SEM_UNDO;
    buf1[1].sem_num = 2;
    buf1[1].sem_op = -1;
    buf1[1].sem_flg = SEM_UNDO;

    semop(sems, buf1, 2);

    int pos_in_oven = oven[OVEN_CAPACITY];
    int new_pizza_pos = 0;

    for(int i = 0; i < OVEN_CAPACITY; i++){
        if(oven[(pos_in_oven + i)%OVEN_CAPACITY] == -1){
            oven[(pos_in_oven + i)%OVEN_CAPACITY] = pizza;
            new_pizza_pos = (pos_in_oven + i)%OVEN_CAPACITY;
            oven[OVEN_CAPACITY] = new_pizza_pos;
            break;
        }
    }
    pizza_in_oven result;
    result.pizza_pos = new_pizza_pos;
    result.pizza_no = pizzas_in_oven_no(sems);

    struct sembuf buf2;
    buf2.sem_num = 0;
    buf2.sem_op = 1;
    buf2.sem_flg = SEM_UNDO;

    semop(sems, &buf2, 1);

    return result;
}
pizza_from_oven take_out_the_oven(int pos, int* oven, int sems){
    struct sembuf buf1;
    buf1.sem_num = 0;
    buf1.sem_op = -1;
    buf1.sem_flg = SEM_UNDO;

    semop(sems, &buf1, 1);

    int pizza;

    pizza = oven[pos];
    oven[pos] = -1;

    struct sembuf buf2[2];
    buf2[1].sem_num = 2;
    buf2[1].sem_op = 1;
    buf2[1].sem_flg = SEM_UNDO;
    buf2[0].sem_num = 0;
    buf2[0].sem_op = 1;
    buf2[0].sem_flg = SEM_UNDO;


    pizza_from_oven result;
    result.pizza_type = pizza;
    result.pizza_no = pizzas_in_oven_no(sems) - 1;

    semop(sems, buf2, 2);

    return result;
}

int put_on_the_table(int pizza, int* table, int sems){
    struct sembuf buf1[2];
    buf1[0].sem_num = 1;
    buf1[0].sem_op = -1;
    buf1[0].sem_flg = SEM_UNDO;
    buf1[1].sem_num = 3;
    buf1[1].sem_op = -1;
    buf1[1].sem_flg = SEM_UNDO;

    semop(sems, buf1, 2);

    int pos_on_table = table[TABLE_CAPACITY];
    int new_pizza_pos = 0;

    for(int i = 0; i < TABLE_CAPACITY; i++){
        if(table[(pos_on_table + i)%TABLE_CAPACITY] == -1){
            table[(pos_on_table + i)%TABLE_CAPACITY] = pizza;
            new_pizza_pos = (pos_on_table + i)%TABLE_CAPACITY;
            table[TABLE_CAPACITY] = new_pizza_pos;
            break;
        }
    }
    struct sembuf buf2[2];
    buf2[0].sem_num = 1;
    buf2[0].sem_op = 1;
    buf2[0].sem_flg = SEM_UNDO;

    buf2[1].sem_num = 4;
    buf2[1].sem_op = 1;
    buf2[1].sem_flg = SEM_UNDO;

    int result = pizzas_on_table_no(sems);

    semop(sems, buf2, 2);

    return result;
}

void notify(int sig){
    kill(main_pid, SIGINT);
}

void quit(int sig){
    exit(0);
}

void notify_wrapper(void){
    notify(0);
}


int main(int argc, char** argv)
{
    main_pid = atoi(argv[1]);

    signal(SIGINT, notify);
    signal(SIGUSR1, notify);
    atexit(notify_wrapper);
    signal(SIGUSR2, quit);

    key_t oven_key = ftok("/home", 'A');
    int oven_common = shmget(oven_key, 0, 0666);
    int* oven = (int*) shmat(oven_common, NULL, 0);

    key_t table_key = ftok("/home", 'B');
    int table_common = shmget(table_key, 0, 0666);
    int* table = (int*) shmat(table_common, NULL, 0);

    key_t semset_key = ftok("/home", 'C');
    int sems = semget(semset_key, 0, 0);


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

        pizza_info = put_in_the_oven(pizza_type, oven, sems);
        pos = pizza_info.pizza_pos;
        m = pizza_info.pizza_no;

        printf("%d %lld Dodałem pizze %d. Liczba pizz w piecu: %d.\n", getpid(), current_timestamp(), pizza_type, m);
        sleep_time.tv_sec = 4;
        sleep_time.tv_nsec = rand() % 999999999;
        nanosleep(&sleep_time, NULL);


        pizza_info2 = take_out_the_oven(pos, oven, sems);
        pizza_type = pizza_info2.pizza_type;
        m = pizza_info2.pizza_no;

        k = put_on_the_table(pizza_type, table, sems);
        printf("%d %lld Wyjmuje pizze %d. Liczba pizz w piecu: %d. Liczba pizz na stole: %d\n", getpid(), current_timestamp(), pizza_type, m, k);
    }

//cook

    return 0;
}

