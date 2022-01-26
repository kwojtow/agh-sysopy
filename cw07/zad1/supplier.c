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
    int pizza_type;
    int pizza_no;
} pizza_from_table;

long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    return milliseconds;
}

int pizzas_on_table_no(int sems){
    return TABLE_CAPACITY - semctl(sems, 3, GETVAL, NULL);
}

pizza_from_table take_out_the_table(int* table, int sems){
    struct sembuf buf1[2];
    buf1[0].sem_num = 1;
    buf1[0].sem_op = 1;
    buf1[0].sem_flg = SEM_UNDO;
    buf1[1].sem_num = 4;
    buf1[1].sem_op = -1;
    buf1[1].sem_flg = SEM_UNDO;

    semop(sems, buf1, 2);

    int pizza;

    int pos_on_table = table[TABLE_CAPACITY];

    for(int i = 0; i < TABLE_CAPACITY; i++){
        if(table[(pos_on_table + i)%TABLE_CAPACITY] != -1){
            pizza = table[(pos_on_table + i)%TABLE_CAPACITY];
            table[(pos_on_table + i)%TABLE_CAPACITY] = -1;
            table[TABLE_CAPACITY] = (pos_on_table + i+1)%TABLE_CAPACITY;
            break;
        }
    }

    struct sembuf buf2[2];
    buf2[1].sem_num = 3;
    buf2[1].sem_op = 1;
    buf2[1].sem_flg = SEM_UNDO;
    buf2[0].sem_num = 1;
    buf2[0].sem_op = 1;
    buf2[0].sem_flg = SEM_UNDO;


    pizza_from_table result;
    result.pizza_type = pizza;
    result.pizza_no = pizzas_on_table_no(sems) - 1;

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

    struct timespec sleep_time;

    key_t table_key = ftok("/home", 'B');
    int table_common = shmget(table_key, 0, 0666);
    int* table = (int*) shmat(table_common, NULL, 0);

    key_t semset_key = ftok("/home", 'C');
    int sems = semget(semset_key, 0, 0);


//supplier

    pizza_from_table pizza_info;
    int pizza_type;
    int k;
    while(1){
        //todo zabrać ze stołu
        pizza_info = take_out_the_table(table, sems);
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
