#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>


int waiting_reindeers_no = 0;
int santa_deliver_gifts = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t reindeers_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t santa_cond = PTHREAD_COND_INITIALIZER;

void santa_behavior(){
    while(1){
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&reindeers_cond, &mutex);
        printf("Mikołaj: budzę się\n");
        santa_deliver_gifts = 1;
        printf("Mikołaj: dostarczam zabawki\n");
        pthread_cond_broadcast(&santa_cond);
        sleep(rand() % 2 + 1);
        waiting_reindeers_no = 0;
        santa_deliver_gifts = 0;
        pthread_mutex_unlock(&mutex);
    }
}


void reindeer_behavior(){
    while(1){
        sleep(rand() % 6 + 5);
        pthread_mutex_lock(&mutex);
        waiting_reindeers_no += 1;
        printf("Renifer: czeka %d reniferów na Mikołaja, %d\n", waiting_reindeers_no, gettid());
        if(waiting_reindeers_no == 9){
            printf("Renifer: wybudzam mikołaja, %d\n", gettid());
            pthread_cond_broadcast(&reindeers_cond);
        }
        pthread_mutex_unlock(&mutex);

        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&santa_cond, &mutex);
        pthread_mutex_unlock(&mutex);

        sleep(rand() % 3 + 2);
    }

}

int main()
{
    srand(time(NULL));

    pthread_t santa;
    pthread_t reindeers[9];
    int res = pthread_create(&santa, NULL, (void*)&santa_behavior, NULL);
    if(res != 0){
        printf("Błąd przy tworzeniu nowych wątków\n");
        exit(errno);
    }

    for(int i = 0; i < 9; i++){
        res = pthread_create(&reindeers[i], NULL, (void*)&reindeer_behavior, NULL);
        if(res != 0){
            printf("Błąd przy tworzeniu nowych wątków\n");
            exit(errno);
        }
    }

    pthread_join(santa, NULL);

    for(int i = 0; i < 9; i++){
        pthread_join(reindeers[i], NULL);
    }

    return 0;
}
