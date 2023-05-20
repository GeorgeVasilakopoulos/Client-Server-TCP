struct queueNode{
	void* data;
	struct queueNode* next;
};


typedef struct fifoQueue{
	int size;
	int sizeOfItem;
	struct queueNode* front;
	struct queueNode* back;
}Queue;

void QueueInitialize(Queue* Q, size_t sizeOfItem);
void QueueInsert(Queue* Q, void* data);
const void* QueueFront(Queue* Q);
void QueuePop(Queue* Q);
int QueueSize(Queue* Q);
void QueueDestruct(Queue* Q);
