#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/un.h>
#include <stddef.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include "header.h"


#define MAX_CLIENTS_NO 16


char clients_names[MAX_CLIENTS_NO][200];
int clients_sockets[MAX_CLIENTS_NO];
int clients_moves[MAX_CLIENTS_NO];
int in_game[MAX_CLIENTS_NO];
int server_running = 1;
unsigned int clients_timestamps[MAX_CLIENTS_NO];

int server_socket_local;
int server_socket_net;
char address_string[256];


pthread_mutex_t socket_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t timestamp_mutex = PTHREAD_MUTEX_INITIALIZER;


void disconnect_client(int client_index){
    clients_moves[client_index] = 0;
    clients_names[client_index][0] = '\0';
    pthread_mutex_lock(&socket_mutex);
    close(clients_sockets[client_index]);
    pthread_mutex_unlock(&socket_mutex);
    clients_sockets[client_index] = -1;
    in_game[client_index] = 0;
}

void ping_clients(){
    char tmp[256];
    tmp[0] = PING;
    while (server_running) {
        sleep(1);
        for (int i = 0; i < MAX_CLIENTS_NO; i++) {
            pthread_mutex_lock(&socket_mutex);
            if (clients_sockets[i] > 0) {
                send(clients_sockets[i], tmp, sizeof(tmp), 0);
            }
            pthread_mutex_unlock(&socket_mutex);

            pthread_mutex_lock(&timestamp_mutex);
            if (time(NULL) - clients_timestamps[i] > 60) {
                disconnect_client(i);
            }
            pthread_mutex_unlock(&timestamp_mutex);
            usleep(1);
        }
    }
}

void substring(char *input, char **output, int start, int length) {
    *output = calloc(1, (length + 1) * sizeof(char));
    for (int i = 0; i < length; i++) {
        (*output)[i] = input[start + i];

    }
    (*output)[length] = '\0';
}

int max(int a, int b) {
    if (a > b)
        return a;
    return b;
}

void clean_tables() {
    for (int i = 0; i < MAX_CLIENTS_NO; i++) {
        strcpy(clients_names[i], "");
        clients_sockets[i] = -1;
        clients_moves[i] = 0;
        in_game[i] = 0;
        clients_timestamps[i] = time(NULL);
    }
}

int make_named_socket(const char *filename) {
    unlink(filename);

    int server_socket_local;
    server_socket_local = socket(AF_LOCAL, SOCK_STREAM, 0);

    struct sockaddr_un server_name;
    server_name.sun_family = AF_LOCAL;
    strcpy(server_name.sun_path, filename);
    server_name.sun_path[sizeof(server_name.sun_path) - 1] = '\0';

    bind(server_socket_local, (struct sockaddr *) &server_name, sizeof(server_name));

    return server_socket_local;
}

int make_net_socket(int port) {
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    return server_socket;
}

int check_client(char *name) {
    for (int i = 0; i < MAX_CLIENTS_NO; i++) {
        if (strcmp(clients_names[i], name) == 0) {
            return 1;
        }
    }
    return 0;
}

void accept_connection(int socket) {
    char server_message[256] = "You have reached the server";
    int client_socket;

    client_socket = accept(socket, NULL, NULL);

    for (int i = 0; i < MAX_CLIENTS_NO; i++) {
        pthread_mutex_lock(&socket_mutex);
        if (clients_sockets[i] < 0) {
            clients_sockets[i] = client_socket;
            pthread_mutex_unlock(&socket_mutex);
            break;
        }
        pthread_mutex_unlock(&socket_mutex);
    }

    send(client_socket, server_message, sizeof(server_message), 0);
}

void close_all(){
    close(server_socket_net);
    unlink(address_string);
    close(server_socket_local);
    exit(0);
}

