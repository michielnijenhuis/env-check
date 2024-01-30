#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "types.h"

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

// internal functions
u64     hashTableIndex(HashTable *self, cstring key);
boolean handleHashTableInsertAtIndex(HashTable *self, u32 index, HashTableBucket *entry, boolean atHead);

#endif // HASH_TABLE_H

#ifdef HASH_TABLE_IMPLEMENTATION
#ifndef HASH_TABLE_INCLUDED
# define HASH_TABLE_INCLUDED

HashTable hashTableCreate(u32 capacity, HashTableBucket **buckets, HashTableConfig *config) {
    HashTable self;
    hashTableInit(&self, capacity, buckets, config);
    return self;
}

void hashTableInit(HashTable *table, u32 capacity, HashTableBucket **buckets, HashTableConfig *config) {
    assert(table != NULL);
    table->capacity = capacity;
    table->entries  = buckets;
    table->size     = 0;
    table->config = config;
}

HashTable *hashTableCreateMalloc(u32 capacity, HashTableBucket **buckets, HashTableConfig *config) {
    HashTable *self = malloc(sizeof(*self));
    hashTableInit(self, capacity, buckets, config);
    return self;
}

HashTableConfig hashTableCreateConfig(boolean      cleanupBucket,
                                      boolean      cleanupBuckets,
                                      boolean      cleanupSelf,
                                      CleanupFunc *keyCleanup,
                                      CleanupFunc *valueCleanup,
                                      HashFunc    *hashfn) {
    HashTableConfig config;
    config.cleanupBucket  = cleanupBucket;
    config.cleanupBuckets = cleanupBuckets;
    config.cleanupSelf    = cleanupSelf;
    config.keyCleanup     = keyCleanup;
    config.valueCleanup   = valueCleanup;
    config.hash           = hashfn;
    return config;
}

HashTableConfig hashTableCreateDefaultConfig(void) {
    return hashTableCreateConfig(false, false, false, NULL, NULL, NULL);
}

void hashTableInitBuckets(HashTableBucket **buckets, u32 capacity) {
    for (u32 i = 0; i < capacity; ++i) {
        buckets[i] = NULL;
    }
}

boolean hashTableInsert(HashTable *self, cstring key, void *value, HashTableBucket *buffer) {
    if (self == NULL || key == NULL || buffer == NULL) {
        return false;
    }

    buffer->key              = key;
    buffer->value            = value;
    buffer->next             = NULL;

    u64              index   = hashTableIndex(self, key);
    HashTableBucket *current = self->entries[index];

    if (current == NULL) {
        return handleHashTableInsertAtIndex(self, index, buffer, false);
    }

    while (current != NULL) {
        // Case: keys match, entry already exists -> update
        if (strcmp(current->key, key) == 0) {
            if (self->config != NULL && self->config->valueCleanup != NULL) {
                self->config->valueCleanup(current->value);
            }
            current->value = buffer->value;
            return true;
        }

        current = current->next;
    }

    return handleHashTableInsertAtIndex(self, index, buffer, true);
}

boolean hashTablePut(HashTable *self, cstring key, void *value) {
    if (self == NULL || key == NULL) {
        return false;
    }

    u64              index = hashTableIndex(self, key);
    HashTableBucket *entry = self->entries[index];

    // case: hash table does not have key yet
    if (entry == NULL) {
        entry = hashTableCreateBucketHeap(key, value);
        return handleHashTableInsertAtIndex(self, index, entry, false);
    }

    // case: collisions
    while (entry != NULL) {
        // Case: keys match, entry already exists -> update
        if (strcmp(entry->key, key) == 0) {
            entry->value = value;
            return true;
        }

        entry = entry->next;
    }

    // case: collision, but no matching keys -> insert new entry at head
    HashTableBucket *newEntry = hashTableCreateBucketHeap(key, value);
    return handleHashTableInsertAtIndex(self, index, newEntry, true);
}

void *hashTableGet(HashTable *self, cstring key) {
    if (self == NULL || key == NULL) {
        return NULL;
    }

    u64              index = hashTableIndex(self, key);
    HashTableBucket *entry = self->entries[index];

    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            return entry->value;
        }

        entry = entry->next;
    }

    return NULL;
}

boolean hashTableHas(HashTable *self, cstring key) {
    return hashTableGet(self, key) != NULL;
}

void *hashTableDelete(HashTable *self, cstring key) {
    if (self == NULL || key == NULL) {
        return NULL;
    }

    u64              index = hashTableIndex(self, key);
    HashTableBucket *entry = self->entries[index];
    HashTableBucket *prev  = NULL;

    while (entry != NULL && strcmp(entry->key, key) != 0) {
        prev  = entry;
        entry = entry->next;
    }

    if (entry == NULL) {
        return NULL;
    }

    if (prev == NULL) {
        self->entries[index] = entry->next;
    } else {
        prev->next = entry->next;
    }

    void *value = entry->value;

    if (self->config != NULL) {
        if (self->config->keyCleanup != NULL) {
            self->config->keyCleanup((string) entry->key);
        }

        if (self->config->cleanupBucket) {
            free(entry);
        }
    }

    self->size--;

    return value;
}

