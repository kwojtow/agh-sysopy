#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <errno.h>
#include "header.h"
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

key_t server_key, client_key;
long client_id;
int server_queue, client_queue, receiver_queue;
int client_running = 1;
int direct_connection = 0;
custom_msgbuf send_buf;
custom_msgbuf rec_buf;

void stop_client(int sig_no) {
    msgctl(client_queue, IPC_RMID, NULL);
    send_buf.mtype = STOP;
    msgsnd(server_queue, (void *) &send_buf, msgbuf_size, IPC_NOWAIT);
    exit(0);
}

void init_client() {
    signal(SIGINT, stop_client);

    srand(time(NULL));


    if ((server_key = ftok("/home", INIT)) == -1) {
        exit(errno);
    }

    if ((client_key = ftok("/home", rand() % 255)) == -1) {
        exit(errno);
    }

    if ((server_queue = msgget(server_key, 0)) == -1) {
        exit(errno);
    }

    if ((client_queue = msgget(client_key, IPC_CREAT | 0666)) == -1) {
        exit(errno);
    }

    send_buf.mtype = INIT;
    send_buf.sender_id = client_key;
    sprintf(send_buf.mtext, "%d", client_queue);

    msgsnd(server_queue, (void *) &send_buf, msgbuf_size, IPC_NOWAIT);
    printf("Client started\n");
}

int main() {
    init_client();

    while (client_running) {

        fd_set fds;
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        int res = 0;
        res = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
        if (res) {
            char tmp[1000];
            fgets(tmp, 1000, stdin);

            char copy[1000];
            char *command;

            strcpy(copy, tmp);

            command = strtok(copy, " \n\t");


            if ((strcmp(command, "LIST")) == 0) {
                send_buf.mtype = LIST;
                msgsnd(server_queue, (void *) &send_buf, msgbuf_size, IPC_NOWAIT);

            } else if ((strcmp(command, "CONNECT")) == 0) {
                send_buf.mtype = CONNECT;
                char *addr = strtok(NULL, " \n\t");
                strcpy(send_buf.mtext, addr);
                msgsnd(server_queue, (void *) &send_buf, msgbuf_size, IPC_NOWAIT);
            } else if ((strcmp(command, "DISCONNECT")) == 0) {
                send_buf.mtype = DISCONNECT;
                msgsnd(server_queue, (void *) &send_buf, msgbuf_size, IPC_NOWAIT);
            } else if ((strcmp(command, "STOP")) == 0) {
                stop_client(SIGINT);
            } else if (direct_connection != 0) {
                send_buf.mtype = DATA;
                strcpy(send_buf.mtext, tmp);
                msgsnd(receiver_queue, (void *) &send_buf, msgbuf_size, IPC_NOWAIT);
            }

        }

        if ((msgrcv(client_queue, &rec_buf, msgbuf_size, -MAX_TYPE, MSG_NOERROR | IPC_NOWAIT)) > 0) {
            if (rec_buf.mtype == INIT) {
                sscanf(rec_buf.mtext, "%ld", &client_id);
                send_buf.sender_id = client_id;
                printf("Client id: %ld\n", client_id);
            }
            if (rec_buf.mtype == LIST) {
                printf("Lista klientów:\n%s", rec_buf.mtext);
            }
            if (rec_buf.mtype == CONNECT) {
                sscanf(rec_buf.mtext, "%d", &receiver_queue);
                direct_connection = 1;
                printf("-----Direct connection started-----\n\n");
            }
            if (rec_buf.mtype == DISCONNECT) {
                direct_connection = 0;
                receiver_queue = -1;
                printf("\n-----Direct connection stopped-----\n");
            }
            if (rec_buf.mtype == STOP) {
                send_buf.mtype = STOP;
                msgsnd(server_queue, (void *) &send_buf, msgbuf_size, IPC_NOWAIT);
                stop_client(SIGINT);
            }
            if (rec_buf.mtype == DATA) {
                printf(">>%s", rec_buf.mtext);
            }
        }
    }


    return 0;
}

