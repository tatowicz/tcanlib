#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "server.h"
#include "ctp.h"  



// Create mock structure for testing, this is a copy
// from the CTP tests, will eventually make this reusable
// TODO: Make this reusable
typedef struct {
    uint32_t id;
    uint8_t data[CAN_MAX_DATA_LENGTH];
    uint8_t length;
} MockFrame;

// Mock frame queue
MockFrame mock_frames[150];
int mock_frame_count = 0;
int mock_frame_index = 0;

// Function to enqueue a mock frame
void enqueue_mock_frame(uint32_t id, uint8_t *data, uint8_t length) {

    if (mock_frame_count < 150) {
        mock_frames[mock_frame_count].id = id;
        memcpy(mock_frames[mock_frame_count].data, data, length);
        mock_frames[mock_frame_count].length = length;
        mock_frame_count++;
    }
}

// Mock driver function to send a CAN message to the bus
bool send_ctp_message(uint32_t id, uint8_t *data, uint8_t length) {
    printf("[DEBUG] Sending CAN message with ID: %u, Data: ", id);
    for (int i = 0; i < length; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
    
    enqueue_mock_frame(id, data, length);
    return true; // Simulate successful send
}

// Modified Mock driver function
bool receive_ctp_message(uint32_t *id, uint8_t *data, uint8_t *length) {    
    // If we have no more frames to dequeue, return false
    if (mock_frame_index >= mock_frame_count) {
        return false;
    }
    
    // Dequeue the next frame
    *id = mock_frames[mock_frame_index].id;
    memcpy(data, mock_frames[mock_frame_index].data, mock_frames[mock_frame_index].length);
    *length = mock_frames[mock_frame_index].length;
    mock_frame_index++;
    
    printf("[DEBUG] Receive mock frame with ID: %u, Data: ", *id);
    for (int i = 0; i < *length; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
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
