#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <wait.h>

int sig_no;
int received_sig_no;
int go = 1;
int catchers_signals_no = 0;
int signal1 = SIGUSR1;
int signal2 = SIGUSR2;

int waiting = 1;
int confirmed = 0;

pid_t catcher_pid = -1;

void sig1_handler(int sig, siginfo_t *info, void *vcontext) {
    waiting = 0;
    received_sig_no++;
    catchers_signals_no = info->si_value.sival_int;
}

void sig2_handler(int sig, siginfo_t *info, void *vcontext) {
    go = 0;
    catchers_signals_no = info->si_value.sival_int;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        exit(1);
    }

    catcher_pid = atoi(argv[1]);
    if (catcher_pid < 0) {
        exit(2);
    }

    sig_no = -1;
    sig_no = atoi(argv[2]);
    if (sig_no < 0) {
        exit(3);
    }

    char *mode = argv[3];


    if (strcmp(mode, "KILL") == 0 || strcmp(mode, "SIGQUEUE") == 0) {
        signal1 = SIGUSR1;
        signal2 = SIGUSR2;
    } else if (strcmp(mode, "SIGRT") == 0) {
        signal1 = SIGRTMIN;
        signal2 = SIGRTMIN+1;
    }

    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, signal1);
    sigdelset(&mask, signal2);
    sigdelset(&mask, SIGINT);

    sigset_t lock_mask;
    sigfillset(&lock_mask);

    struct sigaction received_sigusr1;
    received_sigusr1.sa_handler = (void (*)(int)) sig1_handler;
    received_sigusr1.sa_flags = SA_SIGINFO;
    received_sigusr1.sa_mask = lock_mask;

    struct sigaction received_sigusr2;
    received_sigusr2.sa_handler = (void (*)(int)) sig2_handler;
    received_sigusr2.sa_flags = SA_SIGINFO;
    received_sigusr1.sa_mask = lock_mask;

    sigaction(signal1, &received_sigusr1, NULL);
    sigaction(signal2, &received_sigusr2, NULL);



    if (strcmp(mode, "KILL") == 0 || strcmp(mode, "SIGRT") == 0) {

        if(strcmp(mode, "SIGRT") == 0){
            union sigval value;
            value.sival_int = 2;
            sigqueue(catcher_pid, signal1, value);
        }

        while(confirmed < sig_no) {
            kill(catcher_pid, signal1);
            while (waiting);
            waiting = 1;
            confirmed++;
        }
        kill(catcher_pid, signal2);

        while (go);

        printf("Confirmed signals number: %d\n", confirmed);

    } else if (strcmp(mode, "SIGQUEUE") == 0) {
        union sigval value;
        value.sival_int = 1;
        while (confirmed < sig_no) {

            value.sival_int = -1;
            sigqueue(catcher_pid, signal1, value);
            while (waiting);
            waiting = 1;
            confirmed++;
        }
        sigqueue(catcher_pid, signal2, value);

        while (go);

        printf("Confirmed signals number: %d\n", confirmed);
    }

    return 0;
}
