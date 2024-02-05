#ifndef ARRAY_LIST_H
#define ARRAY_LIST_H

#include <lib-types.h>

#ifndef ARRAYLISTDEF
# define ARRAYLISTDEF
#endif // ARRAYLISTDEF

/* void arrayForEach(ArrayList *array, item, i) */
#define arrayForEach(array, item)                                                                                      \
    for (int keep = 1, count = 0, size = (arrayLength(array)); keep && count != size; keep = !keep, count++)           \
        for (item = (arrayGet(array, count)); keep; keep = !keep)

typedef void(CleanupFunc)(void *);

typedef struct ArrayList {
    u32          length;
    u32          capacity;
    u32          ptr;
    usize        itemSize;
    void       **items;
    CleanupFunc *cleanup;
} ArrayList;

int        arrayInit(ArrayList *array, usize itemSize, u32 capacity, CleanupFunc *cleanupfn);
ArrayList *arrayMalloc(usize itemSize, u32 capacity, CleanupFunc *cleanupfn);
ArrayList  arrayCreate(usize itemSize, u32 capacity, CleanupFunc *cleanupfn);
void       arrayFree(ArrayList *array);
void       arrayShrinkToFit(ArrayList *array);
u32        arrayPushBack(ArrayList *array, void *value);
void      *arrayPopBack(ArrayList *array);
void      *arrayPopFront(ArrayList *array);
u32        arrayPushFront(ArrayList *array, void *item);
string     arrayJoin(const ArrayList *array, cstr glue);
void       arrayJoinBuffer(const ArrayList *strings, string buffer, cstr glue);
ArrayList *arrayReverse(const ArrayList *array);
ArrayList *arrayConcat(const ArrayList *a, const ArrayList *b, CleanupFunc *cleanup);
u32        arrayMerge(ArrayList *target, const ArrayList *source);
ArrayList *arraySlice(const ArrayList *array, u32 start, u32 end);
ArrayList *arrayCopy(const ArrayList *array);
void      *arrayGet(const ArrayList *array, u32 i);
u32        arraySet(ArrayList *array, u32 i, void *item);
void      *arrayFirst(const ArrayList *array);
void      *arrayLast(const ArrayList *array);
void      *arrayNext(ArrayList *array);
void      *arrayCurrent(const ArrayList *array);
void       arrayReset(ArrayList *array);
u32        arrayLength(const ArrayList *array);
void       arrayClear(ArrayList *array);
ArrayList *arrayFilter(const ArrayList *array, bool(predicate)(void *item));

void       sortStringArray(ArrayList *array);
void       printStringsArray(const ArrayList *array);

#endif // ARRAY_LIST_H
