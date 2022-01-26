#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

void sig_handler(int sig){
    printf("Otrzymano sygnał: %d; Bieżący proces: %d; Proces nadrzędny: %d\n", sig, getpid(), getppid());
}

int main(int argc, char** argv)
{
	
    printf("\nProces potomny - exec()\n");
    printf("\nPID dziecka: %d\n", getpid());

    sigset_t mask;
    sigprocmask(SIG_BLOCK, NULL, &mask);

    if(sigismember(&mask, SIGUSR1)){
        printf("Maska procesu %d zawiera sygnał SIGUSR1\n", getpid());
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

    return 0;
}