int main(int argc, char **argv) {
    srand(time(NULL));
    clean_tables();
    signal(SIGINT, close_all);

    if (argc != 3) {
        printf("Wrong arguments number\n");
        exit(1);
    }

    int port = -1;
    sscanf(argv[1], "%d", &port);
    if (port == -1) {
        printf("Wrong port number\n");
        exit(1);
    }

    strcpy(address_string, argv[2]);

    server_socket_local = make_named_socket(address_string);
    server_socket_net = make_net_socket(port);


    listen(server_socket_net, MAX_CLIENTS_NO);
    listen(server_socket_local, MAX_CLIENTS_NO);

    pthread_t pinger;
    pthread_create(&pinger, NULL, (void*)&ping_clients, NULL);

    fd_set fds;
    struct timeval tv;
    int res;
    int max_fd = max(server_socket_net, server_socket_local);
    char buff[256];
    while (server_running) {
        FD_ZERO(&fds);
        FD_SET(server_socket_net, &fds);
        FD_SET(server_socket_local, &fds);
        tv.tv_sec = 10;
        tv.tv_usec = 0;

        for (int i = 0; i < MAX_CLIENTS_NO; i++) {
            int sd = clients_sockets[i];
            if (sd > 0)
                FD_SET(sd, &fds);

            if (sd > max_fd)
                max_fd = sd;
        }
        res = select(max_fd + 1, &fds, NULL, NULL, &tv);

        if (res) {

            if (FD_ISSET(server_socket_net, &fds)) {
                accept_connection(server_socket_net);
            }
            if (FD_ISSET(server_socket_local, &fds)) {
                accept_connection(server_socket_local);
            }

            for (int i = 0; i < MAX_CLIENTS_NO; i++) {
                int sd = clients_sockets[i];
                int pair = -1;
                if (i % 2 == 0) {
                    if (clients_sockets[i + 1] > 0) {
                        pair = i + 1;
                    }
                } else {
                    if (clients_sockets[i - 1] > 0) {
                        pair = i - 1;
                    }
                }

                if (FD_ISSET(sd, &fds)) {

                    if (read(sd, buff, 256) == 0) {
                        close(sd);
                        clients_sockets[i] = -1;
                        clients_names[i][0] = '\0';
                    } else {
                        if (buff[0] == NAME) {
                            int client_index = -1;
                            char *name;
                            for (int j = 0; j < MAX_CLIENTS_NO; j++) {
                                substring(buff, &name, 2, 250);
                                if (strcmp(clients_names[j], name) == 0) {
                                    client_index = j;
                                }
                            }
                            if (client_index == i) {
                                continue;
                            } else if (client_index == -1) {
                                strcpy(clients_names[i], name);
                                printf("New user connected: %s\n", clients_names[i]);
                            } else {
                                char tmp[256];
                                sprintf(tmp, "%c - Given name is already used", USED_NAME);
                                send(clients_sockets[i], tmp, sizeof(tmp), 0);
                                clients_sockets[i] = -1;
                                continue;
                            }
                        } else if (buff[0] == MOVE) {
                            char tmp[2];
                            tmp[0] = buff[1];
                            tmp[1] = '\0';
                            clients_moves[i] = atoi(tmp);

                            char tmp2[256];
                            sprintf(tmp2, "%c%d", MOVE, clients_moves[i]);
                            send(clients_sockets[pair], tmp2, sizeof(tmp2), 0);

                        } else if (buff[0] == WIN) {
                            printf("End of the game\n");
                            disconnect_client(i);
                            disconnect_client(pair);
                            continue;
                        } else if (buff[0] == DISCONNECT) {
                            disconnect_client(i);

                            if (pair != -1) {
                                char tmp[256];
                                tmp[0] = DISCONNECT;
                                send(clients_sockets[pair], tmp, sizeof(tmp), 0);
                            }
                            continue;
                        } else if (buff[0] == PING){
                            clients_timestamps[i] = time(NULL);
                        }
                        else if(buff[0] == DRAW){
                            printf("End of the game\n");
                            disconnect_client(i);
                            disconnect_client(pair);
                            continue;
                        }


                        if (in_game[i] == 0) {
                            char tmp[256];
                            if (pair == -1) {
                                sprintf(tmp, "Waiting for second player");
                                send(clients_sockets[i], tmp, sizeof(tmp), 0);
                            } else {
                                int no = rand();
                                sprintf(tmp, "%d - Your playing with %s", (no + 1) % 2 + 1, clients_names[pair]);
                                in_game[pair] = 1;
                                send(clients_sockets[i], tmp, sizeof(tmp), 0);
                                sprintf(tmp, "%d - Your playing with %s", no % 2 + 1, clients_names[i]);
                                in_game[i] = 1;
                                send(clients_sockets[pair], tmp, sizeof(tmp), 0);
                            }
                        }

                    }
                }
            }
        }
    }


    pthread_join(pinger, NULL);
    


    return 0;
}
