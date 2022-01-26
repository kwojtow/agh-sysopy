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
        puts("Too few arguments");
        exit(0);
    }
    int inFile = open(argv[1], O_RDONLY );
    if(inFile == -1){
        printf("Failed to open file %s", argv[1]);
        exit(1);
    }
    int outFile = open(argv[2], O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, S_IRWXU);
    if(outFile == -1){
        printf("Failed to open file %s", argv[2]);
        exit(1);
    }

    int readBytes = 0;
    char buff[2];
    int lineLength = 0;
    buff[1] = '\0';
    while((readBytes = read(inFile, buff, 1)) != 0){
        lineLength++;
        write(outFile, buff, 1);
        if(lineLength >= 50){
            char newLine = '\n';
            write(outFile, &newLine, 1);
            lineLength = 0;
        }
        if(buff[0] == '\n'){
            lineLength = 0;
        }
    }

    close(inFile);
    close(outFile);


    real_stop = times(stop);

    fprintf( stderr, "\nReal time: %lf\n", (double) (real_stop - real_start) / sysconf(_SC_CLK_TCK));
    fprintf( stderr, "System CPU time: %lf\n", (double) (stop->tms_stime - start->tms_stime) / sysconf(_SC_CLK_TCK));
    fprintf( stderr, "User CPU time: %lf\n", (double) (stop->tms_utime - start->tms_utime) / sysconf(_SC_CLK_TCK));

    free(start);
    free(stop);

    return 0;
}
