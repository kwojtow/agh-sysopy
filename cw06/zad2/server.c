#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <errno.h>
#include "header.h"
#include <string.h>
#include <signal.h>
#include <mqueue.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct {
    char queue_name[10];
    long client_id;
    mqd_t queue;
    int available;
    long receiver_id;
} client;

mqd_t server_queue;
int server_running = 1;

client *clients;
int clients_number = 0;

char rec_buf[MAX_MSG_SIZE];
char send_buf[MAX_MSG_SIZE - 2];


long check_and_add(char *queue_name) {
    int present = 0;
    for (int i = 0; i < clients_number; i++) {
        if (strcmp(clients[i].queue_name, queue_name) == 0) {
            present = 1;
            break;
        }
    }
    if (present == 0) {
        clients_number++;
        clients = realloc(clients, clients_number * sizeof(client));
        strcpy(clients[clients_number - 1].queue_name, queue_name);
        clients[clients_number - 1].queue = mq_open(clients[clients_number - 1].queue_name, O_WRONLY);
        clients[clients_number - 1].available = 1;
        clients[clients_number - 1].client_id = clients_number - 1;
    }
    return clients_number - 1;
}

mqd_t get_client_queue(long client_id) {
    return clients[client_id].queue;
}

long get_client_receiver(long client_id) {
    return clients[client_id].receiver_id;
}

int check_if_available(long client_id) {
    return clients[client_id].available;
}

void set_receiver(long client_id, long rec_id) {
    clients[client_id].receiver_id = rec_id;
    clients[client_id].available = 0;
}

void disconnect_clients(long client_id) {
    clients[client_id].receiver_id = -1;
    clients[client_id].available = 1;
}

void disconnect_pair(long first, long second) {
    disconnect_clients(first);
    disconnect_clients(second);

    sprintf(send_buf, "%d@%s", DISCONNECT, SERVER_QUEUE);
    mq_send(get_client_queue(first), send_buf, sizeof(send_buf), DISCONNECT);
    mq_send(get_client_queue(second), send_buf, sizeof(send_buf), DISCONNECT);
}

void purge_clients_data(long client_id) {
    if (clients[client_id].available == 0) {
        disconnect_pair(client_id, clients[client_id].receiver_id);
    }
    clients[client_id].receiver_id = -1;
    clients[client_id].available = 0;
    clients[client_id].queue_name[0] = '\0';
    mq_close(clients[client_id].queue);
    clients[client_id].queue = -1;
}

void list_clients(char *res) {
    for (int i = 0; i < clients_number; i++) {
        if (clients[i].queue != -1) {
            sprintf(res + strlen(res), "%ld %d\n", clients[i].client_id, clients[i].available);
        }
    }
}


void stop_server() {
    for (int i = 0; i < clients_number; i++) {
        if (clients[i].queue != -1) {
            sprintf(send_buf, "%d@%s&", STOP, SERVER_QUEUE);
            mq_send(clients[i].queue, send_buf, sizeof(send_buf), STOP);
            mq_close(clients[i].queue);
        }
    }
    mq_close(server_queue);
    mq_unlink(SERVER_QUEUE);
    exit(0);
}

void init_server() {
    signal(SIGINT, stop_server);

    struct mq_attr atr;
    atr.mq_maxmsg = 5;
    atr.mq_msgsize = MAX_MSG_SIZE - 1;
    server_queue = mq_open(SERVER_QUEUE, O_RDWR | O_CREAT | O_NONBLOCK, 0666, &atr);
    printf("Server started\n");
}

int main() {
    init_server();

    while (server_running) {
        sleep(1);
        if (mq_receive(server_queue, rec_buf, MAX_MSG_SIZE, NULL) > 0) {
            char tmp[MAX_MSG_SIZE];
            strcpy(tmp, rec_buf);
            char *tok = strtok(tmp, "&");
            char *data = strtok(NULL, "&");
            char header[12];
            strcpy(header, tok);
            char *type = strtok(header, "@");
            int type_num;
            sscanf(type, "%d", &type_num);
            char *client = strtok(NULL, "@");
            long rec_client_id;
            if (strlen(client) < 9) {
                sscanf(client, "%ld", &rec_client_id);
            }

            if (type_num == INIT) {
                long client_id = check_and_add(client);
                sprintf(send_buf, "%d@%s&%ld&", INIT, SERVER_QUEUE, client_id);
                mq_send(get_client_queue(client_id), send_buf, sizeof(send_buf), LIST);
                printf("New client; client id: %ld\n", client_id);
            }
            if (type_num == LIST) {
                printf("Command: LIST from: %ld\n", rec_client_id);
                send_buf[0] = '\0';
                sprintf(send_buf, "%d@%s&", LIST, SERVER_QUEUE);

                char tmp_text[MAX_MSG_SIZE];
                tmp_text[0] = '\0';
                list_clients(tmp_text);
                strcpy(send_buf + strlen(send_buf), tmp_text);

                mq_send(get_client_queue(rec_client_id), send_buf, sizeof(send_buf), LIST);

            }
            if (type_num == CONNECT) {
                printf("Command: CONNECT from: %ld\n", rec_client_id);
                long receiver_id;
                sscanf(data, "%ld", &receiver_id);
                printf("--connecting with: %ld\n", receiver_id);
                if (check_if_available(receiver_id) == 1 && check_if_available(rec_client_id)) {
                    set_receiver(receiver_id, rec_client_id);
                    set_receiver(rec_client_id, receiver_id);


                    sprintf(send_buf, "%d@%s&%s&", CONNECT, SERVER_QUEUE, clients[receiver_id].queue_name);
                    mq_send(get_client_queue(rec_client_id), send_buf, sizeof(send_buf), CONNECT);

                    sprintf(send_buf, "%d@%s&%s&", CONNECT, SERVER_QUEUE, clients[rec_client_id].queue_name);
                    mq_send(get_client_queue(receiver_id), send_buf, sizeof(send_buf), CONNECT);

                }


            }
            if (type_num == DISCONNECT) {
                printf("Command: DISCONNECT from: %ld\n", rec_client_id);

                long second_client = get_client_receiver(rec_client_id);
                disconnect_pair(rec_client_id, second_client);

            }
            if (type_num == STOP) {
                printf("Command: STOP from: %ld\n", rec_client_id);
                purge_clients_data(rec_client_id);
            }
        }
    }


    return 0;
}
