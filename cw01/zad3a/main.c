#include "mylibrary.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>

int correctOperation(char* operation){
    return strcmp(operation, "create_table") *
            strcmp(operation, "merge_files") *
            strcmp(operation, "remove_block") *
            strcmp(operation, "remove_row");
}

void runTest(char* files, int size){
    struct tms* start = calloc(1, sizeof(struct tms));
    struct tms* stop = calloc(1, sizeof(struct tms));
    time_t real_start = 0;
    time_t real_stop = 0;

    MainTable* mainTable = createMainTable(size);


    char** seq = calloc(size, sizeof(char*));

    printf("\n\n+++  Tested pair: %s  +++\n", files);
    printf("+++  Sample size: %d +++\n", size);

    for(int i = 0; i < size; i++){

        seq[i] = calloc(1, sizeof(char*));
        strcpy(seq[i], files);
    }

    SequenceOfPairs* sequenceOfPairs = defineSequenceOfPairOfFiles(seq, size);




    FILE** tmp_files = calloc(size, sizeof(FILE*));

    real_start = times(start);

    for(int i = 0; i < size; i++){
        tmp_files[i] = merge(sequenceOfPairs->pairsOfFiles[i]);
    }

    real_stop = times(stop);

    printf("\nMerging:\n");
    printf("Real time: %lf\n", (double) (real_stop - real_start) / sysconf(_SC_CLK_TCK));
    printf("System CPU time: %lf\n", (double) (stop->tms_stime - start->tms_stime) / sysconf(_SC_CLK_TCK));
    printf("User CPU time: %lf\n", (double) (stop->tms_utime - start->tms_utime) / sysconf(_SC_CLK_TCK));


    real_start = times(start);

    for(int i = 0; i < size; i++){
        addBlock(mainTable, tmp_files[i], sequenceOfPairs->pairsOfFiles[i]->size);
    }



    real_stop = times(stop);

    printf("\nAdding blocks:\n");
    printf("Real time: %lf\n", (double) (real_stop - real_start) / sysconf(_SC_CLK_TCK));
    printf("System CPU time: %lf\n", (double) (stop->tms_stime - start->tms_stime) / sysconf(_SC_CLK_TCK));
    printf("User CPU time: %lf\n", (double) (stop->tms_utime - start->tms_utime) / sysconf(_SC_CLK_TCK));

    real_start = times(start);

    for(int i = 0; i < size; i++){
        deleteBlock(mainTable, i);
    }

    real_stop = times(stop);

    printf("\nRemoving blocks:\n");
    printf("Real time: %lf\n", (double) (real_stop - real_start) / sysconf(_SC_CLK_TCK));
    printf("System CPU time: %lf\n", (double) (stop->tms_stime - start->tms_stime) / sysconf(_SC_CLK_TCK));
    printf("User CPU time: %lf\n", (double) (stop->tms_utime - start->tms_utime) / sysconf(_SC_CLK_TCK));


    real_start = times(start);

    for(int j = 0; j < 2; j++){
        for(int i = 0; i < size; i++){
            addBlock(mainTable, tmp_files[i], sequenceOfPairs->pairsOfFiles[i]->size);
        }
        for(int i = 0; i < size; i++){
            deleteBlock(mainTable, i);
        }
    }


    real_stop = times(stop);

    printf("\nAdding and removing blocks 10 times:\n");
    printf("Real time: %lf\n", (double) (real_stop - real_start) / sysconf(_SC_CLK_TCK));
    printf("System CPU time: %lf\n", (double) (stop->tms_stime - start->tms_stime) / sysconf(_SC_CLK_TCK));
    printf("User CPU time: %lf\n", (double) (stop->tms_utime - start->tms_utime) / sysconf(_SC_CLK_TCK));



    for(int i = 0; i < size; i++){

        //free(seq[i]);
    }
    free(seq);
    for(int i = 0; i < size; i++){
        fclose(tmp_files[i]);
    }
    free(tmp_files);
    exit(0);
}



int main(int argc, char **argv)
{

    if(argc == 4 && strcmp(argv[1], "test") == 0){
        runTest(argv[2], atoi(argv[3]));
    }


    int size = 0;
    size = atoi(argv[1]);

    if(size <= 0){
        printf("Wrong number of pairs to merge\n");
        exit(0);
    }

    MainTable* mainTable = NULL;

    for(int i = 2; i < argc-1; i++){
        if(correctOperation(argv[i]) != 0){
            printf("Unknown command\n");
            exit(0);
        }

        if(strcmp(argv[i], "create_table") == 0){
            if(i+1 > argc-1){
                printf("Wrong number of parameters in create_table\n");
                exit(0);
            }
            int tableSize = 0;
            tableSize = atoi(argv[++i]);
            if(tableSize <= 0){
                printf("Wrong table size\n");
                exit(0);
            }

            mainTable = createMainTable(tableSize);

            printf("Table created\n");
        }

        if(strcmp(argv[i], "merge_files") == 0){
            if(i+size > argc-1){
                printf("Wrong number of parameters in merge_files\n");
                exit(0);
            }

            for(int k = i+1; k <= i+size; k++){
                if(strchr(argv[k], ':') == NULL){
                    printf("Wrong format of pairs to merge\n");
                    exit(0);
                }
            }

            char** seq = calloc(size, sizeof(char*));
            int j;
            for(j=1; j <= size; j++){
                seq[j-1] = argv[i+j];
            }
            i += j-1;

            SequenceOfPairs* sequenceOfPairs = defineSequenceOfPairOfFiles(seq, size);

            free(seq);

            for(int k = 0; k < sequenceOfPairs->size; k++){

                FILE *f = NULL;
                f = merge(sequenceOfPairs->pairsOfFiles[k]);
                printf("Files merged\n");

                addBlock(mainTable, f,  sequenceOfPairs->pairsOfFiles[k]->size);
                printf("Block added\n");

                fclose(f);
            }

            cleanPairs(sequenceOfPairs);

        }

        if(strcmp(argv[i], "remove_block") == 0){
            if(i+1 > argc-1){
                printf("Wrong number of parameters in remove_block\n");
                exit(0);
            }
            int blockToDelete = -1;
            blockToDelete = atoi(argv[++i]);
            if(blockToDelete <= -1){
                printf("Wrong index of block to delete\n");
                exit(0);
            }


            deleteBlock(mainTable, blockToDelete);
            printf("Block delted\n");

        }

        if(strcmp(argv[i], "remove_row") == 0){
            if(i+2 > argc-1){
                    printf("Wrong number of parameters in remove_row\n");
                    exit(0);
                }
            int blockNumber = -1;
            blockNumber = atoi(argv[++i]);
            if(blockNumber <= -1){
                printf("Wrong index of block\n");
                exit(0);
            }
            int rowNumber = -1;
            rowNumber = atoi(argv[++i]);
            if(rowNumber <= -1){
                printf("Wrong index of row\n");
                exit(0);
            }

            deleteRow(mainTable, blockNumber, rowNumber);
            printf("Row delted\n");
        }



    }


    clean(mainTable);

    return 0;
}
