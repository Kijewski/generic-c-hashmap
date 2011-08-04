/*
 * AUTHOR:  René Kijewski  (rene.<surname>@fu-berlin.de)
 * LICENSE: MIT
 */

#ifndef HASHMAP_H__
#define HASHMAP_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef enum {
    HMDR_FIND,    // returns old entry in parameter entry
    HMDR_REPLACE, // puts new entry, replaces current entry if exists
    HMDR_SWAP,    // puts new entry, swappes old entry with **entry otherwise
    HMDR_STACK,   // put an duplicate input the map (later you have to call
                  // find-delete multiple times)
} HashMapDuplicateResolution;

typedef enum {
    HMPR_FAILED,   // map could not grow
    HMPR_FOUND,    // item already existed and was stored in *entry
    HMPR_REPLACED, // item already existed and was replace, *extry set NULL
    HMPR_SWAPPED,  // item already existed and was swapped with **entry
    HMPR_STACKED,  // new item was stacked in map, old value stored in *entry
    HMPR_PUT,      // new item was added to map
} HashMapPutResult;

typedef enum {
    _HMNPR_NOT_NEEDED,
    _HMNPR_FAIL,
    _HMNPR_GREW,
} _HashMapNextPrimeResult;

// http://oeis.org/A014234
// Buckets should mostly contain one element (if the hash function is good), so
// I put in 1 instead of 2.
#define _HASHMAP_PRIMES 1, 3, 7, 13, 31, 61, 127, 251, 509, 1021, 2039,        \
                        4093, 8191, 16381, 32749, 65521, 131071, 262139,       \
                        524287, 1048573, 2097143, 4194301, 8388593, 16777213,  \
                        33554393, 67108859, 134217689, 268435399, 536870909,   \
                        1073741789, 2147483647

#define _HashStructure(VALUE_TYPE)                                             \
struct {                                                                       \
    size_t      size;                                                          \
    uint8_t     nth_prime;                                                     \
    VALUE_TYPE *entries;                                                       \
}

/**
 * Defines hashmap helper functions for type NAME.
 * \param NAME Typedef'd name of the HashMap type.
 * \param TYPE Type of the values to store.
 */
#define DEFINE_HASHMAP(NAME, TYPE)                                             \
                                                                               \
extern const size_t _##NAME##Primes[];                                         \
                                                                               \
typedef TYPE _HashType##NAME;                                                  \
typedef _HashStructure(_HashType##NAME) NAME##Bucket;                          \
typedef _HashStructure(NAME##Bucket)    NAME;                                  \
                                                                               \
/* Initializes an empty hashmap.                                             */\
/* An null'ed map is initalized too, but has an empty capacity (which grows  */\
/* automatically.)                                                           */\
/* \param map [Out] Map to initialize                                        */\
/* \param initialCapacity [In] Hint, how much capacity to reserve.           */\
/* \return false, if it not worked                                           */\
bool NAME##New(NAME *map,                                                      \
               size_t initialCapacity);                                        \
                                                                               \
/* Destroys a hash map. (The map will have an capacity of 0 after his call.) */\
/* \param map Map to destroy.                                                */\
void NAME##Destroy(NAME *map);                                                 \
                                                                               \
/* Ensures the map can hold capacity much entries.                           */\
/* \param map Map to grow if needed.                                         */\
/* \param entry Entry to remove.                                             */\
/* \return false, if could not ensure size.                                  */\
bool NAME##EnsureSize(NAME *map,                                               \
                      size_t capacity);                                        \
                                                                               \
/* Looks up an entry in a map.                                               */\
/* \param map Map to search in.                                              */\
/* \param entry Entry to search                                              */\
/* \return Found entry or NULL                                               */\
_HashType##NAME NAME##Find(const NAME *map,                                    \
                           _HashType##NAME entry);                             \
                                                                               \
/* Adds an entry into a map.                                                 */\
/* \param map Map to add to.                                                 */\
/* \param entry Entry add. If duplicate, return it in here.                  */\
/* \return false, if map could not grow                                      */\
HashMapPutResult NAME##Put(NAME *map,                                          \
                           _HashType##NAME *entry,                             \
                           HashMapDuplicateResolution dr);                     \
                                                                               \
