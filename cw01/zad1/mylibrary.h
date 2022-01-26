#ifndef MYLIBRARY_H_INCLUDED
#define MYLIBRARY_H_INCLUDED
#include <stdio.h>

typedef struct BlockOfRows{
    char** blockOfRows;
    int size;
} BlockOfRows;

typedef struct MainTable{
    BlockOfRows** blocksTable;
    int size;
} MainTable;

typedef struct PairOfFiles{
    char* fileA;
    char* fileB;
    int size;
} PairOfFiles;

typedef struct SequenceOfPairs{
    PairOfFiles ** pairsOfFiles;
    int size;
} SequenceOfPairs;

typedef struct fileData{
    long fileSize;
    char *fileContent;
} fileData;


MainTable* createMainTable(int size);

SequenceOfPairs* defineSequenceOfPairOfFiles(char** sequenceOfFiles, int size);

FILE* merge(PairOfFiles* pairOfFiles);

MainTable* mergeAll(MainTable* mainTable, char** sequenceOfFiles, int size);

int addBlock(MainTable* mainTable, FILE* f, int size);

void deleteBlock(MainTable* mainTable, int index);

void deleteRow(MainTable* mainTable, int blockIndex, int rowIndex);

void display(MainTable* mainTable);

void cleanPairs(SequenceOfPairs* sop);

void clean(MainTable* mt);

#endif // MYLIBRARY_H_INCLUDED
