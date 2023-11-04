#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ctp.h"

#define CTP_ID 0x123

typedef struct {
    char key[50];
    char value[50];
    size_t size;
} KeyValuePair;

#define MAX_DATABASE_SIZE 100

KeyValuePair database[MAX_DATABASE_SIZE];
uint32_t database_size = 0;

const char* get_value(const char* key) {
    for (uint32_t i = 0; i < database_size; i++) {
        if (strcmp(database[i].key, key) == 0) {
            return database[i].value;
        }
    }
    return NULL;  // Key not found
}

const char* process_query(const char *query) {
    char command[10], key[50], value[50];
    
    if (sscanf(query, "%9s %49s %49s", command, key, value) < 2) {
        return  "Invalid query format\n";
    }

    if (strcmp(command, "GET") == 0) {
        const char* val = get_value(key);
        if (val != NULL) {
            return val;
        } else {
            return "Key not found\n";
        }
    } else if (strcmp(command, "SET") == 0) {
        if (sscanf(query, "%9s %49s %49s", command, key, value) != 3) {
            return "Invalid SET format\n";
        }

        if (database_size < MAX_DATABASE_SIZE) {
            strcpy(database[database_size].key, key);
            strcpy(database[database_size].value, value);
            database[database_size].size = strlen(value);
            database_size++;
            return "SET successful\n";
        } else {
            return "Database full\n";
        }
    } else {
        return "Invalid query format\n";
    }
}

// Function to process an upload query
const char* process_upload(const char *key, const char *byte_blob, size_t size) {

    
    // Check if database is full
    if (database_size >= MAX_DATABASE_SIZE) {
        printf("[DEBUG] %s", "Error: Database full, cannot upload more data.\\n");
        return "Error: Database full, cannot upload more data.\\n";
    }

    // Store the key and byte blob
    strncpy(database[database_size].key, key, 50);

    database[database_size].size = size;
    printf("[DEBUG] Uploaded %s: %.*s\\n", key, (int)size, byte_blob);

    // Increment the database size
    database_size++;

    return "Upload successful.\\n";
}

// Function to process a download query
const char* process_download(const char *key) {
    
    // Search for the key in the database
    for (uint32_t i = 0; i < database_size; i++) {
        if (strncmp(database[i].key, key, 50) == 0) {
            // Found the key, send the byte blob
            char msg_data_for_key[] = "Data for key ";
            printf("[DEBUG] %s%s: %.*s\\n", msg_data_for_key, key, (int)database[i].size, database[i].value);
            return "Download successful.\\n";
        }
    }

    // Key not found
    printf("[DEBUG] %s", "Error: Key not found.\\n");
    return "Error: Key not found.\\n";
}

int server_listen(void) {
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
        if (strcmp(command, "AUTH") == 0) {} 
        else if (strcmp(command, "QUERY") == 0) {
            char res[32];
            strcpy(res, process_query(args));
            ctp_send(CTP_ID, (uint8_t*)res, strlen(res), false);
        } 
        else if (strcmp(command, "DOWNLOAD") == 0) {
            char res[64];
            strcpy(res, process_download(args));
            ctp_send(CTP_ID, (uint8_t*)res, strlen(res), false);
        } 
        else if (strcmp(command, "UPLOAD") == 0) {
            char res[64];
            strncpy(res, process_upload(args, args, strlen(args)), sizeof(res));
            ctp_send(CTP_ID, (uint8_t*)res, strlen(res), false);
        } 
        else {
            printf("Unknown command: %s\n", command);
        }
    }

    return 0;
}
