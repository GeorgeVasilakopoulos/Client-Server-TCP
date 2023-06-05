#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hashtable.h"
#include "votersRecord.h"
#include "voteEntry.h"
#include "statsEntry.h"

#define MAX_NAME_LENGTH 100

extern FILE* saveVoteFile;
extern statsEntry* writePtr;

void InitializeRecord(votersRecord* record, const char* pollLogName, const char* logStatsName){
	strcpy(record->pollLogName,pollLogName ? pollLogName : "");
	strcpy(record->logStatsName,logStatsName ? logStatsName : "");
	hashInit(&(record->votersTable), sizeof(voteEntry),&voteEntry_hashFunction);
}

//Returns NULL when voter is not present in the record.
const char* FindPartyOfVoter(votersRecord* record, const char*voterName){
	if(!voterName)return NULL;

	//Create a temporary entry in order to compare
	voteEntry entry;

	//Copy voterName pointer
	entry.voterName = (char*)voterName;

	const voteEntry* foundEntry = hashFind(&(record->votersTable),&entry,&voteEntry_comparator);
	return foundEntry ? foundEntry->party : NULL;
}

//Returns 1 if voter has already voted
int InsertVote(votersRecord* record, const char*voterName, const char* party){
	

	if(!voterName || !party)return -1;
	if(FindPartyOfVoter(record,voterName)){
		//already voted
		return 1;
	}

	voteEntry newEntry;
	voteEntry_constructor(&newEntry,voterName,party);
	

	hashInsert(&(record->votersTable),&newEntry);
	return 0;
}


int RemoveVote(votersRecord* record, const char*voterName){
	if(!voterName)return -1;

	voteEntry searchEntry;
	searchEntry.voterName = (char*)voterName;


	const voteEntry* entry = hashFind(&(record->votersTable),(void*)&searchEntry,&voteEntry_comparator);
	
	//Not found
	if(!entry)return 1;
	
	hashRemove(&(record->votersTable),entry->voterName,&voteEntry_comparator,&voteEntry_destructor);

	return 0;
}


//Open a file and insert votes into record structure
int loadRecordFromFile(votersRecord* record, const char* filename){
	FILE* fileptr = fopen(filename,"r");
	if(!fileptr){
		return 1;
	}
	char voterNameBuf[100]="";
	char partyBuf[100]="";
	while(1){
		if(fscanf(fileptr,"%s",voterNameBuf)==EOF)break;
		if(fscanf(fileptr,"%s",partyBuf) == EOF)break;
		InsertVote(record,voterNameBuf,partyBuf);
	}
	fclose(fileptr);
	return 0;
}




/*
	Save votes with names to pollLog file.

	This function initializes saveVoteFile and calls saveVoteToFile on every voteEntry,
	through hashVisitAllData.
*/
int saveToPollLog(votersRecord* record){
	saveVoteFile = fopen(record->pollLogName,"w");
	hashVisitAllData(&(record->votersTable),&saveVoteToFile);
	fclose(saveVoteFile);
	return 0;
}




//Sort statEntries with indeces l to r (inclusive) using mergesort
static void mergeSortStatEntries(int l, int r, statsEntry* array){
	if(l>=r)return;
	int mid = (l+r)/2;
	mergeSortStatEntries(l,mid,array);
	mergeSortStatEntries(mid+1,r,array);
	statsEntry* temparray = malloc(sizeof(statsEntry)*(r-l+1));
	int i=l;
	int j=mid+1;
	int itemsCount = 0;
	
	//Sort primarily according to votesCount and secondarily according to party
	while(i <= mid && j <= r){
		if(array[i].votesCount != array[j].votesCount)
			temparray[itemsCount++] = (array[i].votesCount > array[j].votesCount) ? array[i++]:array[j++];
		else
			temparray[itemsCount++] = (strcmp(array[i].party,array[j].party) > 0) ? array[i++]:array[j++];
	}
	while(i<=mid)temparray[itemsCount++] = array[i++];
	while(j<=r)temparray[itemsCount++] = array[j++];
	for(int i=0;i<itemsCount;i++){
		array[l+i] = temparray[i];
	}
	free(temparray);
}

/*
	countVoteTable is a global Hashtable structure that stores the votes of each party.
	Given a vote, if the party is already present in countVoteTable, increment the vote count.
	Otherwise, add the party into the global structure, with a vote count of 1
*/
Hashtable* countVoteTable; 
void countVote(void* data){
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
	
	//Locally create a new Hashtable that will store statsEntries
	Hashtable statsTable;
	hashInit(&statsTable, sizeof(statsEntry),&statsEntry_hashFunction);


	//Initialize external variable countVoteTable and count votes stored in the record
	countVoteTable = &statsTable;
	hashVisitAllData(&(record->votersTable),&countVote);

	FILE* statsFilePtr = fopen(record->logStatsName,"w");

	int numParties = hashSize(&statsTable);

	//Dynamically create a buffer to store all the statEntries
	statsEntry* sortedArray = malloc(sizeof(statsEntry)*numParties);

	//Initialize external variable writePtr and call writeStatsToBuffer on every statEntry
	writePtr = sortedArray;
	hashVisitAllData(&statsTable,&writeStatsToBuffer);


	//Sort the entries
	mergeSortStatEntries(0,numParties-1,sortedArray);

	//Print the entries to file, in sorted order.
	for(int i=0; i < numParties; i++){
		fprintf(statsFilePtr, "%s %d\n",sortedArray[i].party,sortedArray[i].votesCount);
	}
	free(sortedArray);

	//Destruct statsTable
	hashVisitAllData(&statsTable,&statsEntry_destructor);
	hashDestruct(&statsTable);
	fclose(statsFilePtr);
	return 0;
}


void DestructRecord(votersRecord* record){
	hashVisitAllData(&(record->votersTable), &voteEntry_destructor);
	hashDestruct(&(record->votersTable));
}

