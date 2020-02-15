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
#include "chat.h"

#define N 100
int connections[N];
int connectionCount = 0;
int sd; 

void killServer(){
    printf("Closing Server\n");
    close(sd);
    exit(0);
}

void handle_my(int sig){
  switch (sig) {
    case SIGINT:
        killServer();
        break;
  }

}

struct Connection {
    int nsd;
    int connectionId;
};

void *connection_handler(void *_connection) {
    struct Connection *connection = (struct Connection *)_connection;

    int nsfd = connection->nsd;
    int connectionId = connection->connectionId;

    struct JoinRequest request;
    struct JoinResponse response;
    read(nsfd, &request, sizeof(request));

    char *name = request.name;

    response.id = connectionId;
    write(nsfd, &response, sizeof(response));

    printf("Client %s joined the chat, ID assigned: %d\n", name, connectionId);

    struct Message message;
    int i;
    while (read(nsfd, &message, sizeof(message))) {
        for (i = 0; i < connectionCount; i++) {
            if (~connections[i] && i != message.id) {
                write(connections[i], &message, sizeof(message));
            }
        }
    }

    printf("%s with connection ID %d has left the chat\n", name, connectionId);
    connections[connectionId] = -1;
    free(connection);
    return NULL;
}

int main() {
    signal(SIGINT, handle_my);
    int nsd;
    socklen_t clientLen;
    pthread_t threads;
    struct sockaddr_in server, client;

    sd = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(5555);

    if(bind(sd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    }
    listen(sd, 5);

    printf("Waiting for the client...\n");

    clientLen = sizeof(client);

    while (1) {
        nsd = accept(sd, (struct sockaddr *)&client, &clientLen);
        struct Connection *connection = (struct Connection *) malloc(sizeof(struct Connection));
        connection->nsd = nsd;
        connection->connectionId = connectionCount;
        connections[connectionCount++] = nsd;
        if (pthread_create(&threads, NULL, connection_handler, (void *)connection) < 0) {
            perror("pthread_create()");
            exit(0);
        }
        printf("Connection Created\n");
    }

    pthread_exit(NULL);
    close(sd);
}
