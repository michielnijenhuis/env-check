#ifndef ARRAY_LIST_H
#define ARRAY_LIST_H

#include "types.h"

#define UTILS_IMPLEMENTATION
#include "utils.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int        array_list__arrayResize(ArrayList *array, u32 capacity); // internal

#endif // ARRAY_LIST_H

#define ARRAY_LIST_IMPLEMENTATION

#ifdef ARRAY_LIST_IMPLEMENTATION
#ifndef ARRAY_LIST_INCLUDED
# define ARRAY_LIST_INCLUDED

int arrayInit(ArrayList *array, usize itemSize, u32 capacity, CleanupFunc *cleanupfn) {
    assert(itemSize > 0);

    array->capacity = capacity;
    array->itemSize = itemSize;
    array->ptr      = 0;
    array->length   = 0;
    array->items    = (void **) calloc(capacity, itemSize);
    array->cleanup  = cleanupfn;

    return array->items == NULL ? -1 : 0;
}

ArrayList arrayCreate(usize itemSize, u32 capacity, CleanupFunc *cleanupfn) {
    ArrayList arr;
    arrayInit(&arr, itemSize, capacity, cleanupfn);
    return arr;
}

ArrayList *arrayMalloc(usize itemSize, u32 capacity, CleanupFunc *cleanupfn) {
    ArrayList *array = malloc(sizeof(*array));

    if (array == NULL) {
        return NULL;
    }

    arrayInit(array, itemSize, capacity, cleanupfn);

    if (array->items == NULL) {
        free(array);
        return NULL;
    }

    return array;
}

void arrayFree(ArrayList *array) {
    if (array == NULL || array->items == NULL) {
        return;
    }

    if (array->cleanup) {
        for (u32 i = 0; i < array->length; ++i) {
            array->cleanup(arrayGet(array, i));
        }
    }

    free(array->items);
    array->items    = NULL;
    array->length   = 0;
    array->capacity = 0;
    array->ptr      = 0;
}

void arrayShrinkToFit(ArrayList *array) {
    if (array == NULL || array->items == NULL || array->length >= array->capacity) {
        return;
    }

    array->items    = realloc(array->items, array->length * array->itemSize);
    array->capacity = array->length;
}

u32 arrayPushBack(ArrayList *array, void *value) {
    if (array == NULL || array->items == NULL) {
        return 0;
    }

    if (array->length >= array->capacity) {
        int success = array_list__arrayResize(array, array->capacity * 2);

        if (success < 0) {
            return array->length;
        }
    }

    return arraySet(array, array->length, value);
}

void *arrayPopBack(ArrayList *array) {
    if (array == NULL || array->length == 0 || array->items == NULL) {
        return NULL;
    }

    void *value = arrayLast(array);
    arraySet(array, array->length - 1, NULL);
    array->length--;

    return value;
}

void *arrayPopFront(ArrayList *array) {
    if (array == NULL || array->length == 0 || array->items == NULL) {
        return NULL;
    }

    void *value = arrayFirst(array);

    for (u32 i = 1; i < array->length; ++i) {
        arraySet(array, i - 1, arrayGet(array, i));
    }

    array->items[array->length - 1] = NULL;
    array->length--;

    return value;
}

string arrayJoin(const ArrayList *strings, cstr glue) {
    if (strings == NULL || glue == NULL) {
        return NULL;
    }

    string str         = NULL;
    usize  count       = strings->length;
    usize  totalLength = 0;
    u16    i           = 0;

    // find total length of joined strings
    for (i = 0; i < count; i++) {
        totalLength += strlen(arrayGet(strings, i));
    }
    // account for null byte
    totalLength++;
    // account for all glues
    totalLength += strlen(glue) * (count - 1);

    str = (string) malloc(totalLength);

    if (str == NULL) {
        return NULL;
    }

    arrayJoinBuffer(strings, str, glue);

    return str;
}

