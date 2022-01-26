#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <wait.h>


int go = 1;
int received_sig_no = 0;
pid_t pid = -1;
pid_t sender_pid = -1;
int mode = 0; //0 - kill, 1 - sigqueue, 2 - sigrt
int sent_back = 0;
int signal1 = SIGUSR1;
int signal2 = SIGUSR2;

void sig1_handler(int sig, siginfo_t *info, void *vcontext) {

    sender_pid = info->si_pid;
    if (info->si_value.sival_int == 1 && received_sig_no == 0) {
        mode = 1;
    }else if (info->si_value.sival_int == 2 && received_sig_no == 0) {
        mode = 2;
        signal1 = SIGRTMIN;
        signal2 = SIGRTMIN+1;
    }
    else {
        received_sig_no++;
        kill(sender_pid, signal1);
    }
}

void sig2_handler(int sig, siginfo_t *info, void *vcontext) {
    go = 0;

    for (int i = 0; i < received_sig_no; i++) {
        if (mode == 0) {
            kill(sender_pid, signal1);
        } else if (mode == 1) {
            union sigval value;
            sent_back++;
            value.sival_int = sent_back;
            sigqueue(sender_pid, signal1, value);
        } else if (mode == 2) {
            kill(sender_pid, signal1);
        }
    }
    if (mode == 0) {
        kill(sender_pid, signal2);
    } else if (mode == 1) {
        union sigval value;
        value.sival_int = sent_back;
        sent_back++;
        sigqueue(sender_pid, signal2, value);
    } else if (mode == 2) {
        kill(sender_pid, signal2);
    }
}

int main(int argc, char **argv) {
    printf("Catcher PID: %d\n", getpid());


    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGUSR1);
    sigdelset(&mask, SIGUSR2);
    sigdelset(&mask, SIGRTMIN);
    sigdelset(&mask, SIGRTMIN+1);
    sigdelset(&mask, SIGRTMIN);

    sigset_t lock_mask;
    sigfillset(&lock_mask);

    struct sigaction received_sig1;
    received_sig1.sa_handler = (void (*)(int)) sig1_handler;
    received_sig1.sa_flags = SA_SIGINFO;
    received_sig1.sa_mask = lock_mask;

    struct sigaction received_sig2;
    received_sig2.sa_handler = (void (*)(int)) sig2_handler;
    received_sig2.sa_flags = SA_SIGINFO;
    received_sig1.sa_mask = lock_mask;

    sigaction(SIGUSR1, &received_sig1, NULL);
    sigaction(SIGUSR2, &received_sig2, NULL);
    sigaction(SIGRTMIN, &received_sig1, NULL);
    sigaction(SIGRTMIN+1, &received_sig2, NULL);

    while (go) {
        sigsuspend(&mask);
    }
    
    printf("Signals received in catcher: %d\n", received_sig_no);


    return 0;
}
