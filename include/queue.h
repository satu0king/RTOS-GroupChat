#pragma once 
#include "messages.h"

struct Queue{
    int l;
    int r; 
    int size; 
    struct Message *queue;
};

struct Queue * init_queue(int size) {
    struct Queue * q = malloc(sizeof(struct Queue));
    q->l = 0;
    q->r = 0; 
    q->size = size + 1;
    q->queue = malloc(sizeof(struct Message) * q->size);
    return q;
};

void delete_queue(struct Queue *q ) {
    free(q->queue);
    free(q);
}

int next(int i, struct Queue *q) {
    return (i+1) % q->size;
}

int queue_is_full(struct Queue *q) {
    return next(q->r, q) == q->l; 
}

int queue_is_empty(struct Queue *q) {
    return q->l == q->r;
}

int insert_queue(struct Queue * q,struct  Message msg) {
    if(queue_is_full(q)) {
        perror("Queue Full");
        return -1;
    }
    q->queue[q->r] = msg;
    q->r = next(q->r, q);
    return 0;
}

struct Message pop_queue(struct Queue * q) {
    if(queue_is_empty(q)) {
        perror("Queue Empty");
    }
    struct Message response = q->queue[q->l];
    q->l = next(q->l, q);
    return response;
}
