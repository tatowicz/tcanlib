#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "server.h"
#include "ctp.h"  




// We need these driver functions to compile the server.c file
// since it depends on ctp, however in our tests we don't actually
// use the ctp functions, so we can just stub them out
bool send_ctp_message(uint32_t id, uint8_t *data, uint8_t length) {
    (void)id;
    (void)data;
    (void)length;
    return true;
}

bool receive_ctp_message(uint32_t *id, uint8_t *data, uint8_t *length) {
    (void)id;
    (void)data;
    (void)length;
    return true;
}

void test_set_query_processing() {
    // Test 1: Set a value for a key
    assert(strcmp(process_query("SET key1 value1"), "SET successful\n") == 0);
    assert(strcmp(get_value("key1"), "value1") == 0);

    // Test 2: Try to set a value with an invalid query
    assert(strcmp(process_query("SET key2"), "Invalid SET format\n") == 0);
}

void test_get_query_processing() {
    // Setup: Add a key-value pair to the database
    process_query("SET key3 value3");

    // Test 1: Retrieve a value for an existing key
    assert(strcmp(process_query("GET key3"), "value3") == 0);

    // Test 2: Try to retrieve a value for a non-existing key
    assert(strcmp(process_query("GET key_non_exist"), "Key not found\n") == 0);

    // Test 3: Try to retrieve a value with an invalid query
    assert(strcmp(process_query("GET"), "Invalid query format\n") == 0);
}

int main() {
    test_set_query_processing();
    test_get_query_processing();
    printf("All tests passed!\n");
    return 0;
}
