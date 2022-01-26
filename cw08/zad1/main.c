#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#define NUMBERS 0
#define BLOCK 1


int* image;
int* out_image;
int width, height, M;

typedef struct{
    int a;
    int b;
} interval;

long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL);
    long long microseconds = te.tv_sec*1000000LL + te.tv_usec;
    return microseconds;
}

void calc_negative_numbers(void* arg){
    long long start = current_timestamp();

    int a = ((interval*) arg)->a;
    int b = ((interval*) arg)->b;

    for(int i = 0; i < width * height; i++){
        if(image[i] >= a && image[i] <= b){
            out_image[i] = 255 - image[i];
        }
    }

    long long end = current_timestamp();

    long* res = calloc(1, sizeof(long));
    *res = end - start;

    pthread_exit(res);
}

void calc_negative_block(void* arg){
    long long start = current_timestamp();

    int a = ((interval*) arg)->a;
    int b = ((interval*) arg)->b;

    for(int i = a; i <= b; i++){
        for(int j = 0; j < height; j++){
            out_image[width * j + i] = 255 - image[width * j + i];
        }
    }

    long long end = current_timestamp();

    long* res = calloc(1, sizeof(long));
    *res = end - start;


    pthread_exit(res);
}


int main(int argc, char** argv)
{
    if(argc != 5){
        printf("Wrong arguments number\n");
        exit(-1);
    }

    int threads_no = 0;

    if((threads_no = atoi(argv[1])) <= 0){
        printf("Wrong threads number\n");
        exit(-1);
    }

    int mode = -1;
    if(strcmp(argv[2], "numbers") == 0){
        mode = NUMBERS;
    }
    else if(strcmp(argv[2], "block") == 0){
        mode = BLOCK;
    }
    else{
        printf("Wrong mode\n");
        exit(-1);
    }

    char* in_file_name = argv[3];
    char* out_file_name = argv[4];

    FILE* in_file;
    if((in_file = fopen(in_file_name, "r")) == NULL){
        printf("Can not open file %s\n", in_file_name);
        exit(-1);
    }


    char buf[4];

    if(fscanf(in_file, "%s\n", buf) != 1){
        printf("Error1 while rading file\n");
        exit(-1);
    }
    if(fscanf(in_file, "%d", &width) != 1){
        printf("Error2 while rading file\n");
        exit(-1);
    }
    if(fscanf(in_file, "%d", &height) != 1){
        printf("Error3 while rading file\n");
        exit(-1);
    }
    if(fscanf(in_file, "%d", &M) != 1){
        printf("Error4 while rading file\n");
        exit(-1);
    }

    if(strcmp(buf, "P2") != 0){
        printf("Wrong file format\n");
        exit(-1);
    }


    image = calloc(width * height, sizeof(int));
    out_image = calloc(width * height, sizeof(int));


    int iter = 0;
    while(fscanf(in_file, "%d ", &image[iter]) == 1){
        iter++;
    }

    FILE* out_file;
    if((out_file = fopen(out_file_name, "w")) == NULL){
        printf("Can not open file %s\n", in_file_name);
        exit(-1);
    }

    pthread_t* threads = calloc(threads_no, sizeof(pthread_t));

    interval* list = calloc(threads_no, sizeof(interval));

    long long start = current_timestamp();

    int res;
    for(int i = 0; i < threads_no; i++){
        if(mode == BLOCK){
            list[i].a = i * ceil(width / threads_no);
            list[i].b = (i+1) * ceil(width / threads_no) - 1;
            list[threads_no-1].b = width - 1;
            if((res = pthread_create(&threads[i], NULL, (void*)&calc_negative_block, &list[i])) != 0){
                printf("error, no: %d\n", res);
            };
        }
        else if(mode == NUMBERS){
            list[i].a = i * ceil(M / threads_no);
            list[i].b = (i+1) * ceil(M / threads_no) - 1;
            list[threads_no-1].b = M;
            if((res = pthread_create(&threads[i], NULL, (void*)&calc_negative_numbers, &list[i])) != 0){
                printf("error, no: %d\n", res);
            }
        }
    }


    long** threads_times = calloc(threads_no, sizeof(long*));


    for (int i = 0; i < threads_no; i++){
        pthread_join(threads[i], (void**)&threads_times[i]);
    }

    long long end = current_timestamp();

    printf("Total time: %lldus\n", end - start);

    printf("Thread times:\n");
    for(int i = 0; i < threads_no; i++){
        printf("Thread no: %d, time: %ldus\n", i, *threads_times[i]);
    }

    fprintf(out_file, "%s\n%d %d\n%d\n", buf, width, height, M);


    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            fprintf(out_file, "%d ", out_image[i * width + j]);
        }
        fseek(out_file, -1, SEEK_CUR);
        fprintf(out_file, "\n");
    }

    free(list);
    free(threads);
    for(int i = 0; i < threads_no; i++){
        free(threads_times[i]);
    }
    free(threads_times);
    free(image);
    free(out_image);

    return 0;
}
