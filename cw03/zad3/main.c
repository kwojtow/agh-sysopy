#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int is_text_file(char* filename){
    char* pattern = ".txt";
    int pattern_len = strlen(pattern);
    int filename_len = strlen(filename);

    int result = 0;
    if(filename_len > pattern_len){
        result = 1;
        for(int i = 0; i < pattern_len; i++){
            if(filename[filename_len-1-i] != pattern[pattern_len-1-i]){
                result = 0;
                break;
            }
        }
    }
    return result;
}

void check_directory(char* path, char* phrase, int max_depth, int curr_depth){
    if(curr_depth > max_depth){
        return;
    }
    DIR* root = opendir(path);

    struct dirent* dir_content = NULL;

    while( (dir_content = readdir(root)) != NULL){

        if(dir_content->d_type == DT_REG && is_text_file(dir_content->d_name) && strcmp(phrase, dir_content->d_name) == 0){
            printf("%d\t%s/%s\n", (int)getpid(), path, dir_content->d_name);
        }

        if(dir_content->d_type == DT_DIR && strcmp(dir_content->d_name, "..") != 0 && strcmp(dir_content->d_name, ".") != 0){
            int new_path_len = strlen(path) + strlen(dir_content->d_name) + 1;
            char* new_path = calloc(new_path_len+1, sizeof(char));
            new_path[new_path_len] = '\0';

            int index = 0;
            while(index < strlen(path)){
                new_path[index] = path[index];
                index++;
            }
            new_path[index] = '/';
            index++;

            for(int i = 0; i < strlen(dir_content->d_name); i++){
                new_path[index+i] = dir_content->d_name[i];
            }

            pid_t pid = fork();
            if(pid == 0){
                check_directory(new_path, phrase, max_depth, curr_depth+1);
                while(wait(NULL) > 0);
                exit(0);
            }
            free(new_path);

        }
    }
    closedir(root);
}

int main(int argc, char** argv)
{
    if(argc != 4){
        exit(1);
    }

    char* dir = argv[1];
    char* phrase = argv[2];
    int max_depth = 0;
    max_depth = atoi(argv[3]);
    if(max_depth == 0){
    	exit(2);
    }

    chdir(dir);
    check_directory(".", phrase, max_depth, 1);

    while(wait(NULL) > 0);

    return 0;
}
