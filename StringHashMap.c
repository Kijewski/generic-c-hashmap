/*
 * AUTHOR:  René Kijewski  (rene.<surname>@fu-berlin.de)
 * LICENSE: MIT
 */

#include <stdlib.h>

#include "StringHashMap.h"

static uint32_t stringHash(const stringHashMapEntry *entry) {
	static uint32_t hash[] = { _HASHMAP_PRIMES };
	uint32_t result = -1u;
	size_t index = 0;
	for(char *string = entry->value; *string; ++string) {
		uint32_t o = result;
		result ^= o << 5;
		result ^= o >> (32-5);
		result += *string * hash[index++];
		if(index >= (sizeof(hash) / sizeof(hash[0]))) {
			index = 0;
		}
	}
	return result;
}

static uint32_t stringCmp(const stringHashMapEntry *left, const stringHashMapEntry *right) {
	return strcmp(left->value, right->value);
}

DECLARE_HASHMAP(stringHashMap, stringCmp, stringHash, free, realloc)
