#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef u_int64_t(hash_func)(const char *);

#define HT_BUCKET_TYPE(__name) __name##_Bucket

#define DEFINE_HASH_MAP(__name, __type)                                                                                \
    typedef void(__name##_Cleanup_Func)(__type);                                                                       \
    typedef struct HT_BUCKET_TYPE(__name) {                                                                            \
        const char *key;                                                                                               \
        __type      value;                                                                                             \
        struct HT_BUCKET_TYPE(__name) * next;                                                                          \
    } HT_BUCKET_TYPE(__name);                                                                                          \
    typedef struct __name {                                                                                            \
        HT_BUCKET_TYPE(__name) * *buckets;                                                                             \
        size_t                 cap;                                                                                    \
        size_t                 size;                                                                                   \
        size_t                 collisions;                                                                             \
        hash_func             *hash_fn;                                                                                \
        __name##_Cleanup_Func *cleanup_fn;                                                                             \
        const char            *name;                                                                                   \
        const char            *type;                                                                                   \
    } __name

#ifndef HT_MALLOC
# define HT_MALLOC malloc
#endif // HT_MALLOC

#ifndef HT_CALLOC
# define HT_CALLOC calloc
#endif // HT_CALLOC

#ifndef HT_REALLOC
# define HT_REALLOC realloc
#endif // HT_REALLOC

#ifndef HT_FREE_FUNC
# define HT_FREE_FUNC free
#endif // HT_FREE_FUNC

#define HT_CALL_FREE_FUNC(p)                                                                                           \
    if (HT_FREE_FUNC != NULL) {                                                                                        \
        HT_FREE_FUNC(p);                                                                                               \
    }

#ifndef HT_DEFAULT_HASH_FUNC
# define HT_DEFAULT_HASH_FUNC hash_djb2
#endif // HT_DEFAULT_HASH_FUNC

#define HT_CREATE(__name, __type, capacity, hashfn, cleanupfn)                                                         \
    {                                                                                                                  \
        .buckets = HT_CALLOC(capacity, sizeof(__type)), .cap = capacity, .collisions = 0,                              \
        .hash_fn = hashfn ? hashfn : HT_DEFAULT_HASH_FUNC, .cleanup_fn = cleanupfn, .name = #__name, .type = #__type,  \
    }

#define HT_SET_KEY(bucket, key)                                                                                        \
    char *cpy = HT_MALLOC((strlen(key) + 1) * sizeof(char));                                                           \
    assert(cpy != NULL);                                                                                               \
    strcpy(cpy, key);                                                                                                  \
    (bucket)->key = cpy

#define HT_CREATE_BUCKET(__ht_name, key, value)                                                                        \
    assert(key != NULL);                                                                                               \
    HT_BUCKET_TYPE(__ht_name) *bucket = HT_MALLOC(sizeof(*bucket__##__name));                                          \
    assert(bucket != NULL);                                                                                            \
    HT_SET_KEY(bucket, key);                                                                                           \
    bucket->value = value;                                                                                             \
    bucket->next  = NULL;                                                                                              \
    return bucket

#define HT_INDEX(ht, key) ((ht)->hash_fn(key) % (ht)->cap)

#define HT_INSERT_AT_INDEX(ht, i, bucket, head)                                                                        \
    assert(ht != NULL);                                                                                                \
    assert(bucket != NULL);                                                                                            \
    assert(i < (ht)->cap);                                                                                             \
    if (head) {                                                                                                        \
        bucket->next = (ht)->buckets[i];                                                                               \
    }                                                                                                                  \
    (ht)->buckets[i] = bucket;                                                                                         \
    (ht)->size += 1

#define HT_PUT(__name, ht, key, value)                                                                                 \
    assert(ht != NULL);                                                                                                \
    assert(key != NULL);                                                                                               \
    u_int64_t i                    = HT_INDEX(ht, key);                                                                \
    HT_BUCKET_TYPE(__name) *bucket = (ht)->buckets[i];                                                                 \
    if (bucket == NULL) {                                                                                              \
        bucket = HT_MALLOC(sizeof(*bucket));                                                                           \
        assert(bucket != NULL);                                                                                        \
        HT_SET_KEY(bucket, key);                                                                                       \
        bucket->value = value;                                                                                         \
        bucket->next  = NULL;                                                                                          \
        HT_INSERT_AT_INDEX(ht, i, bucket, false);                                                                      \
        return true;                                                                                                   \
    }                                                                                                                  \
    (ht)->collisions++;                                                                                                \
    while (bucket != NULL) {                                                                                           \
        if (strcmp(bucket->key, key) == 0) {                                                                           \
            bucket->value = value;                                                                                     \
            return true;                                                                                               \
        }                                                                                                              \
        bucket = bucket->next;                                                                                         \
    }                                                                                                                  \
    HT_BUCKET_TYPE(__name) *new_bucket = HT_MALLOC(sizeof(*new_bucket));                                               \
    assert(new_bucket != NULL);                                                                                        \
    HT_SET_KEY(new_bucket, key);                                                                                       \
    new_bucket->value = value;                                                                                         \
    new_bucket->next  = NULL;                                                                                          \
    HT_INSERT_AT_INDEX(ht, i, new_bucket, true);                                                                       \
    return true

#define HT_GET(__name, ht, key, empty)                                                                                 \
    assert(ht != NULL);                                                                                                \
    assert(key != NULL);                                                                                               \
    u_int64_t i                    = HT_INDEX(ht, key);                                                                \
    HT_BUCKET_TYPE(__name) *bucket = (ht)->buckets[i];                                                                 \
    while (bucket != NULL) {                                                                                           \
        if (strcmp((bucket)->key, key) == 0) {                                                                         \
            return (bucket)->value;                                                                                    \
        }                                                                                                              \
        bucket = (bucket)->next;                                                                                       \
    }                                                                                                                  \
    return empty

#define HT_DELETE(__name, ht, key)                                                                                     \
    assert(ht != NULL);                                                                                                \
    assert(key != NULL);                                                                                               \
    u_int64_t i                    = HT_INDEX(ht, key);                                                                \
    HT_BUCKET_TYPE(__name) *bucket = (ht)->buckets[i];                                                                 \
    HT_BUCKET_TYPE(__name) *prev   = NULL;                                                                             \
    while (bucket != NULL && strcmp((bucket)->key, key) != 0) {                                                        \
        prev   = bucket;                                                                                               \
        bucket = (bucket)->next;                                                                                       \
    }                                                                                                                  \
    if (bucket == NULL) {                                                                                              \
        return false;                                                                                                  \
    }                                                                                                                  \
    if ((bucket)->next != NULL) {                                                                                      \
        (ht)->collisions--;                                                                                            \
    }                                                                                                                  \
    if (prev == NULL) {                                                                                                \
        (ht)->buckets[i] = (bucket)->next;                                                                             \
    } else {                                                                                                           \
        (prev)->next = (bucket)->next;                                                                                 \
    }                                                                                                                  \
    (ht)->size--;                                                                                                      \
    return true

#define HT_SIZE(ht) (ht)->size

#define HT_FREE(__name, ht)                                                                                            \
    if (!ht || !(ht)->buckets)                                                                                         \
        return;                                                                                                        \
    for (size_t i = 0; i < (ht)->cap; ++i) {                                                                           \
        while ((ht)->buckets[i] != NULL) {                                                                             \
            HT_BUCKET_TYPE(__name) *bucket = (ht)->buckets[i];                                                         \
            (ht)->buckets[i]               = (bucket)->next;                                                           \
            HT_CALL_FREE_FUNC((char *) bucket->key);                                                                   \
            if ((ht)->cleanup_fn) {                                                                                    \
                (ht)->cleanup_fn((void *) (bucket)->value);                                                            \
            }                                                                                                          \
            HT_CALL_FREE_FUNC(bucket);                                                                                 \
        }                                                                                                              \
    }                                                                                                                  \
    (ht)->cap        = 0;                                                                                              \
    (ht)->size       = 0;                                                                                              \
    (ht)->collisions = 0;                                                                                              \
    HT_CALL_FREE_FUNC((ht)->buckets);                                                                                  \
    (ht)->buckets    = NULL;                                                                                           \
    (ht)->cleanup_fn = NULL;                                                                                           \
    (ht)->hash_fn    = NULL;

#define HT_KEYS(__name, ht, buffer, buffersize)                                                                        \
    assert(ht != NULL);                                                                                                \
    assert(buffer != NULL);                                                                                            \
    assert(buffersize >= (ht)->size);                                                                                  \
    size_t ptr = 0;                                                                                                    \
    for (size_t i = 0; i < (ht)->cap && ptr < buffersize; ++i) {                                                                           \
        HT_BUCKET_TYPE(__name) *bucket = (ht)->buckets[i];                                                             \
        while (bucket != NULL) {                                                                                       \
            buffer[ptr++] = bucket->key;                                                                     \
            bucket        = bucket->next;                                                                    \
        }                                                                                                              \
    }

#define HT_VALUES(__name, ht, buffer, buffersize)                                                                      \
    assert(ht != NULL);                                                                                                \
    assert(buffer != NULL);                                                                                            \
    assert(buffersize >= (ht)->size);                                                                                  \
    size_t cursor = 0;                                                                                                 \
    for (size_t i = 0; i < (ht)->cap; ++i) {                                                                           \
        HT_BUCKET_TYPE(__name) *bucket = (ht)->buckets[i];                                                             \
        while (bucket != NULL) {                                                                                       \
            buffer[cursor++] = (ht)->buckets[i]->value;                                                                \
            bucket           = (ht)->buckets[i]->next;                                                                 \
        }                                                                                                              \
    }

static u_int64_t hash_djb2(const char *key) {
    assert(key != NULL);
    u_int64_t hash = 5381;
    int       c    = *key++;

    while (c != '\0') {
        hash = ((hash << 5) + hash) + c;
        c    = *key++;
    }

    return hash;
}
