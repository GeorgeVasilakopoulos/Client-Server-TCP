#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hashtable.h"
#include "votersRecord.h"


#define MAX_NAME_LENGTH 100


static int voteEntry_hashFunction(const void* data){
	const voteEntry* entry = (const voteEntry*)data;
	unsigned int sum = 0;
	for(int i=0; i < strlen(entry->voterName); i++){
		sum += entry->voterName[i];
	}
	return sum;
}

static int voteEntry_comparator(const void* data1, const void* data2){
	const voteEntry* entry1 = (voteEntry*)data1;
	const voteEntry* entry2 = (voteEntry*)data2;

	return (!strcmp(entry1->voterName,entry2->voterName));
}

static void voteEntry_constructor(voteEntry* entry, const char* voterName, const char* party){
	entry->voterName = malloc(sizeof(char)*(strlen(voterName)+1));
	strcpy(entry->voterName,voterName);

	entry->party = malloc(sizeof(char)*(strlen(party)+1));
	strcpy(entry->party,party);
}

static void voteEntry_destructor(void* data){
	voteEntry* entry = (voteEntry*)data;
	free(entry->party);
	free(entry->voterName);
}

typedef struct {
	char* party;
	int votesCount;
}statsEntry;

static int statsEntry_comparator(const void* data1, const void* data2){
	const statsEntry* entry1 = (const statsEntry*)data1;
	const statsEntry* entry2 = (const statsEntry*)data2;
	return (!strcmp(entry1->party,entry2->party));
}

static int statsEntry_hashFunction(const void* data){
	const statsEntry* entry = (statsEntry*)data;
	unsigned int sum = 0;
	for(int i=0; i < strlen(entry->party); i++){
		sum += entry->party[i];
	}
	return sum;
}


static void statsEntry_destructor(void* data){
	statsEntry* entry = (statsEntry*)data;
	free(entry->party);
}


void InitializeRecord(votersRecord* record, const char* pollLogName, const char* logStatsName){
	strcpy(record->pollLogName,pollLogName ? pollLogName : "");
	strcpy(record->logStatsName,logStatsName ? logStatsName : "");
	hashInit(&(record->votersTable), sizeof(voteEntry),&voteEntry_hashFunction);
}


const char* FindParty(votersRecord* record, const char*voterName){
	if(!voterName)return NULL;

	voteEntry entry;
	entry.voterName = (char*)voterName;

	const voteEntry* foundEntry = hashFind(&(record->votersTable),&entry,&voteEntry_comparator);
	return foundEntry ? foundEntry->party : NULL;
}

int InsertRecord(votersRecord* record, const char*voterName, const char* party){
	

	if(!voterName || !party)return -1;
	if(FindParty(record,voterName)){
		//already voted
		return 1;
	}

	voteEntry newEntry;
	voteEntry_constructor(&newEntry,voterName,party);
	

	hashInsert(&(record->votersTable),&newEntry);
	return 0;
}



int RemoveRecord(votersRecord* record, const char*voterName){
	if(!voterName)return 1;

	voteEntry searchEntry;
	searchEntry.voterName = (char*)voterName;


	const voteEntry* entry = hashFind(&(record->votersTable),(char*)voterName,&voteEntry_comparator);
	if(!entry)return 1;
	free(entry->voterName);
	free(entry->party);

	hashRemove(&(record->votersTable),entry->voterName,&voteEntry_comparator);

	return 0;
}

int loadRecordFromFile(votersRecord* record, const char* filename){
	FILE* fileptr = fopen(filename,"r");
	if(!fileptr){
		printf("heyy\n");
		return 1;
	}
	char voterNameBuf[100]="";
	char partyBuf[100]="";
	while(1){
		if(fscanf(fileptr,"%s",voterNameBuf)==EOF)break;
		if(fscanf(fileptr,"%s",partyBuf) == EOF)break;
		InsertRecord(record,voterNameBuf,partyBuf);
	}
	return 0;
}


static FILE* saveVoteFile;
static void saveVoteToFile(void* data){
	voteEntry* entry = (voteEntry*)data;
	fprintf(saveVoteFile,"%s %s\n",entry->voterName,entry->party);
}
static void saveStatsToFile(void* data){
	statsEntry* entry = (statsEntry*)data;
	fprintf(saveVoteFile, "%s: %d\n",entry->party,entry->votesCount);
}


int saveToPollLog(votersRecord* record){
	saveVoteFile = fopen(record->pollLogName,"w");
	hashVisitAllData(&(record->votersTable),&saveVoteToFile);
	fclose(saveVoteFile);
	return 0;
}



//Functional Sorcery
static void incrementVote(void* data){
	statsEntry* entry = (statsEntry*)data;
	entry->votesCount++;
}


static Hashtable* countVoteTable; 
static void countVote(void* data){
	voteEntry* vote = (voteEntry*)data;
	statsEntry partyEntry;
	partyEntry.party = vote->party;

	if(!hashFind(countVoteTable,&partyEntry,&statsEntry_comparator)){
		partyEntry.votesCount = 1;
		partyEntry.party = malloc(sizeof(char)*(strlen(vote->party)+1));
		strcpy(partyEntry.party,vote->party);
		hashInsert(countVoteTable,&partyEntry);
		return;
	}
	hashVisitData(countVoteTable,&partyEntry,&incrementVote,&statsEntry_comparator);
}


int saveToPollStats(votersRecord* record){
	Hashtable statsTable;
	hashInit(&statsTable, sizeof(statsEntry),&statsEntry_hashFunction);

	countVoteTable = &statsTable;
	hashVisitAllData(&(record->votersTable),&countVote);

	saveVoteFile = fopen(record->logStatsName,"w");
	hashVisitAllData(&statsTable,&saveStatsToFile);
	hashVisitAllData(&statsTable,&statsEntry_destructor);
	hashDestruct(&statsTable);
	fclose(saveVoteFile);
	return 0;
}


void DestructRecord(votersRecord* record){
	hashVisitAllData(&(record->votersTable), &voteEntry_destructor);
	hashDestruct(&(record->votersTable));
}

