#pragma once 

struct JoinRequest{
    char name[20];
};

struct JoinResponse{
    int id;
};

struct Message {
    int id;
    char name[20];
    char message[200];
};


