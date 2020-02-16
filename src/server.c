#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "messages.h"

#define MAX_GROUPS 5
#define MAX_GROUP_SIZE 200

int connections[MAX_GROUPS][MAX_GROUP_SIZE];
char groupNames[MAX_GROUPS][20];
int connectionCount[MAX_GROUPS];
int groupCount = 0;
int sd;

void killServer() {
    printf(
        "Are you sure you want to close the server ? All groups will be "
        "deleted (Y/N) \n");
    char response;
    scanf("%c", &response);
    if (response == 'Y' || response == 'y') {
        printf("Closing Server\n");
        for (int i = 0; i < groupCount; i++) {
            for (int j = 0; j < connectionCount[i]; j++) {
                close(connections[i][j]);
            }
        }
        close(sd);
        exit(EXIT_SUCCESS);
    }
}

void handle_my(int sig) {
    switch (sig) {
        case SIGINT:
            killServer();
            break;
    }
}

void *connection_handler(void *_connection) {
    int nsfd = *(int *)_connection;

    struct JoinRequest request;
    struct JoinResponse response;
    read(nsfd, &request, sizeof(request));

    char *name = request.name;
    char *groupName = request.groupName;

    int flag = 1;
    int groupId;
    int id;
    for (int i = 0; i < groupCount; i++) {
        if (!strcmp(groupName, groupNames[i])) {
            groupId = i;
            id = connectionCount[i]++;
            connections[groupId][id] = nsfd;
            flag = 0;
            break;
        }
    }

    if (flag) {
        int i = groupCount++;
        strcpy(groupNames[i], request.groupName);
        groupId = i;
        id = connectionCount[i]++;
        connections[groupId][id] = nsfd;
    }

    response.id = id;
    response.groupId = groupId;

    write(nsfd, &response, sizeof(response));

    // printf(
    //     "Client %s joined the chat, ID assigned: %d, Group Id Assigned: %d\n",
    //     name, response.id, response.groupId);

    struct Message message;
    while (read(nsfd, &message, sizeof(message))) {
        for (int i = 0; i < connectionCount[groupId]; i++) {
            if (~connections[groupId][i] && i != message.id) {
                write(connections[groupId][i], &message, sizeof(message));
            }
        }
    }

    // printf("%s with connection ID %d has left the chat\n", name, id);
    connections[groupId][id] = -1;
    free(_connection);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <port>", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    signal(SIGINT, handle_my);
    socklen_t clientLen;
    pthread_t threads;
    struct sockaddr_in server, client;

    sd = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    int true = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int));

    if (bind(sd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    listen(sd, 5);

    // printf("Waiting for the client...\n");

    clientLen = sizeof(client);

    while (1) {
        int *nsd = malloc(sizeof(int));
        *nsd = accept(sd, (struct sockaddr *)&client, &clientLen);
        if (pthread_create(&threads, NULL, connection_handler, (void *)nsd) <
            0) {
            perror("pthread_create()");
            exit(0);
        }
        // printf("Connection Created\n");
    }

    pthread_exit(NULL);
    close(sd);
}
