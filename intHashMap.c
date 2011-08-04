/*
 * AUTHOR:  René Kijewski  (rene.<surname>@fu-berlin.de)
 * LICENSE: MIT
 */

#include <stdlib.h>

#include "intHashMap.h"

#define INTCMP(left, right) left==right ? 0 : left>right ? -1 : +1
#define INTHASH(entry) entry

DECLARE_HASHMAP(stringHashMap, INTCMP, INTHASH, free, realloc)