/* Removes an entry for the list.                                            */\
/* \param map Map to remove from.                                            */\
/* \param entry [In/out] Entry to remove, returns removed entry.             */\
/* \return false, if did not exist                                           */\
bool NAME##Remove(NAME *map,                                                   \
                  _HashType##NAME *entry);

/**
 * To iterate over all entries in order they are safe in the map.
 * You must not insert or delete elements in this loop.
 * You can use continue and break as in usual for-loops.
 * 
 * You HAVE TO put braces:
 *     HASHMAP_FOR_EACH(iter, map) {
 *         do_something();
 *     } HASHMAP_FOR_EACH_END
 *  It's meant as a feature ...
 * 
 * \param NAME Defined name of map
 * \param ITER _HashType##NAME* denoting the current element.
 * \param MAP Map to iterate over.
 */
#define HASHMAP_FOR_EACH(NAME, ITER, MAP)                                      \
    do {                                                                       \
        if(!map.entries && map.size) {                                         \
            break;                                                             \
        }                                                                      \
        for(size_t __i = 0, __broke = false; !__broke &&                       \
                               __i < _##NAME##Primes[map.nth_prime]; ++__i) {  \
            if(!map.entries[__i].entries) {                                    \
                continue;                                                      \
            }                                                                  \
            for(size_t __h = 0; !__broke && __h < map.entries[__i].size;       \
                                                                      ++__h) { \
                ITER = &map.entries[__i].entries[__h];                         \
                __broke = true;                                                \
                do

/**
 * Closes a HASHMAP_FOR_EACH(...)
 */
#define HASHMAP_FOR_EACH_END                                                   \
                while( (__broke = false) );                                    \
            }                                                                  \
        }                                                                      \
    } while(0);

/**
 * Like HASHMAP_FOR_EACH(ITER, MAP), but you are safe to delete elements during
 * the loop. You deleted elements may or may not show up during the for-loop!
 */
