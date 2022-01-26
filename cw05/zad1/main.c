#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <wait.h>


typedef struct{
    char* prog;
    char** args;
    int args_no;
} Command;

typedef struct{
    char* name;
    Command* commands;
    int commands_no;
} Component;

typedef struct{
    Component* components;
    int components_no;

} ComponentsDict;

typedef struct{
    Command* commands;
    int commands_no;
} ExecutionLine;

typedef struct{
    ExecutionLine* executionLines;
    int set_len;
} ExecutionSet;

ComponentsDict* componentsDict;
ExecutionSet* executionSet;

void parse_command(char* command_str, Command* command){
    command->args_no = 0;
    command->prog = calloc(1, sizeof(char*));

    int prog = 1;

    char* end;
    char *tok = strtok_r(command_str, " ", &end);
    while(tok != NULL) {
        if(prog == 1){
            strcpy(command->prog, tok);
        }
        else{
            command->args_no++;
            command->args = realloc(command->args, command->args_no * sizeof(char*));
            command->args[command->args_no - 1] = calloc(1, sizeof(char*));
            strcpy(command->args[command->args_no - 1], tok);
        }
        prog = 0;
        tok = strtok_r(NULL, " ", &end);
    }
}

void parse_component_line(char* line, Component* component){
    component->commands_no = 0;
    component->name = calloc(1, sizeof(char*));

    int title = 1;

    char* end;

    char *tok = strtok_r(line, "|=\n", &end);
    while(tok != NULL) {
        if(title == 1){
            char* title = calloc(1, sizeof(char*));
            strcpy(title, tok);
            title[strlen(title)-1] = '\0';
            strcpy(component->name, title);
            free(title);
        }
        else{
            component->commands_no++;
            component->commands = realloc(component->commands, component->commands_no * sizeof(Command));
            char *tmp = calloc(strlen(tok) + 1, sizeof(char));
            strcpy(tmp, tok);
            parse_command(tmp, &component->commands[component->commands_no - 1]);
        }
        title = 0;
        tok = strtok_r(NULL, "|=\n", &end);
    }
}

void parse_exec_line(char* line, ExecutionLine* executionLine){
    executionLine->commands_no = 0;

    char* end;

    char *tok = strtok_r(line, " |\n\t\0", &end);
    while(tok != NULL) {
        for(int i = 0; i < componentsDict->components_no; i++){
            if(strcmp(tok, componentsDict->components[i].name) == 0){
                int prev_commands_no = executionLine->commands_no;
                executionLine->commands_no += componentsDict->components[i].commands_no;
                executionLine->commands = realloc(executionLine->commands, executionLine->commands_no * sizeof(Command));
                for(int j = 0; j < componentsDict->components[i].commands_no; j++){
                    executionLine->commands[prev_commands_no + j] = componentsDict->components[i].commands[j];
                }
            }
        }

        tok = strtok_r(NULL, " |=\n\0", &end);
    }
}

void parse_file(char* filename){
    int file = open(filename, O_RDONLY);

    if(file < 0){
        exit(1); //file not opened
    }

    char buf[1000];
    int read_bytes = 0;

    char line[1000];
    int position = 0;

    componentsDict = calloc(1, sizeof(componentsDict));
    componentsDict->components_no = 0;

    executionSet = calloc(1, sizeof(ExecutionSet));
    executionSet->set_len = 0;

    int new_part = 0, last_part = 0; // 0 - components part, 1 - execution part
    while((read_bytes = read(file, buf, 1000))){
        int i = 0;
        for(i = 0; i < read_bytes; i++){
            if(buf[i] == '\n' && i+1 < read_bytes && buf[i+1] != '\n'){
                position = position + i + 1;
                lseek(file, position, SEEK_SET);
                break;
            }
            else if(buf[i] == '\n' && i+1 < read_bytes && buf[i+1] == '\n'){
                position = position + i + 2;
                lseek(file, position, SEEK_SET);
                new_part = 1;
                break;
            }
            else{
                line[i] = buf[i];
            }
        }
        line[i] = '\0';
        if(last_part == 0){
            componentsDict->components_no++;
            componentsDict->components = realloc(componentsDict->components, componentsDict->components_no * sizeof(Component));

            parse_component_line(line, &componentsDict->components[componentsDict->components_no - 1]);
        }
        else{
            executionSet->set_len++;
            executionSet->executionLines = realloc(executionSet->executionLines, executionSet->set_len * sizeof(ExecutionLine));

            parse_exec_line(line, &executionSet->executionLines[executionSet->set_len - 1]);
        }

        last_part = new_part;
    }

    close(file);
}

void execute_line(ExecutionLine executionLine){

    int prog_no = executionLine.commands_no;
    int **fds = malloc(sizeof (int *) * prog_no);

    for(int i = 0; i < prog_no ; i++) {
        fds[i] = malloc(sizeof (int) * 2);
        if (pipe(fds[i]) < 0) {
            exit(1);
        }
    }

    for(int i = 0; i < prog_no; i++) {
        char **prog_with_args = calloc(executionLine.commands[i].args_no + 1, sizeof(char*));

        prog_with_args[0] = executionLine.commands[i].prog;

        for(int k = 0; k < executionLine.commands[i].args_no; k++){
            prog_with_args[k+1] = executionLine.commands[i].args[k];
        }
        if (fork() == 0) {
            if( i > 0) {
                dup2(fds[i - 1][0], STDIN_FILENO);
            }
            if(i < prog_no - 1) {
                dup2(fds[i][1], STDOUT_FILENO);
            }
            for(int j = 0; j < prog_no; j++) {
                close(fds[j][1]);
                close(fds[j][0]);
            }
            execvp(prog_with_args[0], prog_with_args);
        }
    }

    for(int j = 0; j < prog_no; j++) {
        close(fds[j][1]);
        close(fds[j][0]);
    }
    for (int i = 0; i < prog_no; i++)
        wait(NULL);


    for(int i = 0; i < prog_no ; i++) {
        free(fds[i]);
    }
    free(fds);
}

void execute_set(){
    for(int i = 0; i < executionSet->set_len; i++){
        execute_line(executionSet->executionLines[i]);
    }
}

void clean_dict(){
    for(int i = 0; i < componentsDict->components_no; i++){
        for(int j = 0; j < componentsDict->components[i].commands_no; j++){
            for(int k = 0; k < componentsDict->components[i].commands[j].args_no; k++){
                free(componentsDict->components[i].commands[j].args[k]);
            }
            free(componentsDict->components[i].commands[j].prog);
            free(componentsDict->components[i].commands[j].args);
        }
        free(componentsDict->components[i].name);
        free(componentsDict->components[i].commands);
    }
    free(componentsDict->components);
}

void clean_exec_set(){
    for(int i = 0; i < executionSet->set_len; i++){
        free(executionSet->executionLines[i].commands);
    }
    free(executionSet->executionLines);
}


int main(int argc, char** argv)
{
    if(argc != 2){
        exit(1);
    }
    char* file = argv[1];

    parse_file(file);
    execute_set();
    clean_dict();
    clean_exec_set();

}
