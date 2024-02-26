#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

typedef void(cleanup_func)(void *);

#define DEFINE_ARRAY_LIST(__name, __type)                                                                              \
    typedef struct __name {                                                                                            \
        const char   *name;                                                                                            \
        const char   *type;                                                                                            \
        __type       *items;                                                                                           \
        cleanup_func *cleanup;                                                                                         \
        size_t        len;                                                                                             \
        size_t        cap;                                                                                             \
        size_t        ptr;                                                                                             \
        size_t        size;                                                                                            \
    } __name

#ifndef ARRAY_RESIZE_FACTOR
# define ARRAY_RESIZE_FACTOR 1.5
#endif // ARRAY_RESIZE_FACTOR

#ifndef ARRAY_MALLOC
# define ARRAY_MALLOC malloc
#endif // ARRAY_MALLOC

#ifndef ARRAY_REALLOC
# define ARRAY_REALLOC realloc
#endif // ARRAY_REALLOC

#ifndef ARRAY_FREE_FN
# define ARRAY_FREE_FN free
#endif // ARRAY_FREE_FN

#define ARRAY_CREATE(__name, __type, capacity, cleanupfn)                                                              \
    {                                                                                                                  \
        .cap = capacity, .len = 0, .ptr = 0, .size = sizeof(__type), .cleanup = cleanupfn,                             \
        .items = ARRAY_MALLOC(capacity * sizeof(__type)), .name = #__name,                                             \
    }

#define ARRAY_FOR_EACH(arr, item)                                                                                      \
    for (int keep = 1, count = 0, size = ((arr)->len); keep && count != size; keep = !keep, count++)                   \
        for (item = ((arr)->items[count]); keep; keep = !keep)

#define ARRAY_FREE(arr)                                                                                                \
    if (arr == NULL || (arr)->items == NULL) {                                                                         \
        return;                                                                                                        \
    }                                                                                                                  \
    if (ARRAY_FREE_FN != NULL) {                                                                                       \
        ARRAY_FREE_FN((arr)->items);                                                                                   \
    }                                                                                                                  \
    (arr)->items = NULL;                                                                                               \
    (arr)->cap   = 0;                                                                                                  \
    (arr)->len   = 0;                                                                                                  \
    (arr)->ptr   = 0

#define ARRAY_FREE_ITEMS(arr)                                                                                          \
    if (arr == NULL || (arr)->items == NULL) {                                                                         \
        return;                                                                                                        \
    }                                                                                                                  \
    if ((arr)->cleanup != NULL) {                                                                                      \
        for (size_t i = 0; i < (arr)->len; ++i) {                                                                      \
            (arr)->cleanup((arr)->items[i]);                                                                           \
        }                                                                                                              \
    }

#define ARRAY_FREE_ALL(arr)                                                                                            \
    if (arr == NULL || (arr)->items == NULL) {                                                                         \
        return;                                                                                                        \
    }                                                                                                                  \
    ARRAY_FREE_ITEMS(arr);                                                                                             \
    ARRAY_FREE(arr);

#define ARRAY_RESIZE(arr)                                                                                              \
    assert(arr && (arr)->cap > 0);                                                                                     \
    (arr)->items = ARRAY_REALLOC((arr)->items, ((arr)->cap * ARRAY_RESIZE_FACTOR) * (arr)->size);                      \
    assert((arr)->items != NULL);                                                                                      \
    (arr)->cap *= ARRAY_RESIZE_FACTOR

#define ARRAY_ERR_OUT_OF_BOUNDS(arr, i)                                                                                \
    fprintf(stderr,                                                                                                    \
            "[ERROR][%s:%d] Array out of bounds (%s, i: %zu, len: %zu)\n",                                             \
            __FILE__,                                                                                                  \
            __LINE__,                                                                                                  \
            (arr)->name,                                                                                               \
            (size_t) i,                                                                                                \
            (size_t) (arr)->len);                                                                                      \
    exit(1);                                                                                                           \
    /* NOT REACHED */

#define ARRAY_ASSERT_INDEX(arr, i)                                                                                     \
    assert(arr != NULL);                                                                                               \
    if (i < 0 || i >= (arr)->len) {                                                                                    \
        ARRAY_ERR_OUT_OF_BOUNDS(arr, i);                                                                               \
    }

