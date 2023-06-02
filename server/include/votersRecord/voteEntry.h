typedef struct{
	char* voterName;
	char* party;
}voteEntry;

int voteEntry_hashFunction(const void* data);
int voteEntry_comparator(const void* data1, const void* data2);
void voteEntry_constructor(voteEntry* entry, const char* voterName, const char* party);
void voteEntry_destructor(void* data);
void saveVoteToFile(void* data);