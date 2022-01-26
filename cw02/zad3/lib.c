#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <sys/times.h>

int main(int argc, char** argv)
{
    struct tms* start = calloc(1, sizeof(struct tms));
    struct tms* stop = calloc(1, sizeof(struct tms));
    time_t real_start = 0;
    time_t real_stop = 0;

    real_start = times(start);


    FILE* dataFile = fopen("dane.txt", "r" );
    if(dataFile == NULL){
        puts("Failed to open file dane.txt");
        exit(1);
    }
    FILE* aFile = fopen("a.txt", "w+");
    if(aFile == NULL){
        puts("Failed to open file a.txt");
        exit(1);
    }
    FILE* bFile = fopen("b.txt", "w+");
    if(bFile == NULL){
        puts("Failed to open file b.txt");
        exit(1);
    }
    FILE* cFile = fopen("c.txt", "w+");
    if(cFile == NULL){
        puts("Failed to open file c.txt");
        exit(1);
    }

    char buff[11];
    int readBytes = 0;
    int numberLength = 0;
    int evenNumber = 0;


    while((readBytes = fread(buff, 1, 1, dataFile)) != 0){
        numberLength ++;
        if(buff[0] == '\n'){
            fseek(dataFile, -numberLength, 1);
            fread(buff, 1, numberLength, dataFile);

            if(atoi(&buff[numberLength-2]) % 2 == 0){
                evenNumber++;
            }
            if(numberLength-3 >=0 && (buff[numberLength-3] == '0' || buff[numberLength-3] == '7')){
                fwrite(buff, 1, numberLength, bFile);
            }
            buff[numberLength] = '\0';
            int number = atoi(buff);
            if(pow(round(sqrt((double) number)), 2) == number){
                buff[numberLength] = '\n';
                fwrite(buff, 1, numberLength, cFile);
            }


            numberLength = 0;
        }
    }

    char evenResults[36];
    int evenResultsLen;
    sprintf(evenResults, "Liczb parzystych jest %d\n", evenNumber);
    for(int i = 21; i < 36; i++){
        if(evenResults[i] == '\n'){
           evenResultsLen = i;
            break;
        }
    }


    fwrite(evenResults, 1, evenResultsLen, aFile);

    fclose(dataFile);
    fclose(aFile);
    fclose(bFile);
    fclose(cFile);


    real_stop = times(stop);

    fprintf( stderr, "\nReal time: %lf\n", (double) (real_stop - real_start) / sysconf(_SC_CLK_TCK));
    fprintf( stderr, "System CPU time: %lf\n", (double) (stop->tms_stime - start->tms_stime) / sysconf(_SC_CLK_TCK));
    fprintf( stderr, "User CPU time: %lf\n", (double) (stop->tms_utime - start->tms_utime) / sysconf(_SC_CLK_TCK));

    free(start);
    free(stop);

    return 0;
}
