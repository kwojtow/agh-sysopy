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


    int dataFile = open("dane.txt", O_RDONLY);
    if(dataFile == -1){
        puts("Failed to open file dane.txt");
        exit(1);
    }
    int aFile = open("a.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if(aFile == -1){
        puts("Failed to open file a.txt");
        exit(1);
    }
    int bFile = open("b.txt", O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, S_IRWXU);
    if(bFile == -1){
        puts("Failed to open file b.txt");
        exit(1);
    }
    int cFile = open("c.txt", O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, S_IRWXU);
    if(cFile == -1){
        puts("Failed to open file c.txt");
        exit(1);
    }

    char buff[11];
    int readBytes = 0;
    int numberLength = 0;
    int evenNumber = 0;


    while((readBytes = read(dataFile, buff, 1)) != 0){
        numberLength ++;
        if(buff[0] == '\n'){
            lseek(dataFile, -numberLength, SEEK_CUR);
            read(dataFile, buff, numberLength);

            if(atoi(&buff[numberLength-2]) % 2 == 0){
                evenNumber++;
            }
            if(numberLength-3 >=0 && (buff[numberLength-3] == '0' || buff[numberLength-3] == '7')){
                write(bFile, buff, numberLength);
            }
            buff[numberLength] = '\0';
            int number = atoi(buff);
            if(pow(round(sqrt((double) number)), 2) == number){
                buff[numberLength] = '\n';
                write(cFile, buff, numberLength);
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


    write(aFile, evenResults, evenResultsLen);

    close(dataFile);
    close(aFile);
    close(bFile);
    close(cFile);


    real_stop = times(stop);

    fprintf( stderr, "\nReal time: %lf\n", (double) (real_stop - real_start) / sysconf(_SC_CLK_TCK));
    fprintf( stderr, "System CPU time: %lf\n", (double) (stop->tms_stime - start->tms_stime) / sysconf(_SC_CLK_TCK));
    fprintf( stderr, "User CPU time: %lf\n", (double) (stop->tms_utime - start->tms_utime) / sysconf(_SC_CLK_TCK));

    free(start);
    free(stop);

    return 0;
}
