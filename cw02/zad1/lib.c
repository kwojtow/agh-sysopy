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



    char filenames[2][256];
    if(argc < 3){

        puts("Enter first filename:");
        scanf("%s", filenames[0]);
        puts("Enter second filename:");
        scanf("%s", filenames[1]);

    }
    else{
        strcpy(filenames[0], argv[1]);
        strcpy(filenames[1], argv[2]);
    }

    FILE* files[2];

    real_start = times(start);

    for(int i = 0; i < 2; i++){
        files[i] = fopen(filenames[i], "r");
        if(files[i] == NULL){
            printf("\nFailed to open file %s\n", filenames[i]);
            exit(1);
        }
    }

    char buff[2];
    buff[sizeof(buff)] = '\0';
    int readBytes = 0;

    int currFile = 0;

    int EOFs[2] = {0, 0};

    while(!EOFs[0] || !EOFs[1]){

        while((readBytes = fread(buff, 1, sizeof(buff)-1, files[currFile])) != 0){
            printf("%s", buff);
            if(buff[0] == '\n')
                break;
        }
        if(readBytes == 0)
            EOFs[currFile] = 1;
        if(EOFs[(currFile + 1) % 2] == 0)
            currFile = (currFile + 1) % 2;
    }

    for(int i = 0; i < 2; i++){
        fclose(files[i]);
    }

    real_stop = times(stop);

    fprintf( stderr, "\nReal time: %lf\n", (double) (real_stop - real_start) / sysconf(_SC_CLK_TCK));
    fprintf( stderr, "System CPU time: %lf\n", (double) (stop->tms_stime - start->tms_stime) / sysconf(_SC_CLK_TCK));
    fprintf( stderr, "User CPU time: %lf\n", (double) (stop->tms_utime - start->tms_utime) / sysconf(_SC_CLK_TCK));

    free(start);
    free(stop);

    return 0;
}
