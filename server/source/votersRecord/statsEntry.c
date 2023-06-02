#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "statsEntry.h"

/*
statsEntry is a structure that stores:
	- One dynamically allocated string (party)
	- One integer, the number of votes that the corresponding party received
*/



//statEntries are hashed according to 'party' field
int statsEntry_hashFunction(const void* data){
	const statsEntry* entry = (statsEntry*)data;
	unsigned int sum = 0;
	for(int i=0; i < strlen(entry->party); i++){
		sum += entry->party[i];
	}
	return sum;
}


//Two statEntries are considered 'equal' if the field party is the same
int statsEntry_comparator(const void* data1, const void* data2){
	const statsEntry* entry1 = (const statsEntry*)data1;
	const statsEntry* entry2 = (const statsEntry*)data2;
	return (!strcmp(entry1->party,entry2->party));
}



void statsEntry_destructor(void* data){
	statsEntry* entry = (statsEntry*)data;
	free(entry->party);
}



/*
	Copy statsEntry to pointer writePtr.

	writePtr is initialized to point into a buffer and is incremented after each call 

	This function is used in order to transfer party-vote counts into a buffer
	so that they can be sorted.
*/
statsEntry* writePtr;
void writeStatsToBuffer(void* data){
	statsEntry* entry = (statsEntry*)data;
	*writePtr++ = *entry; 
}




