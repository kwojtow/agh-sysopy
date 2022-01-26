#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char** argv)
{
    if(argc != 5){
        exit(1);
    }
    FILE* fifo = fopen(argv[1], "w");
    if(fifo == NULL){
        exit(2);
    }

    int row_number = -1;
    row_number = atoi(argv[2]);
    if(row_number < 0){
        exit(3);
    }

    FILE* data = fopen(argv[3], "r");
    if(data == NULL){
        exit(4);
    }

    int char_number = -1;
    char_number = atoi(argv[4]);
    if(char_number < 0){
        exit(5);
    }

    int read_bytes;
    char* buff = calloc(char_number + 1, sizeof(char));
    buff[char_number] = '\0';

    srand(time(NULL));

    while((read_bytes = fread(buff, sizeof(char), char_number, data)) > 0){
        sleep(rand() % 2 + 1);
        buff[read_bytes] = '\0';

        printf("sended data: %d%s\n", row_number, buff);

        fprintf(fifo, "%d%s", row_number, buff);

    }

    free(buff);
    fclose(fifo);
    fclose(data);

    return 0;
}

