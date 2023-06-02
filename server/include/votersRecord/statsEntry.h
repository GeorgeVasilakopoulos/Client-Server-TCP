


typedef struct {
	char* party;
	int votesCount;
}statsEntry;

int statsEntry_hashFunction(const void* data);
int statsEntry_comparator(const void* data1, const void* data2);
void statsEntry_destructor(void* data);
// statsEntry* writePtr;
void writeStatsToBuffer(void* data);


