#include <stdlib.h>
#include <string.h>
#include "queue.h"

void QueueInitialize(Queue* Q, size_t sizeOfItem){
	Q->size = 0;
	Q->sizeOfItem = sizeOfItem;
	Q->front = NULL;
	Q->back = NULL;
}


void QueueInsert(Queue* Q, void* data){
	struct queueNode* newNode = malloc(sizeof(struct queueNode));
	newNode->next = NULL;
	newNode->data = malloc(Q->sizeOfItem);
	memcpy(newNode->data,data,Q->sizeOfItem);
	
	if(!Q->size){
		Q->front = Q->back = newNode;
	}
	else{
		Q->back = Q->back->next = newNode;
	}
	Q->size++;
}

const void* QueueFront(Queue* Q){
	if(!Q->front)return NULL;
	return Q->front->data;
}

void QueuePop(Queue* Q){
	struct queueNode* nodeToBePopped = Q->front;
	if(!nodeToBePopped)return;
	Q->front = Q->front->next;
	Q->size--;
	if(!Q->size)Q->back = NULL;

	free(nodeToBePopped->data);
	free(nodeToBePopped);
}

int QueueSize(Queue* Q){
	return Q->size;
}

void QueueDestruct(Queue* Q){
	while(QueueSize(Q)){
		QueuePop(Q);
	}
}









