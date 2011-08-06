/*
 * AUTHOR:  René Kijewski  (rene.<surname>@fu-berlin.de)
 * LICENSE: MIT
 */

// Compile:
// cc -Wall -Wextra -pedantic -std=gnu99 -Os stringCount.c -o stringCount

#include "../hashmap.h"
#include <stdio.h>
#include <stdlib.h>

// http://www.cse.yorku.ca/~oz/hash.html
uint64_t djb2(char *str) {
	unsigned long hash = 5381;
	char c;
	while( (c = *str++) ) {
		hash = ((hash << 5) + hash) + c;
	}
	return hash;
}

struct entry {
	uint64_t hash;
	char line[128];
	int counter;
};

bool readEntry(struct entry *entry) {
	if(scanf("%128s", entry->line) == EOF) {
		return false;
	}
	entry->hash = djb2(entry->line);
	entry->counter = 0;
	return true;
}

#define ENTRY_CMP(left, right) left->hash == right->hash ? strcmp(left->line, right->line) : 1
#define ENTRY_HASH(entry) entry->hash

DEFINE_HASHMAP(hashMap, struct entry)
DECLARE_HASHMAP(hashMap, ENTRY_CMP, ENTRY_HASH, free, realloc)

int main() {
	hashMap map;
	hashMapNew(&map);
	
	struct entry entryRead, *entryFound;
	while(readEntry(&entryRead)) {
		entryFound = &entryRead;
		HashMapPutResult result = hashMapPut(&map, &entryFound, HMDR_FIND);
		if(result == HMPR_FAILED) {
			break;
		}
		++entryFound->counter;
	}
	
	HASHMAP_FOR_EACH(hashMap, entryFound, map) {
		printf("%.5d %s\n", entryFound->counter, entryFound->line);
	} HASHMAP_FOR_EACH_END
	
	hashMapDestroy(&map);
	
	return 0;
}