#define ARRAY_GET(arr, i)                                                                                              \
    ARRAY_ASSERT_INDEX(arr, i);                                                                                        \
    return (arr)->items[i]

#define ARRAY_GET_SAFE(arr, i, fallback)                                                                               \
    assert(arr != NULL);                                                                                               \
    if (i >= (arr)->len) {                                                                                             \
        return fallback;                                                                                               \
    }                                                                                                                  \
    return (arr)->items[i]

#define ARRAY_SET(arr, i, value)                                                                                       \
    ARRAY_ASSERT_INDEX(arr, i);                                                                                        \
    (arr)->items[i] = value

#define ARRAY_SET_SAFE(arr, i, value)                                                                                  \
    if (i < (arr)->len) {                                                                                              \
        (arr)->items[i] = value;                                                                                       \
    }

#define ARRAY_SET_CLEAN(arr, i, value)                                                                                 \
    ARRAY_ASSERT_INDEX(arr, i);                                                                                        \
    if ((arr)->cleanup && (arr)->items[i] != NULL) {                                                                   \
        (arr)->cleanup((arr)->items[i]);                                                                               \
    }                                                                                                                  \
    return (arr)->items[i] = value

#define ARRAY_SET_CLEAN_SAFE(arr, i, value)                                                                            \
    if (i >= (arr)->len) {                                                                                             \
        return;                                                                                                        \
    }                                                                                                                  \
    if ((arr)->cleanup && (arr)->items[i] != NULL) {                                                                   \
        (arr)->cleanup((arr)->items[i]);                                                                               \
    }                                                                                                                  \
    (arr)->items[i] = value

#define ARRAY_FIRST(arr)                                                                                               \
    ARRAY_ASSERT_INDEX(arr, (size_t) 0);                                                                               \
    return (arr)->items[0];

#define ARRAY_FIRST_SAFE(arr, fallback)                                                                                \
    if ((arr)->len == 0) {                                                                                             \
        return fallback;                                                                                               \
    }                                                                                                                  \
    return (arr)->items[0]

#define ARRAY_LAST(arr)                                                                                                \
    ARRAY_ASSERT_INDEX(arr, (arr)->len - 1);                                                                           \
    return (arr)->items[(arr)->len - 1];

#define ARRAY_LAST_SAFE(arr, fallback)                                                                                 \
    if ((arr)->len == 0) {                                                                                             \
        return fallback;                                                                                               \
    }                                                                                                                  \
    return (arr)->items[(arr)->len - 1];

#define ARRAY_PUSH_BACK(arr, value)                                                                                    \
    assert(arr != NULL);                                                                                               \
    if ((arr)->len >= (arr)->cap) {                                                                                    \
        ARRAY_RESIZE(arr);                                                                                             \
    }                                                                                                                  \
    (arr)->items[(arr)->len++] = value;                                                                                \
    return (arr)->len

#define ARRAY_PUSH_FRONT(arr, value)                                                                                   \
    assert(arr != NULL);                                                                                               \
    if ((arr)->len == 0) {                                                                                             \
        (arr)->items[(arr)->len++] = value;                                                                            \
        return (arr)->len;                                                                                             \
    }                                                                                                                  \
    if ((arr)->len >= (arr)->cap) {                                                                                    \
        ARRAY_RESIZE(arr);                                                                                             \
    }                                                                                                                  \
    for (size_t i = (arr)->len; i >= 0; --i) {                                                                         \
        (arr)->items[i + 1] = (arr)->items[i];                                                                         \
    }                                                                                                                  \
    (arr)->items[0] = value;                                                                                           \
    (arr)->len += 1;                                                                                                   \
    return (arr)->len

#define ARRAY_POP_BACK(arr)                                                                                            \
    ARRAY_ASSERT_INDEX(arr, (size_t) 0);                                                                               \
    return (arr)->items[--((arr)->len)]

#define ARRAY_POP_BACK_SAFE(arr, fallback)                                                                             \
    if ((arr)->len == 0) {                                                                                             \
        return fallback;                                                                                               \
    }                                                                                                                  \
    return (arr)->items[--((arr)->len)]

#define ARRAY_POP_FRONT(arr)                                                                                           \
    ARRAY_ASSERT_INDEX(arr, (size_t) 0);                                                                               \
    for (size_t i = 1; i < (arr)->len; ++i) {                                                                          \
        (arr)->items[i - 1] = (arr)->items[i];                                                                         \
    }                                                                                                                  \
    (arr)->len -= 1;                                                                                                   \
    return (arr)->len