void arrayJoinBuffer(const ArrayList *strings, string buffer, cstr glue) {
    if (strings == NULL || buffer == NULL || glue == NULL) {
        return;
    }

    buffer[0] = '\0';
    u32 len   = strings->length;

    // append all strings
    for (usize i = 0; i < len; i++) {
        strcat(buffer, arrayGet(strings, i));

        if (i < (len - 1)) {
            strcat(buffer, glue);
        }
    }
}

ArrayList *arrayReverse(const ArrayList *array) {
    if (array == NULL || array->items == NULL) {
        return NULL;
    }

    ArrayList *newArr = arrayMalloc(array->itemSize, array->capacity, array->cleanup);

    for (int i = array->length - 1; i >= 0; --i) {
        arrayPushBack(newArr, arrayGet(array, i));
    }

    return newArr;
}

ArrayList *arrayConcat(const ArrayList *a, const ArrayList *b, CleanupFunc *cleanup) {
    if (a == NULL || a->items == NULL || b == NULL || b->items == NULL) {
        return NULL;
    }

    u32        newArrItemSize = max(a->itemSize, b->itemSize);
    u32        newArrCapacity = a->length + b->length;

    ArrayList *newArr         = arrayMalloc(newArrItemSize, newArrCapacity, cleanup);
    memcpy(newArr->items, a->items, a->length * newArr->itemSize);
    memcpy(newArr->items + a->length, b->items, b->length * newArrItemSize);
    newArr->length = a->length + b->length;

    return newArr;
}

u32 arrayMerge(ArrayList *target, const ArrayList *source) {
    if (target == NULL || target->items == NULL) {
        return 0;
    }

    if (source == NULL) {
        return target->length;
    }

    usize requiredItemSize = max(target->itemSize, source->itemSize);

    if (requiredItemSize > target->itemSize) {
        panic("arrayMerge() error: source could not be merged into target. Incompatible item size.");
    }

    usize requiredCapacity = target->length + source->length;

    if (requiredCapacity > target->capacity) {
        int success = array_list__arrayResize(target, requiredCapacity);

        if (success < 0) {
            return target->length;
        }
    }

    memcpy(target->items + target->length, source->items, source->length * target->itemSize);
    target->length += source->length;

    return target->length;
}

ArrayList *arraySlice(const ArrayList *array, u32 start, u32 end) {
    if (array == NULL || array->items == NULL) {
        return NULL;
    }

    ArrayList *slice = malloc(sizeof(*slice));

    if (slice == NULL) {
        return NULL;
    }

    void **items  = calloc(end - start + 1, array->itemSize);

    if (items == NULL) {
        free(slice);
        return NULL;
    }

    slice->length   = 0;
    slice->items    = items;
    slice->capacity = end - start + 1;
    slice->cleanup  = array->cleanup;
    slice->ptr      = 0;
    slice->itemSize = array->itemSize;

    for (u32 i = start; i <= end; i++) {
        arrayPushBack(slice, arrayGet(array, i));
    }

    return slice;
}

u32 arrayPushFront(ArrayList *array, void *item) {
    if (array == NULL || array->items == NULL) {
        return 0;
    }

    u32 len = array->length;

    if (len == 0) {
        return arrayPushBack(array, item);
    }

    if (len >= array->capacity) {
        int success = array_list__arrayResize(array, array->capacity * 2);

        if (success < 0) {
            return array->length;
        }
    }

    for (int i = len - 1; i >= 0; --i) {
        arraySet(array, i + 1, arrayGet(array, i));
    }

    return arraySet(array, 0, item);
}

void *arrayGet(const ArrayList *array, u32 i) {
    if (array == NULL || array->items == NULL || i >= array->capacity) {
        return NULL;
    }

    return array->items[i];
}

u32 arraySet(ArrayList *array, u32 i, void *item) {
    if (array == NULL || array->items == NULL) {
        return 0;
    }

    if (array->capacity <= i) {
        return array->length;
    }

    void *current = arrayGet(array, i);
    array->items[i] = item;

    if (current == NULL) {
        array->length++;
    }

    return array->length;
}

