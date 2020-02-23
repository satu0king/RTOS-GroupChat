#pragma once
#include "messages.h"
#include "ntp.h"
#include "stdlib.h"

// Message Wrapper with insertion time to measure processing delay
struct MessageQ {
    struct Message message;
    struct timeval insertTime;
};

// Queue structure
struct Queue {
    int l;
    int r;
    int size;
    struct MessageQ *queue;
};

// queue constructor. Queue is implemented as a cyclic queue
struct Queue *init_queue(int size) {
    struct Queue *q = (struct Queue *)malloc(sizeof(struct Queue));
    q->l = 0;
    q->r = 0;
    q->size = size + 1;
    q->queue = malloc(sizeof(struct MessageQ) * q->size);
    return q;
};

// Queue destructor
void delete_queue(struct Queue *q) {
    free(q->queue);
    free(q);
}

// next index with cyclic wrap around
int next(int i, struct Queue *q) { return (i + 1) % q->size; }

// helper function to test if queue is full
int queue_is_full(struct Queue *q) { return next(q->r, q) == q->l; }

// helper function to test if queue is empty
int queue_is_empty(struct Queue *q) { return q->l == q->r; }

// queue insert function
int insert_queue(struct Queue *q, struct Message msg) {
    if (queue_is_full(q)) {
        perror("Queue Full");
        return -1;
    }

    struct MessageQ messageWrapper = {
        .message = msg,
        .insertTime = getTime()  // capture insertion time
    };

    q->queue[q->r] = messageWrapper;
    q->r = next(q->r, q);
    return 0;
}

// queue pop function
struct MessageQ pop_queue(struct Queue *q) {
    if (queue_is_empty(q)) {
        perror("Queue Empty");
    }
    struct MessageQ response = q->queue[q->l];
    q->l = next(q->l, q);
    return response;
}

int getSize(struct Queue *q) { return ((q->r) - (q->l) + q->size) % q->size; }