#define ARRAY_POP_FRONT_SAFE(arr, fallback)                                                                            \
    assert(arr != NULL);                                                                                               \
    if ((arr)->len == 0) {                                                                                             \
        return fallback;                                                                                               \
    }                                                                                                                  \
    for (size_t i = 1; i < (arr)->len; ++i) {                                                                          \
        (arr)->items[i - 1] = (arr)->items[i];                                                                         \
    }                                                                                                                  \
    (arr)->len -= 1;                                                                                                   \
    return (arr)->len

#define ARRAY_CURRENT(arr) ARRAY_GET(arr, (arr)->ptr)

#define ARRAY_NEXT(arr)                                                                                                \
    if ((arr)->ptr + 1 < (arr)->len) {                                                                                 \
        (arr)->ptr += 1;                                                                                               \
        ARRAY_CURRENT(arr);                                                                                            \
    } else {                                                                                                           \
        return NULL;                                                                                                   \
    }

#define ARRAY_RESET(arr)                                                                                               \
    assert(arr != NULL);                                                                                               \
    (arr)->ptr = 0

#define ARRAY_CLEAR(arr)                                                                                               \
    assert(arr != NULL);                                                                                               \
    if (!(arr)->cleanup) {                                                                                             \
        (arr)->len = 0;                                                                                                \
        return;                                                                                                        \
    }                                                                                                                  \
    for (size_t i = 0; i < (arr)->len; ++i) {                                                                          \
        (arr)->cleanup((arr)->items[i]);                                                                               \
        (arr)->items[i] = NULL;                                                                                        \
    }

#define ARRAY_FILL(arr, len, fill)                                                                                     \
    assert(arr != NULL);                                                                                               \
    assert((arr)->len == 0);                                                                                           \
    for (size_t i = 0; i < len; ++i) {                                                                                 \
        ARRAY_PUSH_BACK(arr, fill);                                                                                    \
    }

#define ARRAY_FILL_WITH_CALLBACK(arr, __len, cb)                                                                       \
    assert(arr != NULL);                                                                                               \
    assert(cb != NULL);                                                                                                \
    assert((arr)->len == 0);                                                                                           \
    if (__len > (arr)->cap) {                                                                                          \
        ARRAY_RESIZE(arr);                                                                                             \
    }                                                                                                                  \
    for (size_t i = 0; i < __len; ++i) {                                                                               \
        (arr)->items[(arr)->len++] = cb(i);                                                                            \
    }

#define ARRAY_REVERSE(__type, arr)                                                                                     \
    size_t i, j;                                                                                                       \
    for (i = 0, j = (arr)->len - 1; i < (arr)->len && i < j; ++i, --j) {                                               \
        __type tmp      = (arr)->items[i];                                                                             \
        (arr)->items[i] = (arr)->items[j];                                                                             \
        (arr)->items[j] = tmp;                                                                                         \
    }

#define ARRAY_COPY(__name, __type, arr)                                                                                \
    __name copy = ARRAY_CREATE(__name, __type, (arr)->len, (arr)->cleanup);                                            \
    memcpy(copy.items, (arr)->items, (arr)->len);                                                                      \
    copy.len = (arr)->len;                                                                                             \
    return copy

#define ARRAY_SHRINK_TO_FIT(arr)                                                                                       \
    assert(arr != NULL);                                                                                               \
    if ((arr)->len >= (arr)->cap) {                                                                                    \
        return;                                                                                                        \
    }                                                                                                                  \
    (arr)->items = ARRAY_REALLOC((arr)->items, (arr)->len * (arr)->size);                                              \
    (arr)->cap   = (arr)->len

#define ARRAY_FILTER(arr, predicate)                                                                                   \
    assert(arr != NULL);                                                                                               \
    assert(predicate != NULL);                                                                                         \
    size_t ptr = 0;                                                                                                    \
    for (size_t i = 0; i < (arr)->len; ++i) {                                                                          \
        if (predicate((arr)->items[i])) {                                                                              \
            (arr)->items[ptr++] = (arr)->items[i];                                                                     \
        }                                                                                                              \
    }                                                                                                                  \
    (arr)->len = ptr;

