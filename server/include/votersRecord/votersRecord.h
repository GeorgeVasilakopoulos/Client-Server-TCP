#define MAX_NAME_LENGTH 100

typedef struct{
	Hashtable votersTable;
	char pollLogName[20];
	char logStatsName[20];
}votersRecord;



void InitializeRecord(votersRecord* record, const char* pollLogName, const char* logStatsName);
const char* FindPartyOfVoter(votersRecord* record, const char*voterName);
int InsertVote(votersRecord* record, const char*voterName, const char* party);
int RemoveVote(votersRecord* record, const char*voterName);
int loadRecordFromFile(votersRecord* record, const char* filename);
int saveToPollLog(votersRecord* record);
int saveToPollStats(votersRecord* record);
void DestructRecord(votersRecord* record);