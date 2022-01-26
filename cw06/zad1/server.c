#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <errno.h>
#include "header.h"
#include <string.h>
#include <signal.h>

typedef struct {
    long id;
    long key;
    int queue_id;
    int available;
    long rec_key;
} client;

key_t server_key;
int server_queue;
int server_running = 1;

client *clients;
long clients_number = 0;

custom_msgbuf rec_buf;
custom_msgbuf send_buf;


void add_queue(long client_id, char *queue_id) {
    sscanf(queue_id, "%d", &clients[client_id].queue_id);
}

int get_client_queue(long client_id) {
    return clients[client_id].queue_id;
}

long check_and_add(long client_key) {
    int present = 0;
    for (int i = 0; i < clients_number; i++) {
        if (clients[i].key == client_key) {
            present = 1;
            break;
        }
    }
    if (present == 0) {
        clients_number++;
        clients = realloc(clients, clients_number * sizeof(client));
        clients[clients_number - 1].key = client_key;
        clients[clients_number - 1].available = 1;
        clients[clients_number - 1].id = clients_number - 1;

        send_buf.mtype = INIT;
        send_buf.sender_id = server_key;
        sprintf(send_buf.mtext, "%ld", clients_number - 1);
        msgsnd(get_client_queue(clients_number - 1), (void *) &send_buf, msgbuf_size, IPC_NOWAIT);
    }
    return clients_number - 1;
}


long get_client_receiver(long client_id) {
    return clients[client_id].rec_key;
}

int check_if_available(long client_id) {
    return clients[client_id].available;
}

void set_receiver(long client_id, long rec_id) {
    clients[client_id].rec_key = rec_id;
    clients[client_id].available = 0;
}

void disconnect_clients(long client_id) {
    clients[client_id].rec_key = -1;
    clients[client_id].available = 1;
}

void disconnect_pair(long first, long second) {
    disconnect_clients(first);
    disconnect_clients(second);

    send_buf.mtype = DISCONNECT;
    send_buf.sender_id = server_key;

    msgsnd(get_client_queue(first), (void *) &send_buf, msgbuf_size, IPC_NOWAIT);
    msgsnd(get_client_queue(second), (void *) &send_buf, msgbuf_size, IPC_NOWAIT);
}

void purge_clients_data(long client_id) {
    if (clients[client_id].available == 0) {
        disconnect_pair(client_id, clients[client_id].rec_key);
    }
    clients[client_id].rec_key = -1;
    clients[client_id].available = 0;
    clients[client_id].queue_id = -1;
    clients[client_id].key = -1;
}

void list_clients(char *res) {
    for (int i = 0; i < clients_number; i++) {
        if (clients[i].key != -1) {
            sprintf(res + strlen(res), "%ld %d\n", clients[i].id, clients[i].available);
        }
    }
}


void stop_server() {
    for (int i = 0; i < clients_number; i++) {
        if (clients[i].key != -1) {
            send_buf.mtype = STOP;
            msgsnd(clients[i].queue_id, (void *) &send_buf, msgbuf_size, IPC_NOWAIT);
        }
    }

    msgctl(server_queue, IPC_RMID, NULL);
    free(clients);
    exit(0);
}

void init_server() {
    signal(SIGINT, stop_server);

    if ((server_key = ftok("/home", INIT)) == -1) {
        exit(errno);
    }
    if ((server_queue = msgget(server_key, IPC_CREAT | 0666)) == -1) {
        exit(errno);
    }
    printf("Server started\n");
}

int main() {
    init_server();

    while (server_running) {
        if (msgrcv(server_queue, &rec_buf, msgbuf_size, -MAX_TYPE, MSG_NOERROR) > 0) {
            if (rec_buf.mtype == INIT) {
                long client_id = check_and_add(rec_buf.sender_id);
                add_queue(client_id, rec_buf.mtext);
                send_buf.mtype = INIT;
                sprintf(send_buf.mtext, "%ld", client_id);
                msgsnd(get_client_queue(client_id), (void *) &send_buf, msgbuf_size, IPC_NOWAIT);
                printf("New client; client id: %ld\n", client_id);
            }
            if (rec_buf.mtype == LIST) {
                printf("Command: LIST from: %d\n", rec_buf.sender_id);
                send_buf.mtype = LIST;
                send_buf.sender_id = server_key;
                char tmp_text[MAX_MSG_SIZE];
                tmp_text[0] = '\0';
                list_clients(tmp_text);
                strcpy(send_buf.mtext, tmp_text);

                msgsnd(get_client_queue(rec_buf.sender_id), (void *) &send_buf, msgbuf_size, IPC_NOWAIT);
            }
            if (rec_buf.mtype == CONNECT) {
                printf("Command: CONNECT from: %d\n", rec_buf.sender_id);
                long second_client_id;
                sscanf(rec_buf.mtext, "%ld", &second_client_id);
                printf("--connecting with: %ld\n", second_client_id);
                if (check_if_available(second_client_id) == 1 && check_if_available(rec_buf.sender_id)) {
                    set_receiver(rec_buf.sender_id, second_client_id);
                    set_receiver(second_client_id, rec_buf.sender_id);

                    send_buf.mtype = CONNECT;
                    send_buf.sender_id = server_key;

                    sprintf(send_buf.mtext, "%d", get_client_queue(second_client_id));
                    msgsnd(get_client_queue(rec_buf.sender_id), (void *) &send_buf, msgbuf_size, IPC_NOWAIT);

                    sprintf(send_buf.mtext, "%d", get_client_queue(rec_buf.sender_id));
                    msgsnd(get_client_queue(second_client_id), (void *) &send_buf, msgbuf_size, IPC_NOWAIT);

                }


            }
            if (rec_buf.mtype == DISCONNECT) {
                printf("Command: DISCONNECT from: %d\n", rec_buf.sender_id);
                long second_client_id = get_client_receiver(rec_buf.sender_id);
                disconnect_pair(rec_buf.sender_id, second_client_id);

            }
            if (rec_buf.mtype == STOP) {
                printf("Command: STOP from: %d\n", rec_buf.sender_id);
                purge_clients_data(rec_buf.sender_id);
            }
        }
    }


    return 0;
}
