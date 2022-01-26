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
#include <fcntl.h>
#include <mqueue.h>

key_t server_key, client_key;
char client_queue_name[10];
char receiver_queue_name[10];
long client_id;
long receiver_id;
int server_queue, client_queue, receiver_queue;
int client_running = 1;
int direct_connection = 0;
char send_buf[MAX_MSG_SIZE - 2];
char rec_buf[MAX_MSG_SIZE];

void stop_client(int sig_no) {
    mq_close(client_queue);
    mq_unlink(client_queue_name);
    sprintf(send_buf, "%d@%ld&", STOP, client_id);
    mq_send(server_queue, send_buf, sizeof(send_buf), STOP);
    mq_close(server_queue);
    exit(0);
}

void init_client() {
    signal(SIGINT, stop_client);

    srand(time(NULL));

    server_queue = mq_open(SERVER_QUEUE, O_WRONLY);

    client_queue_name[0] = '/';
    client_queue_name[9] = '\0';
    for (int i = 1; i < 9; i++) {
        client_queue_name[i] = rand() % 26 + 65;
    }

    struct mq_attr atr;
    atr.mq_maxmsg = 5;
    atr.mq_msgsize = MAX_MSG_SIZE - 1;
    client_queue = mq_open(client_queue_name, O_RDWR | O_CREAT | O_NONBLOCK, 0666, &atr);

    sprintf(send_buf, "%d@", INIT);
    sprintf(send_buf + strlen(send_buf), "%s&", client_queue_name);

    mq_send(server_queue, send_buf, sizeof(send_buf), INIT);
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
            char tmp[100];
            fgets(tmp, 100, stdin);

            char copy[100];
            char *command;

            strcpy(copy, tmp);

            command = strtok(copy, " \n\t");

            if ((strcmp(command, "LIST")) == 0) {
                sprintf(send_buf, "%d@%ld&", LIST, client_id);
                mq_send(server_queue, send_buf, sizeof(send_buf), LIST);

            } else if ((strcmp(command, "CONNECT")) == 0) {
                char *addr = strtok(NULL, " \n\t");
                sscanf(addr, "%ld", &receiver_id);
                sprintf(send_buf, "%d@%ld&%ld&", CONNECT, client_id, receiver_id);
                mq_send(server_queue, send_buf, sizeof(send_buf), CONNECT);
            } else if ((strcmp(command, "DISCONNECT")) == 0) {
                sprintf(send_buf, "%d@%ld&", DISCONNECT, client_id);
                mq_send(server_queue, send_buf, sizeof(send_buf), DISCONNECT);
            } else if ((strcmp(command, "STOP")) == 0) {
                stop_client(SIGINT);
            } else if (direct_connection != 0) {
                sprintf(send_buf, "%d@%ld&%s&", DATA, client_id, tmp);
                mq_send(receiver_queue, send_buf, sizeof(send_buf), DATA);
            }
        }

        if (mq_receive(client_queue, rec_buf, sizeof(rec_buf), NULL) > 0) {
            char tmp[MAX_MSG_SIZE];
            strcpy(tmp, rec_buf);
            char *tok = strtok(tmp, "&");
            char *data = strtok(NULL, "&");
            char header[12];
            strcpy(header, tok);
            char *type = strtok(header, "@");
            int type_num;
            sscanf(type, "%d", &type_num);

            if (type_num == INIT) {
                long rec_client_id;
                sscanf(data, "%ld", &rec_client_id);
                client_id = rec_client_id;
                printf("Client id: %ld\n", client_id);
            }
            if (type_num == LIST) {
                printf("Lista klientów:\n%s", data);
            }
            if (type_num == CONNECT) {
                sscanf(data, "%s", receiver_queue_name);
                receiver_queue = mq_open(receiver_queue_name, O_WRONLY, 0666);
                direct_connection = 1;
                printf("-----Direct connection started-----\n\n");
            }
            if (type_num == DISCONNECT) {
                direct_connection = 0;
                mq_close(receiver_queue);
                receiver_queue_name[0] = '\0';
                receiver_queue = -1;
                receiver_id = -1;
                printf("\n-----Direct connection stopped-----\n");
            }
            if (type_num == STOP) {
                stop_client(SIGINT);
            }
            if (type_num == DATA) {
                printf(">>%s", data);
            }
        }
    }


    return 0;
}