void hashTableDestroy(HashTable *self) {
    if (self == NULL) {
        return;
    }

    for (u32 i = 0; i < self->capacity; ++i) {
        while (self->entries[i] != NULL) {
            HashTableBucket *entry = self->entries[i];
            self->entries[i]       = self->entries[i]->next;

            if (self->config != NULL) {
                if (self->config->keyCleanup != NULL) {
                    self->config->keyCleanup((string) entry->key);
                }

                if (self->config->valueCleanup != NULL) {
                    self->config->valueCleanup(entry->value);
                }

                if (self->config->cleanupBucket) {
                    free(entry);
                }
            }
        }
    }

    self->capacity = 0;
    self->size     = 0;

    if (self->config != NULL) {
        if (self->config->cleanupBuckets) {
            free(self->entries);
        }

        if (self->config->cleanupSelf) {
            free(self);
        }
    }
}

u32 hashTableSize(HashTable *self) {
    if (self == NULL) {
        return 0;
    }

    return self->size;
}

void hashTablePrint(HashTable *self, HashTablePrintFunc *printfn) {
    if (self == NULL) {
        fprintf(stderr, "Trying to print hash table, but received NULL pointer\n");
        return;
    }

    printf("Printing hash table (size: %u) (capacity: %i)\n", self->size, self->capacity);
    for (u32 i = 0; i < self->capacity; ++i) {
        HashTableBucket *bucket = self->entries[i];

        if (printfn != NULL) {
            printfn(i, bucket);
            continue;
        }

        if (bucket == NULL) {
            printf("  %i)\t---\n", i);
        } else {
            printf("  %i)\t", i);
            u32 j = 0;
            while (bucket != NULL) {
                printf("%s\"%s\" (%p)", j == 0 ? "" : " - ", bucket->key, bucket->value);
                bucket = bucket->next;
                j++;
            }
            printf("\n");
        }
    }
    printf("\n");
}

cstring *hashTableKeys(HashTable *self) {
    if (self == NULL) {
        return NULL;
    }

    cstring *keys = (cstring *) malloc(sizeof(cstr) * (self->size + 1));

    if (keys == NULL) {
        return NULL;
    }

    hashTableKeysBuffer(self, keys, self->size + 1);
    return keys;
}

int hashTableKeysBuffer(HashTable *self, cstring *buffer, usize bufferSize) {
    if (bufferSize < self->size) {
        return -1;
    }

    u32 ptr = 0;

    for (u32 i = 0; i < self->capacity; i++) {
        HashTableBucket *entry = self->entries[i];

        while (entry != NULL) {
            buffer[ptr++] = entry->key;
            entry         = entry->next;
        }
    }

    if (ptr < bufferSize) {
        buffer[ptr] = NULL;
    }

    return 0;
}

void **hashTableValues(HashTable *self, usize valueSize) {
    if (self == NULL) {
        return NULL;
    }

    void **values = (void **) malloc(valueSize * (self->size + 1));

    if (values == NULL) {
        return NULL;
    }

    hashTableValuesBuffer(self, values, self->size + 1);
    return values;
}

int hashTableValuesBuffer(HashTable *self, void **buffer, usize bufferSize) {
    if (bufferSize < self->size) {
        return -1;
    }

    u32 ptr = 0;

    for (u32 i = 0; i < self->capacity; i++) {
        HashTableBucket *entry = self->entries[i];

        while (entry != NULL) {
            buffer[ptr++] = entry->value;
            entry         = entry->next;
        }
    }

    if (ptr < bufferSize) {
        buffer[ptr] = NULL;
    }

    return 0;
}

u64 hashTableIndex(HashTable *self, cstring key) {
    if (key == NULL) {
        return 0;
    }

    HashFunc *hashfn = self->config && self->config->hash ? self->config->hash : hash;
    u64       index  = hashfn(key);
    return index % self->capacity;
}

u64 hash(cstring key) {
    if (key == NULL) {
        return 0;
    }

    u64 hash = 5381;
    int c    = *key++;

    while (c != '\0') {
        if (isupper(c)) {
            c = c + 32;
        }
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
        c    = *key++;
    }

    return hash;
}

HashTableBucket hashTableCreateBucket(cstring key, void *value) {
    HashTableBucket bucket;
    bucket.key   = key;
    bucket.value = value;
    return bucket;
}

HashTableBucket *hashTableCreateBucketHeap(cstring key, void *value) {
    if (key == NULL) {
        return NULL;
    }

    HashTableBucket *entry = (HashTableBucket *) malloc(sizeof(*entry));

    if (entry == NULL) {
        return NULL;
    }

    entry->key   = strdup(key);
    entry->value = value;
    entry->next  = NULL;

    return entry;
}

boolean handleHashTableInsertAtIndex(HashTable *self, u32 index, HashTableBucket *entry, boolean atHead) {
    if (entry == NULL) {
        return false;
    }

    if (atHead) {
        entry->next = self->entries[index];
    }

    self->entries[index] = entry;
    self->size++;
    return true;
}

#endif // HASH_TABLE_INCLUDED
#endif // HASH_TABLE_IMPLEMENTATION
