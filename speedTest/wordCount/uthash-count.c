#include "uthash.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// http://www.cse.yorku.ca/~oz/hash.html
static uint64_t djb2(char *str) {
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
	UT_hash_handle hh;
};

static bool readEntry(FILE *input, struct entry *entry) {
	if(fscanf(input, "%128s", entry->line) == EOF) {
		return false;
	}
	entry->hash = djb2(entry->line);
	entry->counter = 0;
	return true;
}

struct entry ENTRY_MALLOC_TABLE[1 << 20];
int ENTRY_MALLOC_INDEX = 0;

int main(int argc, char **argv) {
	FILE *input = fopen(argv[1], "r");
	
	struct entry *entries = NULL;
	
	struct entry *entryRead = &ENTRY_MALLOC_TABLE[ENTRY_MALLOC_INDEX ++], *entryFound, *tmp;
	while(readEntry(input, entryRead)) {
		HASH_FIND_INT(entries, &entryRead->hash, entryFound);
		if(!entryFound) {
			entryFound = entryRead;
			entryRead = &ENTRY_MALLOC_TABLE[ENTRY_MALLOC_INDEX ++];
			HASH_ADD_INT(entries, hash, entryFound);
		}
		++entryFound->counter;
	}
	
	HASH_ITER(hh, entries, entryFound, tmp) {
		char buffer[1024];
		snprintf(buffer, sizeof(buffer), "%.5d %s\n", entryFound->counter, entryFound->line);
	}
	
	return 0;
}
