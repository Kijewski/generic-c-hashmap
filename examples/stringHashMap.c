/*
 * AUTHOR:  René Kijewski  (rene.<surname>@fu-berlin.de)
 * LICENSE: MIT
 */

#include <stdlib.h>

#include "stringHashMap.h"

// http://www.cse.yorku.ca/~oz/hash.html
static uint64_t djb2(char **entry) {
	unsigned long hash = 5381;
	for(char *c = *entry; *c; ++c) {
		hash = ((hash << 5) + hash) + *c;
	}
	return hash;
}

#define STRING_CMP(left, right) strcmp(*left, *right)

DECLARE_HASHMAP(stringHashMap, STRING_CMP, djb2, free, realloc)
