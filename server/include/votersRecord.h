#define MAX_NAME_LENGTH 100

typedef struct{
	Hashtable votersTable;
	char pollLogName[20];
	char logStatsName[20];
}votersRecord;



typedef struct{
	char* voterName;
	char* party;
}voteEntry;




void InitializeRecord(votersRecord* record, const char* pollLogName, const char* logStatsName);
const char* FindParty(votersRecord* record, const char*voterName);
int InsertRecord(votersRecord* record, const char*voterName, const char* party);
int RemoveRecord(votersRecord* record, const char*voterName);
int loadRecordFromFile(votersRecord* record, const char* filename);
int saveToPollLog(votersRecord* record);
int saveToPollStats(votersRecord* record);
void DestructRecord(votersRecord* record);