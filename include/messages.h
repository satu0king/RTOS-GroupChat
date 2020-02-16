#pragma once 

struct JoinRequest{
    char groupName[20];
    char name[20];
};

struct JoinResponse{
    int id;
    int groupId;
};

enum Mode { DEV, TEST, PROD };

struct Message {
    int id;
    int groupId;
    struct timeval time;
    char name[20];
    char message[200];
};


