#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <wait.h>

int check_param(char* param, char** correct_params, int correct_params_number){
    for(int i = 0; i < correct_params_number; i++){
        if(strcmp(param, correct_params[i]) == 0){
            return i;
        }
    }
    return -1;
}

void sig_handler(int sig){
    printf("Otrzymano sygnał: %d; Bieżący proces: %d; Proces nadrzędny: %d\n", sig, getpid(), getppid());
}

int main(int argc, char** argv)
{

    char** correct_params = calloc(4, sizeof(char*));
    correct_params[0] = "ignore";
    correct_params[1] = "handler";
    correct_params[2] = "mask";
    correct_params[3] = "pending";
    int correct_params_number = 4;

    if(argc != 2){
        printf("%s\n", argv[0]);
        return 1;
    }
    int param_no;
    if((param_no = check_param(argv[1], correct_params, correct_params_number)) == -1){
        return 2;
    }

    switch(param_no){
        case 0:
            {
                printf("\n\n----------IGNORE----------\nPID procesu głównego: %d\n", getpid());
                signal(SIGUSR1, SIG_IGN);
                raise(SIGUSR1);
            }
            break;
        case 1:
            {
                printf("\n\n----------HANDLER----------\nPID procesu głównego: %d\n", getpid());
                struct sigaction act;
                act.sa_handler = sig_handler;
                sigemptyset(&act.sa_mask);
                act.sa_flags = 0;
                sigaction(SIGUSR1, &act, NULL);
            }
            break;
        case 2:
            {
                printf("\n\n----------MASK----------\nPID procesu głównego: %d\n", getpid());
                sigset_t new_set;
                sigemptyset(&new_set);
                sigaddset(&new_set, SIGUSR1);
                sigprocmask(SIG_SETMASK, &new_set, NULL);
            }
            break;
        case 3:
            {
                printf("\n\n----------PENDING----------\nPID procesu głównego: %d\n", getpid());
                sigset_t new_set;
                sigemptyset(&new_set);
                sigaddset(&new_set, SIGUSR1);
                sigprocmask(SIG_SETMASK, &new_set, NULL);
            }
            break;
    }


    sigset_t pending_set;
    sigpending(&pending_set);
    if(sigismember(&pending_set, SIGUSR1)){
        printf("Kolejka procesu %d zawiera sygnał SIGUSR1\n", getpid());
    }
    else{
        printf("Kolejka procesu %d nie zawiera sygnału SIGUSR1\n", getpid());
    }
    printf("Wysyłanie sygnału...\n");
    raise(SIGUSR1);

    sigpending(&pending_set);
    if(sigismember(&pending_set, SIGUSR1)){
        printf("Kolejka procesu %d zawiera sygnał SIGUSR1\n", getpid());
    }
    else{
        printf("Kolejka procesu %d nie zawiera sygnału SIGUSR1\n", getpid());
    }

    pid_t child_pid = fork();
    if(child_pid == 0){
    	printf("\nProces potomny - fork()\n");
    
        printf("\nPID dziecka: %d\n", getpid());

        sigset_t child_pending_set;
        sigpending(&child_pending_set);

        sigset_t mask;
        sigprocmask(SIG_BLOCK, NULL, &mask);

        if(sigismember(&mask, SIGUSR1)){
            printf("Maska procesu %d zawiera sygnał SIGUSR1\n", getpid());
        }

        if(sigismember(&child_pending_set, SIGUSR1)){
            printf("Kolejka procesu %d zawiera sygnał SIGUSR1\n", getpid());
        }
        else{
            printf("Kolejka procesu %d nie zawiera sygnału SIGUSR1\n", getpid());
        }

        printf("Wysyłanie sygnału...\n");
        raise(SIGUSR1);

        sigpending(&child_pending_set);

        if(sigismember(&child_pending_set, SIGUSR1)){
            printf("Kolejka procesu %d zawiera sygnał SIGUSR1\n", getpid());
        }
        else{
            printf("Kolejka procesu %d nie zawiera sygnału SIGUSR1\n", getpid());
        }
    }
    else{
        while(wait(NULL) > 0);
        execl("./child", "child", NULL);
    }



    return 0;
}
