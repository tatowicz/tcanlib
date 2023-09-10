#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "kvs.h"

#define LOAD_FACTOR_THRESHOLD 0.7

bool should_resize(HashTable *table) {
    return (double)table->count / table->size > LOAD_FACTOR_THRESHOLD;
}

unsigned int hash(const char *key, int table_size) {
    unsigned int value = 0;
    for (const char *p = key; *p != '\0'; p++) {
        value = (value << 4) + *p;
        unsigned int high = value & 0xF0000000;
        if (high != 0) {
            value ^= high >> 24;
        }
        value &= ~high;
    }
    return value % table_size;
}

HashTable* create_table(int size) {
    HashTable *table = malloc(sizeof(HashTable));
    if (!table) {
        return NULL;
    }
    table->size = size;
    table->count = 0;
    table->buckets = calloc(size, sizeof(KeyValue*));
    if (!table->buckets) {
        free(table);
        return NULL;
    }
    return table;
}

void free_table(HashTable* table) {
    // Iterate over the hash table entries
    for (int i = 0; i < table->size; i++) {
        KeyValue* current_node = table->buckets[i];
        while (current_node) {
            KeyValue* temp = current_node;
            current_node = current_node->next;
            
            // Free the key and value strings
            free(temp->key);
            free(temp->value);
            
            // Free the node itself
            free(temp);
        }
    }
    
    // Free the hash table list (array of pointers)
    free(table->buckets);
    
    // Free the hash table structure
    free(table);
}


void resize_table(HashTable *table) {
    int old_size = table->size;
    table->size *= 2;  // Double the size
    KeyValue **new_buckets = calloc(table->size, sizeof(KeyValue*));

    if (!new_buckets) {
        // Handle memory allocation failure
        printf("[DEBUG] Failed to allocate memory for new buckets\n");
        return;
    }

    // Swap the old bucket list with the new one, then re-hash all entries
    KeyValue **old_buckets = table->buckets;
    table->buckets = new_buckets;
    table->count = 0;

    for (int i = 0; i < old_size; i++) {
        KeyValue *entry = old_buckets[i];
        while (entry) {
            insert(table, entry->key, entry->value);
            KeyValue *temp = entry;
            entry = entry->next;
            free(temp->key);
            free(temp->value);
            free(temp);
        }
    }

    free(old_buckets);
}

void insert(HashTable *table, const char *key, const char *value) {
    unsigned int bucket_index = hash(key, table->size);
    KeyValue *new_entry = malloc(sizeof(KeyValue));
    new_entry->key = strdup(key);
    new_entry->value = strdup(value);
    new_entry->next = table->buckets[bucket_index];
    table->buckets[bucket_index] = new_entry;
    table->count++;

    if (should_resize(table)) {
        resize_table(table);
    }
}

char* lookup(HashTable *table, const char *key) {
    unsigned int bucket_index = hash(key, table->size);
    KeyValue *entry = table->buckets[bucket_index];
    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            return entry->value;
        }
        entry = entry->next;
    }
    return NULL;  // Key not found
}

void delete(HashTable *table, const char *key) {
    unsigned int bucket_index = hash(key, table->size);
    KeyValue *entry = table->buckets[bucket_index];
    KeyValue *prev = NULL;
    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            if (prev == NULL) {
                table->buckets[bucket_index] = entry->next;
            } else {
                prev->next = entry->next;
            }
            free(entry->key);
            free(entry->value);
            free(entry);
            table->count--;
            return;
        }
        prev = entry;
        entry = entry->next;
    }
}

bool exists(HashTable *table, const char *key) {
    char *value = lookup(table, key);
    return value != NULL;
}

bool save_to_file(HashTable *table, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        return false; // Failed to open the file for writing
    }

    for (int i = 0; i < table->size; i++) {
        KeyValue *entry = table->buckets[i];
        while (entry) {
            fprintf(file, "%s,%s\n", entry->key, entry->value);
            entry = entry->next;
        }
    }

    fclose(file);
    return true;
}


bool load_from_file(HashTable *table, const char *filename) {
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    FILE *file = fopen(filename, "r");
    if (!file) {
        return false; // Failed to open the file for reading
    }

    while ((read = getline(&line, &len, file)) != -1) {
        char *comma_pos = strchr(line, ',');
        if (!comma_pos) {
            // Invalid line format, skipping
            continue;
        }
        *comma_pos = '\0'; // Split the line into key and value
        char *key = line;
        char *value = comma_pos + 1;
        // Remove newline character from value, if present
        char *newline_pos = strchr(value, '\n');
        if (newline_pos) {
            *newline_pos = '\0';
        }
        insert(table, key, value);
    }

    free(line);
    fclose(file);
    return true;
}
