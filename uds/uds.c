#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "ctp.h"
#include "uds.h"


uint32_t current_security_level = SECURITY_LEVEL_LOCKED;

uint8_t response_buffer[1024];

// error codes database
ErrorCodeInfo error_codes_db[MAX_ERROR_CODES];
uint32_t num_error_codes = 0;

// Instantiate the global database
// TODO: Revise this to use a kvs
SystemDatabase global_database;
uint8_t current_session = DEFAULT_SESSION;


// Functions to interact with the database
bool get_data_by_identifier(uint16_t identifier, uint8_t* data, uint8_t* data_length) {
    for (int i = 0; i < MAX_DATA_IDENTIFIERS; i++) {
        if (global_database.data_by_identifier[i].valid && 
            global_database.data_by_identifier[i].identifier == identifier) {
            memcpy(data, global_database.data_by_identifier[i].data, 
                   global_database.data_by_identifier[i].data_length);
            *data_length = global_database.data_by_identifier[i].data_length;
            return true;
        }
    }
    return false; // Identifier not found
}

bool set_data_by_identifier(uint16_t identifier, const uint8_t* data, uint8_t data_length) {
    for (int i = 0; i < MAX_DATA_IDENTIFIERS; i++) {
        if (global_database.data_by_identifier[i].valid && 
            global_database.data_by_identifier[i].identifier == identifier) {
            // Update existing entry
            memcpy(global_database.data_by_identifier[i].data, data, data_length);
            global_database.data_by_identifier[i].data_length = data_length;
            return true;
        }
    }
    // If not found, find an empty slot and add it
    for (int i = 0; i < MAX_DATA_IDENTIFIERS; i++) {
        if (!global_database.data_by_identifier[i].valid) {
            global_database.data_by_identifier[i].identifier = identifier;
            memcpy(global_database.data_by_identifier[i].data, data, data_length);
            global_database.data_by_identifier[i].data_length = data_length;
            global_database.data_by_identifier[i].valid = true;
            return true;
        }
    }

    return false; // No space left in the database
}

void send_positive_response(uint8_t original_sid, uint8_t* data, uint32_t data_length) {
    uint8_t response_sid = original_sid + SID_POS_RESPONSE;
    uint8_t response_data[1] = {response_sid};  // Normally, there might be additional data in a positive response

    if (data == NULL || data_length == 0) {
        // No data to send, just send the SID
        ctp_send(RESPONSE_CAN_ID, response_data, 1);
        return;
    }

    memcpy(response_data + 1, data, data_length);
    
    // Send the response using the CTP (CAN Transport Protocol) send function
    ctp_send(RESPONSE_CAN_ID, response_data, sizeof(response_data));
}

void send_negative_response(uint8_t original_sid, uint8_t error_code) {
    uint8_t response_data[3] = {SID_NEGATIVE_RESPONSE, original_sid, error_code};
    
    // Send the response using the CTP (CAN Transport Protocol) send function
    ctp_send(RESPONSE_CAN_ID, response_data, sizeof(response_data));
}

void send_response(uint16_t sid, uint8_t* data, uint32_t data_length) {
    // Check if data can fit in our buffer
    if (data_length + 1 > 1024) {
        // Handle error - data too large to fit in buffer
        printf("Error: Data too large to fit in buffer.\n");
        return;
    }
    
    // Populate the buffer with the SID as the first byte, followed by the actual data
    response_buffer[0] = sid + SID_POS_RESPONSE;   // First byte is SID
    memcpy(response_buffer + 1, data, data_length);  // Copy the actual data after the SID

    // Send the buffer using CTP
    ctp_send(RESPONSE_CAN_ID, response_buffer, data_length + 1);
}

void handle_message(uint8_t sid, uint8_t* data, uint32_t data_length) {
    switch (sid) {
        case SID_SECURITY_ACCESS:
            security_access(data, data_length);
            break;

        case SID_READ_DATA_BY_IDENTIFIER:
            read_data_by_identifier(sid, data, data_length);
            break;

        case SID_WRITE_DATA_BY_ID:
            write_data_by_identifier(*((uint16_t*)data), data + 2, data_length - 2);
            break;

        case SID_ROUTINE_CONTROL:
            routine_control(*((uint16_t*)data));
            break;

        case SID_REQUEST_DOWNLOAD:
            request_download(data, data_length);
            break;

        case SID_READ_ERROR_CODE:
            read_error_code_information();
            break;

        case SID_SYSTEM_RESET:
            system_reset_request();
            break;

        case SID_SESSION_CONTROL:
            session_control(data[0]);
            break;

        default:
            send_negative_response(sid, ERROR_CODE_UNSUPPORTED);
            break;
    }
}


// Read Data By Identifier
void read_data_by_identifier(uint16_t identifier, uint8_t* response_data, uint8_t* response_length) {
    uint8_t data[MAX_IDENTIFIER_VALUE_SIZE];
    uint8_t data_length;

    if (get_data_by_identifier(identifier, data, &data_length)) {
        // If the identifier exists in the database, prepare a positive response
        response_data[0] = SID_POS_RESPONSE + SID_READ_DATA_BY_IDENTIFIER;
        response_data[1] = (identifier >> 8) & 0xFF; // High byte of identifier
        response_data[2] = identifier & 0xFF;        // Low byte of identifier
        memcpy(&response_data[3], data, data_length);
        *response_length = 3 + data_length;
    } else {
        // If the identifier doesn't exist in the database, prepare a negative response
        response_data[0] = SID_NEGATIVE_RESPONSE;
        response_data[1] = SID_READ_DATA_BY_IDENTIFIER;
        response_data[2] = SID_REQ_OUT_OF_RANGE;
        *response_length = 3;
    }
}

