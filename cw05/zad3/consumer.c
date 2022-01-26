#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    if(argc != 4){
        exit(1);
    }
    FILE* fifo = fopen(argv[1], "r");
    if(fifo  == NULL){
        puts("Opening pipe failed\n");
        exit(2);
    }

    FILE* out_file = fopen(argv[2], "w");
    if(out_file == NULL){
        puts("Opening output file failed\n");
        exit(3);
    }

    int chars_number = -1;
    chars_number = atoi(argv[3]);
    if(chars_number < 0){
        puts("Wrong chars number\n");
        exit(4);
    }

    int row_number;
    int read_bytes;
    char* buff = calloc(chars_number + 2, sizeof(char));


    int producers_number = 0;
    char** to_save_storage = NULL;

    int* storage_lines_len = NULL;

    char* tmp = calloc(chars_number + 2, sizeof(char));
    int chars_to_read = chars_number+1;

    while((read_bytes = fread(buff, sizeof(char), chars_to_read, fifo)) > 0){
        if(chars_to_read < chars_number +1){
            if(read_bytes == chars_to_read){
                read_bytes = chars_number + 1;
            }
            for(int i = 0; i < chars_number+1; i++){
                tmp[i] = buff[ (chars_to_read + i) % (chars_number + 1) ];
            }
            for(int i = 0; i < chars_number+1; i++){
                buff[i] = tmp[i];
            }
        }
        chars_to_read = chars_number + 1;

        for(int i = 0; i < read_bytes; i++){
            if(buff[i] == '\n'){
                read_bytes = i;
                chars_to_read = i+1;
                break;
            }
        }

        buff[read_bytes] = '\0';
        char row_number_string[2];
        row_number_string[0] = buff[0];
        row_number_string[1] = '\0';
        row_number = atoi(row_number_string);
        if(row_number >= producers_number){
            int old_producers_number = producers_number;
            producers_number = row_number+1;
            to_save_storage = realloc(to_save_storage, producers_number * sizeof(char*));
            storage_lines_len = realloc(storage_lines_len, producers_number * sizeof(int));
            for(int i = old_producers_number; i < producers_number; i++){
                storage_lines_len[i] = 0;
            }
        }

        storage_lines_len[row_number] += read_bytes-1;

        to_save_storage[row_number] = realloc(to_save_storage[row_number], (storage_lines_len[row_number] + 1) * sizeof(char));

        for(int i = 0; i < chars_number && i < read_bytes; i++){
            to_save_storage[row_number][storage_lines_len[row_number] - read_bytes + 1 + i] = buff[i+1];
        }
        to_save_storage[row_number][storage_lines_len[row_number]] = '\0';
    }

    for(int i = 0; i < producers_number; i++){
        fprintf(out_file, "%s\n", to_save_storage[i]);
    }

    free(buff);
    fclose(fifo);
    fclose(out_file);

    return 0;
}

