//
// Created by Amrik Sadhra on 16/02/2017.
//

#include "queue.h"
#define DEBUG 1

QueueNode *NodeConstructor(QueueNode *prev, char toStore[]) {
    QueueNode *myNode;

    myNode = malloc(sizeof(QueueNode));

    myNode->strBuf = toStore;
    myNode->next = NULL;
    myNode->prev = prev;

    return myNode;
}

Queue *QueueConstructor(int maxLength, char* name) {
    Queue *myQueue;

    myQueue = malloc(sizeof(Queue));
    myQueue->name = name;
    myQueue->length = 0;
    myQueue->maxLength = maxLength;
    myQueue->head = NULL;
    myQueue->tail = myQueue->head;

    return myQueue;
}

void Enqueue(Queue *targetQueue, QueueNode *toQueue) {
    toQueue->prev = targetQueue->tail;
    toQueue->next = NULL;
    targetQueue->tail = toQueue;

    //If the node to enqueue has no prev, should be head
    if (toQueue->prev == NULL) {
        targetQueue->head = toQueue;
    } else {
        toQueue->prev->next = toQueue;
    }

    targetQueue->length++;
}

void EnqueueString(Queue *targetQueue, char *toStore) {
    QueueNode *toQueue = NodeConstructor(NULL, toStore);

    if (targetQueue->length == targetQueue->maxLength) {
#ifdef DEBUG
        printf("[%s] Queue is full! Dequeuing tail. DROPPING: %s\n", targetQueue->name, DequeueString(targetQueue));
#endif
    }

    Enqueue(targetQueue, toQueue);
}

QueueNode *Dequeue(Queue *targetQueue) {
    QueueNode *temp = targetQueue->head;

    if (temp == NULL) {
        return NodeConstructor(NULL, "Queue is Empty!\n");
    } else if (temp->next != NULL) {
        targetQueue->head = temp->next;
        targetQueue->head->prev = NULL;
        targetQueue->length--;
    } else {
        //If queue is empty, make head and tail null
        targetQueue->head = NULL;
        targetQueue->tail = NULL;
    }

    return temp;
}

char *DequeueString(Queue *targetQueue) {
    QueueNode *temp = Dequeue(targetQueue);

    char *buf = temp->strBuf;

    if (temp->next == NULL) {
        free(temp);
    }

    return buf;
}

void displayQueue(Queue *targetQueue){
    QueueNode *temp = targetQueue->head;

    if(temp == NULL){
        printf("%s Queue is empty!\n", targetQueue->name);
    }

    printf("Displaying %s Queue:\n", targetQueue->name);

    while(temp->next != NULL){
        printf("%s\n",temp->strBuf);
        temp = temp->next;
    }
    printf("%s\n",temp->strBuf); //Print tail
}
