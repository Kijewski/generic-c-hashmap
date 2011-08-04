/*
 * AUTHOR:  René Kijewski  (rene.<surname>@fu-berlin.de)
 * LICENSE: MIT
 */

#include <stdlib.h>

#include "stringHashMap.h"

static uint32_t stringHash(char **entry) {
	static uint32_t hash[] = { _HASHMAP_PRIMES };
	uint32_t result = -1u;
	size_t index = 0;
	for(const char *string = *entry; *string; ++string) {
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

#define STRING_CMP(left, right) strcmp(*left, *right)

DECLARE_HASHMAP(stringHashMap, STRING_CMP, stringHash, free, realloc)