#define ARRAY_FILTER_WITH_CLEAN(arr, predicate)                                                                        \
    assert(arr != NULL);                                                                                               \
    assert(predicate != NULL);                                                                                         \
    assert((arr)->cleanup != NULL);                                                                                    \
    size_t ptr = 0;                                                                                                    \
    size_t len = (arr)->len;                                                                                           \
    for (size_t i = 0; i < len; ++i) {                                                                                 \
        if (predicate((arr)->items[i])) {                                                                              \
            (arr)->items[ptr++] = (arr)->items[i];                                                                     \
        }                                                                                                              \
    }                                                                                                                  \
    (arr)->len = ptr;                                                                                                  \
    for (; ptr < len; ++ptr) {                                                                                         \
        (arr)->cleanup((arr)->items[ptr]);                                                                             \
    }

#define ARRAY_MAP(__src_type, dest, src, callback)                                                                     \
    assert(dest != NULL);                                                                                              \
    assert(src != NULL);                                                                                               \
    assert(callback != NULL);                                                                                          \
    for (size_t i = 0; i < (arr)->len; ++i) {                                                                          \
        (dest)->items[i] = callback((src)->items[i]);                                                                  \
    }

#define ARRAY_SLICE(__name, __type, arr, start, end)                                                                   \
    assert(arr != NULL);                                                                                               \
    assert(end >= start);                                                                                              \
    assert(end < (arr)->len);                                                                                          \
    assert(start < (arr)->len);                                                                                        \
    size_t new_len = end - start + 1;                                                                                  \
    __name slice   = ARRAY_CREATE(__name, __type, new_len, (arr)->cleanup);                                            \
    memcpy(slice.items, (arr)->items + start, new_len);                                                                \
    slice.len = new_len;                                                                                               \
    return slice

#define ARRAY_CONCAT(__name, __type, a, b)                                                                             \
    assert(a != NULL);                                                                                                 \
    assert(b != NULL);                                                                                                 \
    assert(a->size == b->size);                                                                                        \
    assert(strcmp(a->type, b->type) == 0);                                                                             \
    size_t new_cap      = a->len + b->len;                                                                             \
    __name concatenated = ARRAY_CREATE(__name, __type, new_cap, a->cleanup);                                           \
    memcpy(concatenated.items, a->items, a->len * a->size);                                                            \
    memcpy(concatenated.items + a->len, b->items, b->len * b->size);                                                   \
    concatenated.len = a->len + b->len;                                                                                \
    return concatenated

#define ARRAY_MERGE(target, source)                                                                                    \
    assert(target != NULL);                                                                                            \
    assert(source != NULL);                                                                                            \
    assert(target->size == source->size);                                                                              \
    assert(strcmp(target->type, source->type) == 0);                                                                   \
    size_t required_cap = target->len + source->len;                                                                   \
    if (required_cap > target->cap) {                                                                                  \
        ARRAY_RESIZE(target);                                                                                          \
    }                                                                                                                  \
    memcpy(target->items + target->len, source->items, source->len * target->size);                                    \
    target->len += source->len;                                                                                        \
    return target->len

#define ARRAY_LENGTH(arr)   (arr)->len

#define ARRAY_ITEMS(arr)    (arr)->items

#define ARRAY_IS_FULL(arr)  ((arr)->len >= (arr)->cap)

#define ARRAY_IS_EMPTY(arr) ((arr)->len == 0)

#define ARRAY_PRINT(arr, fmt)                                                                                          \
    assert(arr != NULL);                                                                                               \
    for (size_t i = 0; i < (arr)->len; ++i) {                                                                          \
        printf("%zu) " fmt "\n", i, (arr)->items[i]);                                                                  \
    }

#define ARRAY_BUBBLE_SORT(__type, arr, cmp)                                                                            \
    assert(arr != NULL);                                                                                               \
    assert(cmp != NULL);                                                                                               \
    for (size_t i = 0; i < (arr)->len; ++i) {                                                                          \
        for (size_t j = 0; j < (arr)->len - 1 - i; ++j) {                                                              \
            if (!cmp((arr)->items[i], (arr)->items[j + 1])) {                                                          \
                __type tmp          = (arr)->items[j];                                                                 \
                (arr)->items[j]     = (arr)->items[j + 1];                                                             \
                (arr)->items[j + 1] = tmp;                                                                             \
            }                                                                                                          \
        }                                                                                                              \
    }