#define HASHMAP_FOR_EACH_SAFE_TO_DELETE(NAME, ITER, MAP)                       \
    do {                                                                       \
        if(!map.entries && map.size) {                                         \
            break;                                                             \
        }                                                                      \
        for(size_t __i = 0, __broke = false; !__broke &&                       \
                               __i < _##NAME##Primes[map.nth_prime]; ++__i) {  \
            if(!map.entries[__i].entries) {                                    \
                continue;                                                      \
            }                                                                  \
            const size_t __size = map.entries[__i].size;                       \
            _HashType##NAME __entries[__size];                                 \
            memcpy(__entries, &map.entries[__i].entries, sizeof(__entries));   \
            for(size_t __h = 0; !__broke && __h < __size; ++__h) {             \
                ITER = &map.entries[__i].entries[__h];                         \
                __broke = true;                                                \
                do

/**
 * Closes a HASHMAP_FOR_EACH_SAFE_TO_DELETE(...)
 */
#define HASHMAP_FOR_EACH_SAFE_TO_DELETE_END HASHMAP_FOR_EACH_END

/**
 * Declares the hash map functions.
 * \param NAME Typedef'd name of the HashMap type.
 * \param CMP Value comparator function. Could be easily a macro.
 * \param GET_HASH inttype (*getHash)(const _HashType##NAME entry). Could be
 *                 easily a macro.
 * \param FREE free() to use
 * \param REALLOC realloc() to use
 */
#define DECLARE_HASHMAP(NAME, CMP, GET_HASH, FREE, REALLOC)                    \
                                                                               \
const size_t _##NAME##Primes[] = { _HASHMAP_PRIMES, 0 };                       \
                                                                               \
bool NAME##New(NAME *map,                                                      \
               size_t initialCapacity) {                                       \
    map->size = 0;                                                             \
    map->nth_prime = 0;                                                        \
    map->entries = NULL;                                                       \
    return NAME##EnsureSize(map, initialCapacity);                             \
}                                                                              \
                                                                               \
void NAME##Destroy(NAME *map) {                                                \
    if(map->entries && map->size) {                                            \
        size_t capacity = _##NAME##Primes[map->nth_prime];                     \
        for(size_t i = 0; i < capacity; ++i) {                                 \
            void *entries = map->entries[i].entries;                           \
            if(entries) {                                                      \
                FREE(entries);                                                 \
            }                                                                  \
        }                                                                      \
    }                                                                          \
    FREE(map->entries);                                                        \
    map->size = 0;                                                             \
    map->nth_prime = 0;                                                        \
    map->entries = NULL;                                                       \
}                                                                              \
                                                                               \
/* Looks for prime p: capacity <= 2^n <= p < 2^(n+1)                         */\
/* \param capacity Capacity to ensure.                                       */\
/* \param entries Boolean flag, if nth_prime_ is meaningful.                 */\
/* \param entries nth_prime_ [In/out] current capacity, see _HASHMAP_PRIMES  */\
/* \param newSize_ [Out] p (see description)                                 */\
static _HashMapNextPrimeResult _##NAME##NextPrime(size_t capacity,             \
                                                  const void *entries,         \
                                                  uint8_t *nth_prime_,         \
                                                  size_t *newSize_) {          \
    size_t oldSize = _##NAME##Primes[*nth_prime_];                             \
    if(!capacity || (entries && oldSize >= capacity)) {                        \
        return _HMNPR_NOT_NEEDED;                                              \
    }                                                                          \
    int nth_prime = 0;                                                         \
    size_t newSize;                                                            \
    while((newSize = _##NAME##Primes[nth_prime]) < capacity) {                 \
        if(!newSize) {                                                         \
            return _HMNPR_FAIL;                                                \
        }                                                                      \
        ++nth_prime;                                                           \
    }                                                                          \
    *nth_prime_ = nth_prime;                                                   \
    *newSize_ = newSize;                                                       \
    return _HMNPR_GREW;                                                        \
}                                                                              \
                                                                               \
/* Helper function that puts an entry into the map, with checking the size   */\
/* or minding duplicates.                                                    */\
/* \param map Map to put entry into.                                         */\
/* \param entry Entry to insert in map.                                      */\
/* \return FALSE if bucket could not grow                                    */\
static bool _##NAME##PutReal(NAME *map,                                        \
                             _HashType##NAME entry) {                          \
    NAME##Bucket *bucket = &map->entries[((size_t) GET_HASH(entry)) %          \
                                         _##NAME##Primes[map->nth_prime]];     \
    uint8_t nth_prime = bucket->nth_prime;                                     \
    size_t newSize = 0;                                                        \
    switch(_##NAME##NextPrime(bucket->size+1, bucket->entries, &nth_prime,     \
                                                               &newSize)) {    \
        case _HMNPR_FAIL:                                                      \
            return false;                                                      \
        case _HMNPR_GREW:                                                      \
            bucket->entries = REALLOC(bucket->entries, newSize);               \
            break;                                                             \
        case _HMNPR_NOT_NEEDED:                                                \
            break;                                                             \
        default:                                                               \
            return false;                                                      \
    }                                                                          \
    bucket->entries[bucket->size ++] = entry;                                  \
    return true;                                                               \
}                                                                              \
                                                                               \
