#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "mylibrary.h"
#include <sys/types.h>
#include <sys/times.h>
#include <sys/wait.h>

int main(int argc, char** argv)
{
    struct tms* start = calloc(1, sizeof(struct tms));
    struct tms* stop = calloc(1, sizeof(struct tms));
    time_t real_start = 0;
    time_t real_stop = 0;

    int n = 0;
    char** sequenceOfFiles;

    if(argc == 4 && strcmp(argv[1], "test")==0){
        n = atoi(argv[3]);
        sequenceOfFiles = calloc(n, sizeof(char*));
        for(int i=0; i<n; i++){
            sequenceOfFiles[i] = calloc(1, sizeof(argv[2]));
            strcpy(sequenceOfFiles[i], argv[2]);
        }
    }
    else{
        if(argc < 2){
            exit(1);
        }

        n = atoi(argv[1]);

        if(n == 0){
            exit(2);
        }

        if(argc-2 != n){
            exit(3);
        }

        sequenceOfFiles = calloc(n, sizeof(char*));

        for(int i = 2; i<argc; i++){
            sequenceOfFiles[i-2] = argv[i];
        }
    }



    MainTable* mainTable = createMainTable(n);


    SequenceOfPairs* sequenceOfPairs = defineSequenceOfPairOfFiles(sequenceOfFiles, n);

    real_start = times(start);

    pid_t child_pid;
    for(int i=0; i<n; i++){
        child_pid = fork();
        if(child_pid == 0){
            //printf("PID: %d, pair: %s:%s\n", (int) getpid(), sequenceOfPairs->pairsOfFiles[i]->fileA, sequenceOfPairs->pairsOfFiles[i]->fileB);
            FILE* f = merge(sequenceOfPairs->pairsOfFiles[i]);
            addBlock(mainTable, f, sequenceOfPairs->pairsOfFiles[i]->size);
            fclose(f);
            exit(0);
        }
    }

    while(wait(NULL) > 0);


    real_stop = times(stop);

    printf("\n\n+++  Tested pair: %s  +++\n", argv[2]);
    printf("+++  Sample size: %d +++\n", n);
    printf("\nReal time: %lf\n", (double) (real_stop - real_start) / sysconf(_SC_CLK_TCK));
    printf("System CPU time: %lf\n", (double) (stop->tms_cstime - start->tms_cstime) / sysconf(_SC_CLK_TCK));
    printf("User CPU time: %lf\n", (double) (stop->tms_cutime - start->tms_cutime) / sysconf(_SC_CLK_TCK));


    cleanPairs(sequenceOfPairs);
    clean(mainTable);

    return 0;
}
