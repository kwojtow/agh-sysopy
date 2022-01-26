#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

int depth = 0;

void sig_handler1(int sig, siginfo_t *info, void *vcontext) {
    printf("Signal number: %d\n", info->si_signo);
    printf("Process ID of sending process: %d\n", info->si_pid);
    printf("Signal code: %d\n", info->si_code);
    printf("Real user ID of sending process: %d\n", info->si_uid);
    printf("Exit value or signal for process termination %d\n", info->si_status);
}

void sig_handler2(int sig, siginfo_t *info, void *vcontext) {
    printf("Handler begin\n");
    depth++;
    if(depth < 5){
        raise(SIGUSR1);
    }
    printf("Handler end\n");
}

void sig_handler3(int sig, siginfo_t *info, void *vcontext) {
    printf("Custom handler\n");
}


int main()
{
    printf("\nUsing SA_SIGINFO\n\n");

    sigset_t lock_mask;
    sigemptyset(&lock_mask);

    struct sigaction received_sig;
    received_sig.sa_handler = (void (*)(int)) sig_handler1;
    received_sig.sa_flags = SA_SIGINFO;
    received_sig.sa_mask = lock_mask;
    sigaction(SIGUSR1, &received_sig, NULL);

    raise(SIGUSR1);

    printf("\nNot using SA_NODEFER\n\n");

    received_sig.sa_handler = (void (*)(int)) sig_handler2;
    sigaction(SIGUSR1, &received_sig, NULL);

    raise(SIGUSR1);

    printf("\nUsing SA_NODEFER\n\n");

    sigemptyset(&lock_mask);
    received_sig.sa_mask = lock_mask;
    received_sig.sa_flags = SA_NODEFER;
    sigaction(SIGUSR1, &received_sig, NULL);

    depth = 0;
    raise(SIGUSR1);

    printf("\nNot using SA_RESETHAND\n\n");

    sigemptyset(&lock_mask);
    received_sig.sa_mask = lock_mask;
    received_sig.sa_handler = (void (*)(int)) sig_handler3;
    received_sig.sa_flags = 0;
    sigaction(SIGUSR1, &received_sig, NULL);

    raise(SIGUSR1);
    raise(SIGUSR1);

    printf("\nUsing SA_RESETHAND\n\n");

    received_sig.sa_flags = SA_RESETHAND;
    sigaction(SIGUSR1, &received_sig, NULL);

    raise(SIGUSR1);
    raise(SIGUSR1);

    return 0;
}
