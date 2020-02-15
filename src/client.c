#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "messages.h"

int sd;
struct Message message;

enum Mode { DEV, TEST, PROD };

enum Mode mode = PROD;
int logfileFD;

struct timeval getTime() {
    struct timeval timestamp;
    gettimeofday(&timestamp, NULL);
    return timestamp;
}

void killServer() {
    printf("Are you sure you want to close the client ? (Y/N) \n");
    char response;
    scanf("%c", &response);
    if (response == 'Y' || response == 'y') {
        close(sd);
        if (mode == TEST) {
            close(logfileFD);
        }
        exit(EXIT_SUCCESS);
    }
}

void handle_my(int sig) {
    switch (sig) {
        case SIGINT:
            killServer();
            break;
        case SIGUSR1:
            printf("I received a message\n");
            strcpy(message.message, "This is a message!!\n");
            message.time = getTime();
            write(sd, &message, sizeof(message));
            break;
    }
}

void *connection_handler(void *nsd) {
    int nsfd = *(int *)nsd;

    struct Message message;

    while (read(nsfd, &message, sizeof(message))) {
        printf("%s: %s", message.name, message.message);
        if (mode == DEV || mode == TEST) {
            struct timeval receiveTime = getTime();
            struct timeval sendTime = message.time;
            unsigned long delay =
                (receiveTime.tv_sec - sendTime.tv_sec) * 1000000 +
                receiveTime.tv_usec - sendTime.tv_usec;
            printf("Delay: %lu\n", delay);
            if (mode == TEST) {
                // char log[100];
                // sprintf(log, "%lu", delay);
                write(logfileFD, &delay, sizeof(delay));
            }
        }
        fflush(stdout);
    }

    printf("Disconnected from server\n");
    exit(0);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf(
            "Usage: %s <server ID> <server port> <user handle> <group handle> "
            "[mode [logfile]]",
            argv[0]);
        exit(EXIT_FAILURE);
    }

    if (argc >= 6) {
        if (!strcmp(argv[5], "TEST")) {
            mode = TEST;

            logfileFD = open(argv[6], O_WRONLY | O_APPEND | O_CREAT, 0744);
            if (logfileFD < 0) {
                perror("open\n");
                exit(EXIT_FAILURE);
            }
        } else if (!strcmp(argv[5], "DEV")) {
            mode = DEV;
        }
    }

    signal(SIGINT, handle_my);
    signal(SIGUSR1, handle_my);

    struct sockaddr_in server, client;
    pthread_t threads;

    int IP = inet_addr(argv[1]);  // INADDR_ANY;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = IP;
    server.sin_port = htons(atoi(argv[2]));

    char *name = argv[3];
    char *groupName = argv[4];

    struct JoinRequest request;
    strcpy(request.name, name);
    strcpy(request.groupName, groupName);

    if (connect(sd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect()");
        exit(EXIT_FAILURE);
    }

    write(sd, &request, sizeof(request));
    struct JoinResponse response;
    read(sd, &response, sizeof(response));

    int connectionId = response.id;
    int groupId = response.groupId;

    printf("Id assigned by server: %d\n", connectionId);

    if (pthread_create(&threads, NULL, connection_handler, (void *)&sd) < 0) {
        perror("pthread_create()");
        exit(EXIT_FAILURE);
    }

    message.id = connectionId;
    message.groupId = groupId;

    strcpy(message.name, name);

    if (mode == DEV) {
        strcpy(message.message, "I Joined the Chat\n");
        message.time = getTime();
        write(sd, &message, sizeof(message));
    }

    // fgets(message.message, sizeof(message.message), stdin); // some hack to
    // remove trailing spaces

    while (1) {
        fgets(message.message, sizeof(message.message), stdin);
        message.time = getTime();
        write(sd, &message, sizeof(message));
    }
}
