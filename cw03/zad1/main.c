#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


int main(int argc, char** argv)
{
    if(argc == 2){
        int n = atoi(argv[1]);
        pid_t child_pid;
        for(int i=0; i<n; i++){
            child_pid = fork();
            wait(NULL);
            if(child_pid == 0){
                printf("Child PID: %d,  parent PID: %d\n", (int)getpid(), (int)getppid());
                break;
            }
        }
    }
    return 0;
}