bool NAME##EnsureSize(NAME *map,                                               \
                      size_t capacity) {                                       \
    uint8_t nth_prime = map->nth_prime;                                        \
    size_t newSize = 0;                                                        \
    switch(_##NAME##NextPrime(capacity, map->entries, &nth_prime, &newSize)) { \
        case _HMNPR_FAIL:                                                      \
            return false;                                                      \
        case _HMNPR_NOT_NEEDED:                                                \
            return true;                                                       \
        case _HMNPR_GREW:                                                      \
            break;                                                             \
        default:                                                               \
            return false;                                                      \
    }                                                                          \
    NAME##Bucket *oldEntries = map->entries;                                   \
    NAME##Bucket *newEntries = (NAME##Bucket*) REALLOC(NULL,                   \
                                                       sizeof(NAME##Bucket));  \
    if(!newEntries) {                                                          \
        return false;                                                          \
    }                                                                          \
    memset(&newEntries[0], 0, sizeof(NAME##Bucket));                           \
    map->entries = newEntries;                                                 \
    map->nth_prime = nth_prime;                                                \
    /* TODO: a failed _##NAME##PutReal(...) would corrupt the map! */          \
    if(map->size) {                                                            \
        for(size_t i = 0; i < capacity; ++i) {                                 \
            NAME##Bucket *bucket = &oldEntries[i];                             \
            for(size_t h = 0; h < oldEntries[i].size; ++h) {                   \
                _##NAME##PutReal(map, oldEntries[i].entries[h]);               \
            }                                                                  \
            FREE(bucket->entries);                                             \
        }                                                                      \
    }                                                                          \
    return true;                                                               \
}                                                                              \
                                                                               \
_HashType##NAME NAME##Find(const NAME *map,                                    \
                           _HashType##NAME entry) {                            \
    if(!map->entries) {                                                        \
        return NULL;                                                           \
    }                                                                          \
    NAME##Bucket *bucket = &map->entries[((size_t) GET_HASH(entry)) %          \
                                         _##NAME##Primes[map->nth_prime]];     \
    for(size_t h = 0; h < bucket->size; ++h) {                                 \
        if(CMP(bucket->entries[h], entry) == 0) {                              \
            return bucket->entries[h];                                         \
        }                                                                      \
    }                                                                          \
    return NULL;                                                               \
}                                                                              \
                                                                               \
HashMapPutResult NAME##Put(NAME *map,                                          \
                           _HashType##NAME *entry,                             \
                           HashMapDuplicateResolution dr) {                    \
    _HashType##NAME current = NAME##Find(map, *entry);                         \
    HashMapPutResult result;                                                   \
    if(!current) {                                                             \
        current = *entry;                                                      \
        result = HMPR_PUT;                                                     \
    } else switch(dr) {                                                        \
        case HMDR_FIND:                                                        \
            *entry = current;                                                  \
            return HMPR_FOUND;                                                 \
        case HMDR_REPLACE: {                                                   \
            current = *entry;                                                  \
            return HMPR_REPLACED;                                              \
        }                                                                      \
        case HMDR_SWAP: {                                                      \
            _HashType##NAME tmp = current;                                     \
            current = *entry;                                                  \
            *entry = tmp;                                                      \
            return HMPR_SWAPPED;                                               \
        }                                                                      \
        case HMDR_STACK: {                                                     \
            _HashType##NAME tmp = *entry;                                      \
            *entry = current;                                                  \
            current = tmp;                                                     \
            result = HMPR_STACKED;                                             \
            break;                                                             \
        }                                                                      \
        default:                                                               \
            return HMPR_FAILED;                                                \
    }                                                                          \
    if(!NAME##EnsureSize(map, map->size+1) || !_##NAME##PutReal(map,           \
                                                                current)) {    \
        return HMPR_FAILED;                                                    \
    }                                                                          \
    return result;                                                             \
}                                                                              \
                                                                               \
bool NAME##Remove(NAME *map,                                                   \
                  _HashType##NAME *entry) {                                    \
    NAME##Bucket *bucket = &map->entries[((size_t) GET_HASH(*entry)) %         \
                                         _##NAME##Primes[map->nth_prime]];     \
    for(size_t nth = 0; nth < bucket->size; ++nth) {                           \
        if(CMP(*entry, bucket->entries[nth]) == 0) {                           \
            *entry = bucket->entries[nth];                                     \
            memmove(&bucket->entries[nth],                                     \
                    &bucket->entries[nth+1],                                   \
                    sizeof(NAME##Bucket[bucket->size - nth - 1]));             \
            --bucket->size;                                                    \
            return true;                                                       \
        }                                                                      \
    }                                                                          \
    return false;                                                              \
}

#endif // ifndef HASHMAP_H__

