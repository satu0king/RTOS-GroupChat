#include <arpa/inet.h>

#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "chat.h"

int connectionId = -1;

void *connection_handler(void *nsd) {
    int nsfd = *(int *)nsd;

    struct Message message;

    while (read(nsfd, &message, sizeof(message))) {
        printf("%s: %s", message.name, message.message);
    }

    printf("Disconnected from server");
    exit(0);
    return NULL;
}

int main() {
    struct sockaddr_in server, client;
    pthread_t threads;

    int IP = inet_addr("172.16.130.123"); // INADDR_ANY;

    int sd = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = IP;
    server.sin_port = htons(5555);

    char name[20];
    printf("Your Handler: ");
    struct JoinRequest request;
    scanf("%s", request.name);

    if(connect(sd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect()");
        exit(0);
    }

    write(sd, &request, sizeof(request));
    struct JoinResponse response;
    read(sd, &response, sizeof(response));

    connectionId = response.id;
    printf("Id assigned by server: %d\n", connectionId);

    if (pthread_create(&threads, NULL, connection_handler, (void *)&sd) < 0) {
        perror("pthread_create()");
        exit(0);
    }

    struct Message message;
    message.id = connectionId;
    strcpy(message.name, request.name);

    strcpy(message.message, "I Joined the Chat\n");
    write(sd, &message, sizeof(message));
    fgets(message.message, sizeof(message.message), stdin); // some hack to remove trailing spaces

    while (1) {
        fgets(message.message, sizeof(message.message), stdin); 
        write(sd, &message, sizeof(message));
    }
}
