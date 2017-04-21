//
// Created by Amrik Sadhra on 16/02/2017.
//

#ifndef QUEUEIMPLEMENTATION_BUFFER_H
#define QUEUEIMPLEMENTATION_BUFFER_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "dmm_config.h"


/* --------- Queue Structures -------------*/
typedef struct QueueNode {
    char *strBuf;
    struct QueueNode *next, *prev;
} QueueNode;

typedef struct Queue {
    int length;
    int maxLength;
    char *name;
    struct QueueNode *head, * tail;
} Queue;

/* -------- Function Prototypes -------- */
Queue *QueueConstructor(int maxLength, char* name);
QueueNode *NodeConstructor(QueueNode *prev, char *toStore);
void EnqueueString(Queue *targetQueue, char *toStore);
char *DequeueString(Queue *targetQueue);
void displayQueue(Queue *targetQueue);

#endif //QUEUEIMPLEMENTATION_BUFFER_H
