#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "kvs.h"

void test_insert_and_lookup() {
    HashTable *table = create_table(10);

    // Test insertion
    insert(table, "key1", "value1");
    insert(table, "key2", "value2");
    insert(table, "key3", "value3");

    // Test lookup
    char *val1 = lookup(table, "key1");
    char *val2 = lookup(table, "key2");
    char *val3 = lookup(table, "key3");
    assert(val1 && strcmp(val1, "value1") == 0);
    assert(val2 && strcmp(val2, "value2") == 0);
    assert(val3 && strcmp(val3, "value3") == 0);

    free_table(table);
    printf("test_insert_and_lookup PASSED\n");
}

void test_resize() {
    HashTable *table = create_table(2); // Small size to trigger resize

    // Insert more than 2 elements to trigger resizing
    insert(table, "key1", "value1");
    insert(table, "key2", "value2");
    insert(table, "key3", "value3");

    // Check if values still exist after resize
    char *val1 = lookup(table, "key1");
    char *val2 = lookup(table, "key2");
    char *val3 = lookup(table, "key3");
    assert(val1 && strcmp(val1, "value1") == 0);
    assert(val2 && strcmp(val2, "value2") == 0);
    assert(val3 && strcmp(val3, "value3") == 0);

    free_table(table);
    printf("test_resize PASSED\n");
}

void test_save_and_load() {
    HashTable *table = create_table(10);

    // Insert some key-value pairs
    insert(table, "key1", "value1");
    insert(table, "key2", "value2");
    insert(table, "key3", "value3");

    // Save to file
    assert(save_to_file(table, "test_save.txt"));

    // Load into a new table
    HashTable *loaded_table = create_table(10);
    assert(load_from_file(loaded_table, "test_save.txt"));

    // Check if the loaded table has the same values
    char *val1 = lookup(loaded_table, "key1");
    char *val2 = lookup(loaded_table, "key2");
    char *val3 = lookup(loaded_table, "key3");
    assert(val1 && strcmp(val1, "value1") == 0);
    assert(val2 && strcmp(val2, "value2") == 0);
    assert(val3 && strcmp(val3, "value3") == 0);

    free_table(table);
    free_table(loaded_table);
    printf("test_save_and_load PASSED\n");
}

void test_basic_functionality() {
    HashTable* table = create_table(5);

    // Test set and get
    insert(table, "key1", "value1");
    char* value = lookup(table, "key1");
    assert(value && strcmp(value, "value1") == 0);

    // Test update and get
    insert(table, "key1", "value_updated");
    value = lookup(table, "key1");
    assert(value && strcmp(value, "value_updated") == 0);

    // Test delete
    delete(table, "key1");
    value = lookup(table, "key1");
    assert(value);

    free_table(table);
    printf("test_basic_functionality PASSED\n");
}


int main() {
    test_insert_and_lookup();
    test_resize();
    test_save_and_load();
    test_basic_functionality();

    printf("All tests PASSED\n");
    return 0;
}
