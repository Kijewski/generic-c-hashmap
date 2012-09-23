# generic-c-hashmap

*generic-c-hashmap* should be the easiest to use hash map for C possible.

It only knows how to find, put, remove, and iterate over entries.

You don't have make any adjustments at all to the elements to store.
If you can find a hash function and an equality comparator for your data type,
you can store it in a *generic-c-hashmap*.

## TOC

* [Examples](#examples)
* [Set up](#set-up)
* API:
    * [Hash function](#hash-function)
    * [Comparator function](#comparator-function)
    * [Hashmap initialization and destruction](#hashmap-initialization-and-destruction)
    * [Data retrieval](#data-retrieval)
    * [Data modification](#data-modification)
* [Note](#Note)
* [Naming](#naming)
* [Performance](#performance)
* [Contributers](#contributers)
* [License](#license)

<a name="examples"></a>

## Examples

See [examples folder](
https://github.com/Kijewski/generic-c-hashmap/tree/master/examples).

Especially [stringCount.c](
https://github.com/Kijewski/generic-c-hashmap/blob/master/examples/stringCount.c
) is an example for a blazing fast (and I mean it!) word counter.

<a name="set-up"></a>

## Set up

*generic-c-hashmap* consists of two macros to set up your hashmap type:

    DEFINE_HASHMAP(NAME, TYPE)
    DECLARE_HASHMAP(NAME, CMP, GET_HASH, FREE, REALLOC)

`DEFINE_HASHMAP(NAME, TYPE)` defines the function prototypes and the type
`NAME`:

    typedef struct {
        size_t size;
    } NAME;
    
    void NAMENew(NAME *map);
    void NAMEDestroy(NAME *map);
    bool NAMEEnsureSize(NAME *map, size_t capacity);
    
    bool NAMEFind(const NAME *map, TYPE **entry);
    
    HashMapPutResult NAMEPut(NAME *map, TYPE **entry, HashMapDuplicateResolution dr);
    bool NAMERemove(NAME *map, TYPE *entry);

You should use DEFINE_HASHMAP in a header file (`.h`). You may put multiple
hashmap definitions in one header file.

`DECLARE_HASHMAP(NAME, CMP, GET_HASH, FREE, REALLOC)` declares the actual
function and should be put in a code file (`.c`):

* `NAME` is the same name as in DEFINE_HASHMAP
* `CMP` is your comparator function/macro
* `GET_HASH` is your comparator function/macro
* `FREE` is the free function to use, e.g. `free` or `g_free`
* `REALLOC` is the realloc function to use, e.g. `realloc` or `g_realloc`

<a name="hash-function"></a>

## Hash function

    SOME_INT_TYPE hash(TYPE *entry);

Function to calculate/retrieve entry's hash. E.g.

    static uint64_t djb2(char **entry) {
        unsigned long hash = 5381;
        for(char *c = *entry; *c; ++c) {
            hash = ((hash << 5) + hash) + *c;
        }
        return hash;
    }

for `TYPE = char*` (C strings) or identity if you store integers in your
hashmap:

    #define INT_HASH(entry) *entry

<a name="comparator-function"></a>

## Comparator function

    SOME_INT_TYPE compare(TYPE *left, TYPE *right)

Must return `0` if `left` equals `right`, and some other value otherwise. E.g.

    #define STRING_CMP(left, right) strcmp(*left, *right)
    #define INT_CMP(left, right) *left==*right ? 0 : 1

<a name="hashmap-initialization-and-destruction"></a>

## Hashmap initialization and destruction

    void NAMENew(NAME *map);

sets up a new empty hashmap.

    void NAMEDestroy(NAME *map);

removes every element from the map and sets its capacity to zero.

    bool NAMEEnsureSize(NAME *map, size_t capacity);

ensures that `map` can hold  at least `capacity` elements without need to grow.
So if you want to add `n` elements at batch, you may call
`NAMEEnsureSize(&map, map.size + n)` before.
Returns `false` if your memory is exhausted.

<a name="data-retrieval"></a>

## Data retrieval

    bool NAMEFind(const NAME *map, TYPE **entry);

Find element `*entry` an returns it pointer in `*entry`.
Returns `false` if `*entry` was not found. In latter case `*entry` will contain
a random value.

You may change value of the retrieved `*entry`but you must not change its hash!

    TYPE *iter;
    HASHMAP_FOR_EACH(NAME, iter, map) {
        do_something_with(iter);
    } HASHMAP_FOR_EACH_END

Iterates over all elements stored in the map. `iter` is the iterator.
You may use `break` and `continue` and in usual loops.
While iterating you must not add to or remove from the map!

If your map stores pointers, you may want to iterate over all elements and the
`free(*iter)` before destroying the hashmap to avoid memory leaks.

<a name="data-modification"></a>

## Data modification

    HashMapPutResult NAMEPut(NAME *map, TYPE **entry, HashMapDuplicateResolution dr);

Puts a new value `*entry` in the map. What to do if value already existed
depends on `dr`:

* `HMDR_FAIL`: NAMEPut() will return `HMPR_FAILED=0`, `*entry` contains a
  pointer to the found element.
* `HMDR_FIND`: NAMEPut() will return `HMPR_FOUND`, `*entry` contains a
  pointer to the found element.
* `HMDR_REPLACE`: NAMEPut() will return `HMPR_REPLACED`, will overwrite the
  stored element, `*entry` contains a pointer to the newly stored element in the
  map.
* `HMDR_SWAP`: NAMEPut() will return `HMPR_SWAPPED`, and exchange `**entry` and
  the old element.
* `HMDR_STACK`: NAMEPut() will return `HMPR_STACKED`, put `*entry` a second
  time, and return a pointer to the old element in `*entry`. NAMEFind() will
  only find the firstly put element!

If your memory is exhausted, NAMEPut() will return `HMDR_FAIL`. If `*entry` did
not exist in the map and was put in to it, `HMPR_PUT` will be returned.

    bool NAMERemove(NAME *map, TYPE *entry);

Removes `*entry` form the map. Returns `false` if it did not exist.
The maps capacity will never shrink.

<a name="note"></a>

## Note

*generic-c-hashmap* uses C99 syntax. Use `--std=c99` or `--std=gnu99` with gcc.

<a name="naming"></a>

## Naming

I was to lazy to come up with a name …
Maybe I will click “Random article” in Wikipedia until I find a cool name. :-)

<a name="performance"></a>

## Performance

Seems to perform in same league as widely used [uthash](
http://uthash.sourceforge.net/). See [speed tests](
https://github.com/Kijewski/generic-c-hashmap/tree/master/speedTest).

Execution time (ms):

<pre>
                -O0  -O1  -O2  -O3  -Os
gen.-c-hashmap  258  211  212  202  218
uthash          278  214  219  216  201
</pre>

Compiling time (ms):

<pre>
                -O0  -O1  -O2  -O3  -Os
gen.-c-hashmap   40   63   84  108   69
uthash          941  953  971  978  966
</pre>

<a name="contributers"></a>

## Contributers

* [René Kijewski](https://github.com/Kijewski/generic-c-hashmap):
  original author
* [Stefan Friesel](https://github.com/sfriesel):
  bug spotted
* [Pete Diemert](https://github.com/pdiemert):
  bugs spotted ([1](https://github.com/Kijewski/generic-c-hashmap/issues/1),
                [2](https://github.com/Kijewski/generic-c-hashmap/issues/2))

<a name="license"></a>

## License

MIT license, see
[LICENSE](https://github.com/Kijewski/generic-c-hashmap/blob/master/LICENSE).
In short:

* you may use *generic-c-hashmap* in your commercial applications,
* you don't have give to credit to *generic-c-hashmap*, if you use it in a
  binary distribution, but you may if you want to,
* you have to put the license file in your source code distribution and must
  attribute the contributers to *generic-c-hashmap* in the respective source code
  files.
