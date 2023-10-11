#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ctp.h"

#define CTP_ID 0x123

// Define a simple key-value pair structure to hold database entries.
typedef struct {
    char key[50];
    char value[50];
    size_t size;
} KeyValuePair;

#define MAX_DATABASE_SIZE 100

// Database to store key-value pairs and a counter for the current database size.
KeyValuePair database[MAX_DATABASE_SIZE];
uint32_t database_size = 0;

/**
 * Process a given query and perform database operations.
 * 
 * The function supports the following query formats:
 * - GET [key]: Retrieve the value for the provided key.
 * - SET [key] [value]: Set the value for the provided key.
 * 
 * @param query A string containing the query to be processed.
 */
void process_query(const char *query) {
    char command[10], key[50], value[50];
    
    char err_invalid_query[] = "Invalid query format\\n";
    char err_key_not_found[] = "Key not found\\n";
    char err_invalid_set_format[] = "Invalid SET format\\n";
    char err_database_full[] = "Database full\\n";
    char err_unknown_command[] = "Unknown command\\n";
    
    // Parse the query
    if (sscanf(query, "%s %s %s", command, key, value) < 2) {
        printf("[DEBUG] %s", err_invalid_query);
        ctp_send(CTP_ID, (uint8_t*)err_invalid_query, strlen(err_invalid_query), false);
        return;
    }
    
    // Handle GET command
    if (strcmp(command, "GET") == 0) {
        for (int i = 0; i < MAX_DATABASE_SIZE; ++i) {
            if (strcmp(database[i].key, key) == 0) {
                printf("[DEBUG] Value: %s\\n", database[i].value);
                ctp_send(CTP_ID, (uint8_t*)database[i].value, strlen(database[i].value), false);
                return;
            }
        }
        printf("[DEBUG] %s", err_key_not_found);
        ctp_send(CTP_ID, (uint8_t*)err_key_not_found, strlen(err_key_not_found), false);
        return;
    }
    
    // Handle SET command
    else if (strcmp(command, "SET") == 0) {
        if (sscanf(query, "%s %s %s", command, key, value) != 3) {
            printf("[DEBUG] %s", err_invalid_set_format);
            ctp_send(CTP_ID, (uint8_t*)err_invalid_set_format, strlen(err_invalid_set_format), false);
            return;
        }

        // Check for existing key or empty slot
        for (int i = 0; i < MAX_DATABASE_SIZE; ++i) {
            if (strcmp(database[i].key, key) == 0 || database[i].key[0] == '\0') {
                strncpy(database[i].key, key, sizeof(database[i].key));
                strncpy(database[i].value, value, sizeof(database[i].value));
                database[i].size = strlen(value);  // Update the size variable
                char msg_set_successful[] = "[DEBUG] Set successful\\n";
                printf("[DEBUG] %s", msg_set_successful);
                ctp_send(CTP_ID, (uint8_t*)msg_set_successful, strlen(msg_set_successful), false);
                return;
            }
        }
        printf("[DEBUG] %s", err_database_full);
        ctp_send(CTP_ID, (uint8_t*)err_database_full, strlen(err_database_full), false);
        return;
    }
    
    else {
        printf("[DEBUG] %s", err_unknown_command);
        ctp_send(CTP_ID, (uint8_t*)err_unknown_command, strlen(err_unknown_command), false);
    }
}

// Function to process an upload query
void process_upload(const char *key, const char *byte_blob, size_t size) {
    char err_database_full[] = "Error: Database full, cannot upload more data.\\n";
    char upload_successful[] = "Upload successful.\\n";
    
    // Check if database is full
    if (database_size >= MAX_DATABASE_SIZE) {
        printf("[DEBUG] %s", err_database_full);
        return;
    }

    // Store the key and byte blob
    strncpy(database[database_size].key, key, 50);

    database[database_size].size = size;
    printf("[DEBUG] Uploaded %s: %.*s\\n", key, (int)size, byte_blob);
    ctp_send(CTP_ID, (uint8_t*)upload_successful, strlen(upload_successful), false);

    // Increment the database size
    database_size++;
}

// Function to process a download query
void process_download(const char *key) {
    char err_key_not_found[] = "Error: Key not found.\\n";
    
    // Search for the key in the database
    for (uint32_t i = 0; i < database_size; i++) {
        if (strncmp(database[i].key, key, 50) == 0) {
            // Found the key, send the byte blob
            char msg_data_for_key[] = "Data for key ";
            printf("[DEBUG] %s%s: %.*s\\n", msg_data_for_key, key, (int)database[i].size, database[i].value);
            ctp_send(CTP_ID, (uint8_t*)database[i].value, database[i].size, false);
            return;
        }
    }

    // Key not found
    printf("[DEBUG] %s", err_key_not_found);
    ctp_send(CTP_ID, (uint8_t*)err_key_not_found, sizeof(err_key_not_found)-1, false);
}


// Move to test file and make some tests

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
    printf("Sending CAN message with ID: %u, Data: ", id);
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
    
    printf("[DEBUG] Dequeued mock frame with ID: %u, Data: ", *id);
    for (int i = 0; i < *length; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
    return true;
}

int main(void) {
    char buffer[1024];
    ssize_t received_length;

    printf("Server is running...\n");

    while (1) {
        // Receive message
        received_length = ctp_receive((uint8_t*)buffer, sizeof(buffer) - 1, false);
        if (received_length < 0) {
            perror("receive");
            continue;
        }

        // Null-terminate the received data
        buffer[received_length] = '\0';

        // Parse command and arguments
        char command[50], args[1024];
        sscanf(buffer, "%[^;];%s", command, args);

        // Dispatch to appropriate handler based on command
        if (strcmp(command, "AUTH") == 0) {
        } 
        else if (strcmp(command, "QUERY") == 0) {
            process_query(args);
        } 
        else if (strcmp(command, "DOWNLOAD") == 0) {
            process_download(args);
        } 
        else if (strcmp(command, "UPLOAD") == 0) {
        } 
        else {
            printf("Unknown command: %s\n", command);
        }
    }

    return 0;
}
