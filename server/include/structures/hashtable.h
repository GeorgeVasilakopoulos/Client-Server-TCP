#define BUCKETNUMBER 100

struct tableEntry{
	void* data;
	struct tableEntry* next;
};



typedef struct hashtable{
	struct tableEntry*bucket[BUCKETNUMBER];
	size_t sizeOfItem;
	int itemsCount;
	int(*hashFunction)(const void*);
}Hashtable;




void hashInit(Hashtable* table, size_t sizeOfItem, int (*hashFunction)(const void*));
int hashSize(Hashtable* table);
void hashInsert(Hashtable* table, void* data);
const void* hashFind(Hashtable* table, void* data, int (*compare)(const void*, const void*));
void hashRemove(Hashtable* table, void* data, int (*compare)(const void*, const void*), void (*destructor)(void* data));
void hashDestruct(Hashtable* table);
void hashVisitData(Hashtable* table, void*data, void(*visit)(void*), int (*compare)(const void*, const void*));
void hashVisitAllData(Hashtable* table, void(*visit)(void*));
