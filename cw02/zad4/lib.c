#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/times.h>

void changeString(char* in, char* out, char* find, char* insert){
    FILE* inFile = fopen(in, "r");
    if(inFile == NULL){
        printf("Failed to open file %s", in);
        exit(1);
    }
    FILE* outFile = fopen(out, "w+");
    if(outFile == NULL){
        printf("Failed to open file %s", out);
        exit(1);
    }

    int findLen = strlen(find);
    int insertLen = strlen(insert);


    int readBytes;
    char buff[2];
    char buff2[findLen];
    buff[1] = '\0';
    int matched = 0;
    while((readBytes = fread(buff, 1, 1, inFile)) != 0){
        if(buff[0] == find[matched]){
            buff2[matched] = buff[0];
            matched++;
        }
        else{
            if(matched != 0){
                fwrite(buff2, 1, matched, outFile);
                matched = 0;
            }
            fwrite(buff, 1, 1, outFile);
        }
        if(matched == findLen){
            fwrite(insert, 1, insertLen, outFile);
            matched = 0;
        }
    }
    fclose(inFile);
    fclose(outFile);
}


int main(int argc, char** argv)
{
    struct tms* start = calloc(1, sizeof(struct tms));
    struct tms* stop = calloc(1, sizeof(struct tms));
    time_t real_start = 0;
    time_t real_stop = 0;

    real_start = times(start);


    if(argc < 5){
        puts("Too few arguments");
        exit(0);
    }

    changeString(argv[1], argv[2], argv[3], argv[4]);


    real_stop = times(stop);

    fprintf( stderr, "\nReal time: %lf\n", (double) (real_stop - real_start) / sysconf(_SC_CLK_TCK));
    fprintf( stderr, "System CPU time: %lf\n", (double) (stop->tms_stime - start->tms_stime) / sysconf(_SC_CLK_TCK));
    fprintf( stderr, "User CPU time: %lf\n", (double) (stop->tms_utime - start->tms_utime) / sysconf(_SC_CLK_TCK));

    free(start);
    free(stop);

    return 0;
}