void *arrayFirst(const ArrayList *array) {
    if (array->length == 0) {
        return NULL;
    }

    return arrayGet(array, 0);
}

void *arrayLast(const ArrayList *array) {
    if (array == NULL || array->items == NULL) {
        return NULL;
    }

    u32 len = array->length;

    if (len == 0) {
        return NULL;
    }

    return arrayGet(array, len - 1);
}

void *arrayNext(ArrayList *array) {
    void *current = arrayGet(array, array->ptr);

    if (current != NULL) {
        array->ptr++;
    }

    return current;
}

void *arrayCurrent(const ArrayList *array) {
    return arrayGet(array, array->ptr);
}

void arrayReset(ArrayList *array) {
    if (array == NULL) {
        return;
    }

    array->ptr = 0;
}

u32 arrayLength(const ArrayList *array) {
    if (array == NULL) {
        return 0;
    }

    return array->length;
}

void arrayClear(ArrayList *array) {
    if (array == NULL || array->items == NULL) {
        return;
    }

    for (u32 i = 0; i < array->length; ++i) {
        if (array->cleanup) {
            array->cleanup(arrayGet(array, i));
        }

        arraySet(array, i, NULL);
    }

    array->length = 0;
    array->ptr    = 0;
}

ArrayList *arrayCopy(const ArrayList *array) {
    if (array == NULL || array->items == NULL) {
        return NULL;
    }

    ArrayList *copy = malloc(sizeof(*copy));

    if (copy == NULL) {
        return NULL;
    }

    void **newItems = calloc(array->capacity, array->itemSize);

    if (newItems == NULL) {
        free(copy);
        return NULL;
    }

    memcpy(newItems, array->items, array->itemSize * array->capacity);

    copy->itemSize = array->itemSize;
    copy->ptr      = array->ptr;
    copy->length   = array->length;
    copy->capacity = array->capacity;
    copy->cleanup  = array->cleanup;
    copy->items    = newItems;

    return copy;
}

/* Sort array of strings using bubble sort */
void sortStringArray(ArrayList *array) {
    if (array == NULL || array->items == NULL) {
        return;
    }

    u32   len = array->length;
    void *tmp = NULL;
    for (u32 i = 0; i < len; ++i) {
        for (u32 j = 0; j < len - 1 - i; ++j) {
            if (strcmp(arrayGet(array, j), arrayGet(array, j + 1)) > 0) {
                // swap array[j] and array[j + 1]
                tmp = arrayGet(array, j);
                arraySet(array, j, arrayGet(array, j + 1));
                arraySet(array, j + 1, tmp);
            }
        }
    }
}

void printStringsArray(const ArrayList *array) {
    if (array == NULL) {
        return;
    }

    u32 i = 0;
    arrayForEach(array, string item) {
        printf("%u) %s\n", i++, item);
    }
}

ArrayList *arrayFilter(const ArrayList *array, bool(predicate)(void *item)) {
    if (array == NULL || array->items == NULL) {
        return NULL;
    }

    ArrayList *filtered = arrayMalloc(array->itemSize, array->capacity, array->cleanup);

    arrayForEach(array, void *item) {
        if (predicate(item)) {
            arrayPushBack(filtered, item);
        }
    }

    return filtered;
}

int array_list__arrayResize(ArrayList *array, u32 capacity) {
    if (array == NULL || array->items == NULL) {
        return -1;
    }

    u32 oldCapacity = array->capacity;
    assert(capacity > 0 && array->capacity < capacity);
    array->capacity = capacity;
    array->items    = (void **) realloc(array->items, capacity * array->itemSize);

    if (array->items != NULL && capacity > oldCapacity) {
        for (u32 i = oldCapacity; i < capacity; ++i) {
            array->items[i] = NULL;
        }
    }

    return 0;
}

#endif // ARRAY_LIST_INCLUDED
#endif // ARRAY_LIST_IMPLEMENTATION
