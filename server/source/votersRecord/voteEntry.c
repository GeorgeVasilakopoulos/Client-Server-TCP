#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "voteEntry.h"

/*
voteEntry is a structure that stores two (dynamically allocated) strings
	The name of the voter (voterName)
	The name of the party that they voted for (party)
*/


//In our hashtable structure, voteEntries are hashed according to voterName
int voteEntry_hashFunction(const void* data){
	const voteEntry* entry = (const voteEntry*)data;
	unsigned int sum = 0;
	for(int i=0; i < strlen(entry->voterName); i++){
		sum += entry->voterName[i];
	}
	return sum;
}


//Two voteEntries are considered 'equal' if voterName is the same
int voteEntry_comparator(const void* data1, const void* data2){
	const voteEntry* entry1 = (voteEntry*)data1;
	const voteEntry* entry2 = (voteEntry*)data2;

	return (!strcmp(entry1->voterName,entry2->voterName));
}

void voteEntry_constructor(voteEntry* entry, const char* voterName, const char* party){
	entry->voterName = malloc(sizeof(char)*(strlen(voterName)+1));
	strcpy(entry->voterName,voterName);

	entry->party = malloc(sizeof(char)*(strlen(party)+1));
	strcpy(entry->party,party);
}

void voteEntry_destructor(void* data){
	voteEntry* entry = (voteEntry*)data;
	free(entry->party);
	free(entry->voterName);
}

//Print voteEntry to file
FILE* saveVoteFile;	//Careful with initialization
void saveVoteToFile(void* data){
	voteEntry* entry = (voteEntry*)data;
	fprintf(saveVoteFile,"%s %s\n",entry->voterName,entry->party);
}
