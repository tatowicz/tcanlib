#ifndef KVS_H
#define KVS_H

#include <stdbool.h>

typedef struct KeyValue {
    char *key;
    char *value;
    struct KeyValue *next;  // Next entry in the chain (for handling collisions)
} KeyValue;

typedef struct HashTable {
    KeyValue **buckets;  // Array of pointers to KeyValue pairs
    int size;            // Size of the hash table (number of buckets)
    int count;           // Number of entries in the table
} HashTable;


HashTable* create_table(int size);
void free_table(HashTable* table);
void insert(HashTable *table, const char *key, const char *value);
char* lookup(HashTable *table, const char *key);
void delete(HashTable *table, const char *key);

bool save_to_file(HashTable *table, const char *filename);
bool load_from_file(HashTable *table, const char *filename);

#endif