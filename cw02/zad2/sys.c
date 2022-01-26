#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/times.h>

int main(int argc, char** argv)
{
    struct tms* start = calloc(1, sizeof(struct tms));
    struct tms* stop = calloc(1, sizeof(struct tms));
    time_t real_start = 0;
    time_t real_stop = 0;

    real_start = times(start);


    if(argc < 3){
        puts("\nTo few arguments!\n");
        exit(2);
    }
    else if(argc > 3){
        puts("\nTo many arguments!\n");
        exit(2);
    }

    char c = argv[1][0];
    char* filename = argv[2];

    int file = open(filename, O_RDONLY);
    if(file == -1){
        printf("\nFailed to open file %s\n", filename);
        exit(1);
    }

    int readBytes = 0;
    int lineLength = 0;
    int charFound = 0;
    char buff[256];

    while((readBytes = read(file, buff, 1)) != 0){
        lineLength += readBytes;
        if(buff[0] == c)
            charFound = 1;
        if(buff[0] == '\n'){
            if(charFound == 1){
                lseek(file, -lineLength, SEEK_CUR);
                read(file, buff, lineLength);
                buff[lineLength] = '\0';
                printf("%s", buff);
                charFound = 0;
            }
            lineLength = 0;
        }
    }
    close(file);

    real_stop = times(stop);

    fprintf( stderr, "\nReal time: %lf\n", (double) (real_stop - real_start) / sysconf(_SC_CLK_TCK));
    fprintf( stderr, "System CPU time: %lf\n", (double) (stop->tms_stime - start->tms_stime) / sysconf(_SC_CLK_TCK));
    fprintf( stderr, "User CPU time: %lf\n", (double) (stop->tms_utime - start->tms_utime) / sysconf(_SC_CLK_TCK));

    free(start);
    free(stop);

    return 0;
}
