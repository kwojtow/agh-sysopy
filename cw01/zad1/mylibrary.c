#define  _GNU_SOURCE
#include "mylibrary.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

MainTable* createMainTable(int size){
    MainTable* mainTable = calloc(1, sizeof(MainTable));
    mainTable->blocksTable = calloc(size, sizeof(BlockOfRows*));
    for(int i = 0; i < size; i++){
        mainTable->blocksTable[i] = NULL;
    }
    mainTable->size = size;

    return mainTable;
}

SequenceOfPairs* defineSequenceOfPairOfFiles(char** sequenceOfFiles, int size){
    SequenceOfPairs* sequenceOfPairs = calloc(size, sizeof(SequenceOfPairs));
    sequenceOfPairs->size = size;
    sequenceOfPairs->pairsOfFiles = calloc(sequenceOfPairs->size, sizeof(PairOfFiles));

    for(int i = 0; i < sequenceOfPairs->size; i++){
        sequenceOfPairs->pairsOfFiles[i] = calloc(1, sizeof(PairOfFiles));
        sequenceOfPairs->pairsOfFiles[i]->fileA = strtok(sequenceOfFiles[i], ":");
        sequenceOfPairs->pairsOfFiles[i]->fileB = strtok(NULL, "");
    }

    return sequenceOfPairs;
}

FILE* merge(PairOfFiles* pairOfFiles){

    struct fileData* openFile(char *name){

        struct fileData *results = calloc(1, sizeof(struct fileData));

        FILE *f = NULL;
        f = fopen(name, "r");

        if (f == NULL)
        {
            perror(strcat("Cannot open file ", name));
            exit(1);
        }
        fseek(f, 0, SEEK_END);
        results->fileSize = ftell(f);
        fseek(f, 0, SEEK_SET);


        results->fileContent = calloc(results->fileSize + 1, sizeof(char));


        fread(results->fileContent, 1, results->fileSize, f);

        fclose(f);

        return results;
    }



    struct fileData *fileAData = openFile(pairOfFiles->fileA);
    struct fileData *fileBData = openFile(pairOfFiles->fileB);

    FILE *temp = tmpfile();


    char *tmp = calloc(fileAData->fileSize + fileBData->fileSize + 1, sizeof(char));


    long indexA = 0;
    long indexB = 0;
    int rowsNumber = 0;



    while(indexA < fileAData->fileSize && indexB < fileBData->fileSize){
        while(fileAData->fileContent[indexA]){
            tmp[indexA+indexB] = fileAData->fileContent[indexA];
            indexA ++;
            if(fileAData->fileContent[indexA - 1] == '\n'){
                rowsNumber++;
                break;
            }
        }
        while(fileBData->fileContent[indexB]){
            tmp[indexA+indexB] = fileBData->fileContent[indexB];
            indexB ++;
            if(fileBData->fileContent[indexB - 1] == '\n'){
                rowsNumber++;
                break;
            }
        }
    }

    pairOfFiles->size = rowsNumber;

    fwrite(tmp, 1, indexA + indexB, temp);

    rewind(temp);

    return temp;

}

MainTable* mergeAll(MainTable* mainTable, char** sequenceOfFiles, int size){
    SequenceOfPairs* sequenceOfPairs = defineSequenceOfPairOfFiles(sequenceOfFiles, size);

    for(int i = 0; i < sequenceOfPairs->size; i++){

        FILE* f = merge(sequenceOfPairs->pairsOfFiles[i]);


        for(int k = 0; k < mainTable->size; k++){
            if(mainTable->blocksTable[k] != NULL)
                continue;


            mainTable->blocksTable[k] = calloc(1, sizeof(BlockOfRows*));
            mainTable->blocksTable[k]->size = sequenceOfPairs->pairsOfFiles[k]->size;
            mainTable->blocksTable[k]->blockOfRows = calloc(mainTable->blocksTable[k]->size, sizeof(char*));

            size_t size_buf = 0;
            for(int j = 0; j < mainTable->blocksTable[k]->size; j++) {
                getline(&(mainTable->blocksTable[k]->blockOfRows[j]), &size_buf, f);
                size_buf = 0;
            }
            break;
        }

        fclose(f);


    }

    return mainTable;
}

int addBlock(MainTable* mainTable, FILE* f, int size){
    int blockIndex = -1;
    for(int k = 0; k < mainTable->size; k++){
        if(mainTable->blocksTable[k] != NULL)
            continue;


        mainTable->blocksTable[k] = calloc(1, sizeof(BlockOfRows*));
        mainTable->blocksTable[k]->size = size;
        mainTable->blocksTable[k]->blockOfRows = calloc(mainTable->blocksTable[k]->size, sizeof(char*));

        size_t size_buf = 0;
        for(int j = 0; j < mainTable->blocksTable[k]->size; j++) {
            getline(&(mainTable->blocksTable[k]->blockOfRows[j]), &size_buf, f);
            size_buf = 0;
        }
        blockIndex = k;
        break;
    }

        return blockIndex;
}

int countRows(BlockOfRows* blockOfRows){
    return blockOfRows->size;
}

void deleteBlock(MainTable* mainTable, int index){
    free(mainTable->blocksTable[index]->blockOfRows);
    mainTable->blocksTable[index]->blockOfRows = NULL;
    free(mainTable->blocksTable[index]);
    mainTable->blocksTable[index] = NULL;
}

void deleteRow(MainTable* mainTable, int blockIndex, int rowIndex){
    free(mainTable->blocksTable[blockIndex]->blockOfRows[rowIndex]);
    mainTable->blocksTable[blockIndex]->blockOfRows[rowIndex] = NULL;
}

void display(MainTable* mainTable){
    for(int i = 0; i < mainTable->size; i++){
        if(mainTable->blocksTable[i] == NULL)
            continue;
        for(int j = 0; j < mainTable->blocksTable[i]->size; j++){
            if(mainTable->blocksTable[i]->blockOfRows[j] == NULL)
                continue;
            printf("%s", mainTable->blocksTable[i]->blockOfRows[j]);
        }
    }
}
void cleanPairs(SequenceOfPairs* sop){
    for(int i = 0; i < sop->size; i++){
        free(sop->pairsOfFiles[i]);
    }
    free(sop);
}

void clean(MainTable* mt){


    for(int i = 0; i < mt->size; i++){
        if(mt->blocksTable[i] == NULL)
            continue;
        for(int j = 0; j < mt->blocksTable[i]->size; j++){
            if(mt->blocksTable[i]->blockOfRows[j] == NULL)
                continue;
            free(mt->blocksTable[i]->blockOfRows[j]);
        }
        free(mt->blocksTable[i]);
    }
    free(mt);
}
