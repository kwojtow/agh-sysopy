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
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include "header.h"

int player_no;
int net_socket;
char client_message[256];
char server_response[256];
int game_data = 0;
int player_active = 1;
pthread_t receiver;


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void disconnect() {
    char tmp[256];
    tmp[0] = DISCONNECT;

    send(net_socket, tmp, sizeof(tmp), 0);

    close(net_socket);
    exit(0);

}
void data_handling() {
    while (player_active) {
        pthread_mutex_lock(&mutex);
        recv(net_socket, &server_response, sizeof(server_response), 0);
        if (server_response[0] != PING) {
            if(server_response[0] == DISCONNECT){
                printf("Second player left the game\n");
                disconnect();
            }
            game_data = 1;
            pthread_cond_broadcast(&cond);
        }
        else{
            client_message[0] = PING;
            send(net_socket, client_message, sizeof(client_message), 0);
        }
        pthread_mutex_unlock(&mutex);
        usleep(1);
    }
}


int open_local_socket(char *filename) {

    int server_socket_local;
    server_socket_local = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un server_name;
    server_name.sun_family = AF_UNIX;
    strncpy(server_name.sun_path, filename, sizeof(server_name.sun_path));
    server_name.sun_path[sizeof(server_name.sun_path) - 1] = '\0';

    int connection_status = connect(server_socket_local, (struct sockaddr *) &server_name, sizeof(server_name));
    if (connection_status != 0) {
        perror("Błąd przy połączeniu: ");
    }

    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
//    recv(server_socket_local, &server_response, sizeof(server_response), 0);

    printf("Server's responese: %s\n", server_response);

    return server_socket_local;
}

int open_net_socket(char *server_address_string) {

    int network_socket;
    network_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    inet_aton(server_address_string, &server_address.sin_addr);

    int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    if (connection_status != 0) {
        perror("Błąd przy połączeniu: ");
    }
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
    //recv(network_socket, &server_response, sizeof(server_response), 0);

    printf("Server's responese: %s\n", server_response);
    return network_socket;
}

void print_table(int *table) {
    char out[9];
    for (int i = 0; i < 9; i++) {
        if (table[i] == 0) {
            out[i] = ' ';
        }
        if (table[i] == 1) {
            out[i] = 'X';
        }
        if (table[i] == 2) {
            out[i] = 'O';
        }
    }
    printf("\n %c | %c | %c \n-----------\n %c | %c | %c \n-----------\n %c | %c | %c \n\n", out[0], out[1], out[2],
           out[3], out[4], out[5], out[6], out[7], out[8]);
}

int check_if_win(int *t, int n) {
    if ((t[0] == n && t[1] == n && t[2] == n) ||
        (t[3] == n && t[4] == n && t[5] == n) ||
        (t[6] == n && t[7] == n && t[8] == n) ||
        (t[0] == n && t[3] == n && t[6] == n) ||
        (t[1] == n && t[4] == n && t[7] == n) ||
        (t[2] == n && t[5] == n && t[8] == n) ||
        (t[0] == n && t[4] == n && t[8] == n) ||
        (t[2] == n && t[4] == n && t[6] == n)) {
        return 1;
    }
    return 0;
}

int check_if_moving_possible(int* t){
    int res = 0;
    for(int i = 0; i < 9; i++){
        if(t[i] == 0){
            res = 1;
            break;
        }
    }
    return res;
}


int main(int argc, char **argv) {
    signal(SIGINT, disconnect);
    pthread_create(&receiver, NULL, (void *) &data_handling, NULL);

    int table[9];
    for (int i = 0; i < 9; i++) {
        table[i] = 0;
    }

    if (argc != 4) {
        printf("Wrong args number\n");
        exit(1);
    }

    char name[250];
    strcpy(name, argv[1]);

    char connection_type[256];
    strcpy(connection_type, argv[2]);
    if (strcmp(connection_type, "net") && strcmp(connection_type, "local")) {
        printf("Wrong connection type\n");
        exit(1);
    }

    char server_address_string[256];
    strcpy(server_address_string, argv[3]);

    if (strcmp(connection_type, "net") == 0) {
        net_socket = open_net_socket(server_address_string);
    }

    if (strcmp(connection_type, "local") == 0) {
        net_socket = open_local_socket(server_address_string);
    }

    //SENDING NAME TO SERVER
    sprintf(client_message, "0|%s", name);
    send(net_socket, client_message, sizeof(client_message), 0);

    //RECEIVING DATA ABOUT NAME ACCEPTAIION
    pthread_mutex_lock(&mutex);
    while (game_data == 0) {
        pthread_cond_wait(&cond, &mutex);
    }
    game_data = 0;
    printf("%s\n", server_response);
    if (server_response[0] == USED_NAME) {
        close(net_socket);
        exit(0);
    }
    pthread_mutex_unlock(&mutex);

    //WAITING FOR SECOND PLAYER
    pthread_mutex_lock(&mutex);
    while (server_response[0] != '1' && server_response[0] != '2') {
        pthread_cond_wait(&cond, &mutex);
        printf("%s\n", server_response);
    }
    char tmp[2];
    tmp[0] = server_response[0];
    tmp[1] = '\0';
    player_no = atoi(tmp);
    if(player_no == 1){
        printf("Your sign is \'X\'\n");
    }
    if(player_no == 2){
        printf("Your sign is \'O\'\n");
    }
    pthread_mutex_unlock(&mutex);

    print_table(table);

    //FIRST MOVE IF PLAYER NUMBER EQUALS 1
    if (player_no == 1) {
        int move;
        printf("Your turn\n");
        scanf("%d", &move);


        table[move - 1] = player_no;

        print_table(table);

        sprintf(client_message, "%c%d", MOVE, move);
        send(net_socket, client_message, sizeof(client_message), 0);
    }


    //MAIN GAME LOOP
    while (1) {
        printf("Waiting for opponent's move\n");

        pthread_mutex_unlock(&mutex);
        pthread_cond_wait(&cond, &mutex);

        printf("%s\n", server_response);

        int enemy_move;
        sscanf(server_response, "1%d", &enemy_move);
        table[enemy_move - 1] = player_no % 2 + 1;
        pthread_mutex_unlock(&mutex);

        print_table(table);

        if (check_if_win(table, player_no % 2 + 1)) {
            printf("YOU LOSE\n");
            break;
        }

        if (check_if_moving_possible(table) == 0) {
            printf("DEAD-HEAT\n");
            break;
        }

        int move;
        printf("Your turn\n");
        scanf("%d", &move);

        while (table[move - 1] != 0 || move < 1 || move > 9) {
            printf("Invalid move\n");
            scanf("%d", &move);
        }
        table[move - 1] = player_no;

        print_table(table);


        sprintf(client_message, "%c%d", MOVE, move);
        send(net_socket, client_message, sizeof(client_message), 0);

        if (check_if_win(table, player_no)) {
            printf("YOU WIN\n");
            sprintf(client_message, "%c%d", WIN, player_no);
            send(net_socket, client_message, sizeof(client_message), 0);
            break;
        }

        if (check_if_moving_possible(table) == 0) {
            printf("DEAD-HEAT\n");
            sprintf(client_message, "%c", DRAW);
            send(net_socket, client_message, sizeof(client_message), 0);
            break;
        }
    }
    player_active = 0;
    close(net_socket);

    return 0;
}