void write_data_by_identifier(uint16_t identifier, uint8_t* data, uint32_t data_length) {
    // Find the identifier in the database
    if (get_data_by_identifier(identifier, data, data_length)) {
        // Update the data in the database
        set_data_by_identifier(identifier, data, data_length);

        // Send a positive response
        send_positive_response(SID_WRITE_DATA_BY_ID, NULL, 0);
    } else {
        // Identifier not found, send a negative response
        send_negative_response(SID_WRITE_DATA_BY_ID, ERROR_CODE_NOT_FOUND);
    }
}

void routine_control(uint16_t routine_id) {
    // Mock routine control. In a real-world scenario, this would be more complex.
    if (routine_id == ROUTINE_ID_MOCK) {
        // Execute the routine
        execute_mock_routine(routine_id);

        // Send a positive response
        send_positive_response(SID_ROUTINE_CONTROL, NULL, 0);
    } else {
        // Routine not recognized, send a negative response
        send_negative_response(SID_ROUTINE_CONTROL, ERROR_CODE_NOT_FOUND);
    }
}

bool execute_mock_routine(uint8_t routine_id) {
    // Mock implementation of a routine.
    printf("Executing mock routine with ID: %02X\n", routine_id);
    // Simulate the execution of the routine. 
    // Here, we're just printing the action. In a real implementation, you'd have actual logic.
    if(routine_id == 0x01) {
        printf("Mock Routine 1 executed.\n");
        return true;
    }
    else if(routine_id == 0x02) {
        printf("Mock Routine 2 executed.\n");
        return true;
    }
    else {
        printf("Unknown routine ID.\n");
        return false;
    }
}

void request_download(uint8_t* file_data, uint32_t file_size) {
    // Mock download implementation
    // In reality, this would involve storing the file, possibly on non-volatile memory
    store_file("file", file_data, file_size);

    // Send a positive response
    send_positive_response(SID_REQUEST_DOWNLOAD, NULL, 0);
}

void store_file(const char* file_name, uint8_t* data, uint32_t size) {
    // Mock implementation to store a file
    printf("Mock storing file: %s\n", file_name);
    // Here, we're just printing the action, but in a real implementation, you'd store the data
    for(uint16_t i = 0; i < size; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

// This function populates the provided buffer with error codes from the system or device.
// Returns the number of error codes retrieved.
int get_error_codes(ErrorCodeInfo *buffer, int max_entries) {
    int count = 0;
    for (int i = 0; i < num_error_codes && i < max_entries; i++) {
        buffer[i] = error_codes_db[i];
        count++;
    }
    return count;
}

void read_error_code_information() {
    // Retrieve error codes
    int codes = get_error_codes(error_codes_db, MAX_ERROR_CODES);

    // Send the error codes as a response (for simplicity, sending directly)
    send_response(SID_READ_ERROR_CODE, (uint8_t*)error_codes_db, codes * sizeof(ErrorCodeInfo));
}

void system_reset_request() {
    // Reset the system (mocked)
    mock_system_reset();

    // Send a positive response before resetting
    send_positive_response(SID_SYSTEM_RESET, NULL, 0);
}

bool mock_system_reset() {
    printf("Executing mock system reset.\n");
    // Here, we're just printing the action. 
    // In a real-world scenario, this function would reset the system or perform the necessary operations.
    return true;
}

void session_control(uint8_t session_type) {
    if (session_type == DEFAULT_SESSION || session_type == EXTENDED_SESSION) {
        current_session = session_type;

        // Send a positive response
        send_positive_response(SID_SESSION_CONTROL, NULL, 0);
    } else {
        // Invalid session type, send a negative response
        send_negative_response(SID_SESSION_CONTROL, ERROR_CODE_INVALID_SESSION_TYPE);
    }
}

// TODO:TEST VALUES
#define SECURITY_SEED 0x1234  // Mock seed for example purposes
#define EXPECTED_KEY  0x5678  // Mock expected key for example purposes

void security_access(uint8_t sub_function, uint16_t data) {
    switch (sub_function) {
        case REQUEST_SEED:
            // Send seed to client
            send_positive_response(SID_SECURITY_ACCESS, SECURITY_SEED, sizeof(SECURITY_SEED));
            break;

        case SEND_KEY:
            if (current_security_level == SECURITY_LEVEL_UNLOCKED) {
                // Already unlocked, send positive response
                send_positive_response(SID_SECURITY_ACCESS, NULL, 0);
            } else if (data == EXPECTED_KEY) {
                // Correct key received, unlock security access
                current_security_level = SECURITY_LEVEL_UNLOCKED;
                send_positive_response(SID_SECURITY_ACCESS, NULL, 0);
            } else {
                // Incorrect key, send negative response
                send_negative_response(SID_SECURITY_ACCESS, ERROR_INCORRECT_SECURITY_KEY);
            }
            break;

        default:
            // Invalid sub-function, send negative response
            send_negative_response(SID_SECURITY_ACCESS, ERROR_SUBFUNCTION_NOT_SUPPORTED);
            break;
    }
}


