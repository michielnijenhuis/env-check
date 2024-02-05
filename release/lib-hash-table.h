#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <lib-types.h>

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef HASH_TABLE_DEF
# define HASH_TABLE_DEF
#endif // HASH_TABLE_DEF

#define HASH_TABLE_DEFAULT_CONFIG hashTableCreateDefaultConfig()

typedef struct HashTableBucket {
    cstring                 key;
    void                   *value;
    struct HashTableBucket *next;
} HashTableBucket;

typedef u64(HashFunc)(cstr);

typedef void(CleanupFunc)(void *);

typedef void(HashTablePrintFunc)(u32, HashTableBucket *);

typedef struct HashTableConfig {
    HashFunc    *hash;
    CleanupFunc *keyCleanup;
    CleanupFunc *valueCleanup;
    boolean      cleanupBucket;
    boolean      cleanupBuckets;
    boolean      cleanupSelf;
} HashTableConfig;

typedef struct HashTable {
    HashTableBucket **entries;
    u32               capacity;
    u32               size;
    HashTableConfig  *config;
} HashTable;

HashTable        hashTableCreate(u32 capacity, HashTableBucket **buckets, HashTableConfig *config);
void             hashTableInit(HashTable *table, u32 capacity, HashTableBucket **buckets, HashTableConfig *config);
HashTable       *hashTableCreateMalloc(u32 capacity, HashTableBucket **buckets, HashTableConfig *config);
HashTableConfig  hashTableCreateConfig(boolean      cleanupBucket,
                                       boolean      cleanupBuckets,
                                       boolean      cleanupSelf,
                                       CleanupFunc *keyCleanup,
                                       CleanupFunc *valueCleanup,
                                       HashFunc    *hashfn);
HashTableConfig  hashTableCreateDefaultConfig(void);
HashTableBucket  hashTableCreateBucket(cstring key, void *value);
HashTableBucket *hashTableCreateBucketHeap(cstring key, void *value);
void             hashTableInitBuckets(HashTableBucket **buckets, u32 capacity);
boolean          hashTablePut(HashTable *self, cstring key, void *value);
boolean          hashTableInsert(HashTable *self, cstring key, void *value, HashTableBucket *buckey);
void            *hashTableGet(HashTable *self, cstring key);
boolean          hashTableHas(HashTable *self, cstring key);
void            *hashTableDelete(HashTable *self, cstring key);
void             hashTableDestroy(HashTable *self);
u32              hashTableSize(HashTable *self);
void             hashTablePrint(HashTable *self, HashTablePrintFunc *printfn);
cstring         *hashTableKeys(HashTable *self);
int              hashTableKeysBuffer(HashTable *self, cstring *buffer, usize bufferSize);
void           **hashTableValues(HashTable *self, usize valueSize);
int              hashTableValuesBuffer(HashTable *self, void **buffer, usize bufferSize);
u64              hash(cstring key);

#endif // HASH_TABLE_H
