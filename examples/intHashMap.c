/*
 * AUTHOR:  René Kijewski  (rene.<surname>@fu-berlin.de)
 * LICENSE: MIT
 */

#include <stdlib.h>

#include "intHashMap.h"

#define INT_CMP(left, right) *left==*right ? 0 : 1
#define INT_HASH(entry) *entry

DECLARE_HASHMAP(intHashMap, INT_CMP, INT_HASH, free, realloc)
