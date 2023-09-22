#ifndef UDS_H
#define UDS_H

#include <stdint.h>
#include <stdbool.h>


// Assuming each identifier corresponds to a 4-byte value for simplicity
#define MAX_IDENTIFIER_VALUE_SIZE 4

// Maximum number of data identifiers in the system
#define MAX_DATA_IDENTIFIERS 100

#define MAX_APP_LAYER_DATA_LENGTH (1024 - 1) // 1 byte reserved for SID

// Service IDs
#define SID_READ_DATA_BY_IDENTIFIER     0x02
#define SID_ROUTINE_CONTROL             0x04
#define SID_REQUEST_DOWNLOAD            0x34
#define SID_REQ_OUT_OF_RANGE            0x31
#define SID_WRITE_DATA_BY_ID            0x2E
#define SID_READ_ERROR_CODE             0x19
#define SID_SYSTEM_RESET                0x11
#define SID_SECURITY_ACCESS             0x27
#define SID_SESSION_CONTROL             0x10
#define SID_POS_RESPONSE                0x40
#define SID_NEGATIVE_RESPONSE           0x7F

// Error codes
#define ERROR_CODE_INVALID_SESSION_TYPE 0x7E
#define ERROR_CODE_UNSUPPORTED          0x12
#define ERROR_CODE_NOT_FOUND            0x31
#define ERROR_INCORRECT_SECURITY_KEY    0x35
#define ERROR_SUBFUNCTION_NOT_SUPPORTED 0x12


// Session levels
#define DEFAULT_SESSION 0x01
#define EXTENDED_SESSION 0x02

// Routine IDs
#define ROUTINE_ID_MOCK 0x1234

// USER CONFIGURATION IDs
#define RESPONSE_CAN_ID                 0x123


#define MAX_ERROR_CODES 10

// Sub-functions for Security Access
#define REQUEST_SEED 0x01
#define SEND_KEY     0x02


#define SECURITY_LEVEL_LOCKED       0x00
#define SECURITY_LEVEL_UNLOCKED     0x01


// Define a structure for Data By Identifier
typedef struct {
    uint16_t identifier; // The identifier for this data
    uint8_t data[MAX_IDENTIFIER_VALUE_SIZE]; // The data value
    uint8_t data_length; // Actual data length
    bool valid; // Indicates if this data identifier is populated and valid
} DataByIdentifier;

// Define a structure for the global database
typedef struct {
    DataByIdentifier data_by_identifier[MAX_DATA_IDENTIFIERS];
    // ... any other global data or settings for the system
} SystemDatabase;

typedef struct {
    uint16_t error_code;
    uint8_t status;  // 0: Not Active, 1: Active, etc.
    // Additional fields if necessary, such as timestamp, module ID, etc.
} ErrorCodeInfo;

typedef struct {
    uint8_t sid; // Service Identifier
    uint8_t data_length; // Length of the data
    uint8_t data[MAX_APP_LAYER_DATA_LENGTH]; // Pointer to the data
} AppLayerMessage;


void store_file(const char* file_name, uint8_t* data, uint32_t size);
bool mock_system_reset();
void read_error_code_information();
bool execute_mock_routine(uint8_t routine_id);
void request_download(uint8_t* file_data, uint32_t file_size);
void routine_control(uint16_t routine_id);
void write_data_by_identifier(uint16_t identifier, uint8_t* data, uint32_t data_length);
void read_data_by_identifier(uint16_t identifier, uint8_t* response_data, uint8_t* response_length);
bool get_data_by_identifier(uint16_t identifier, uint8_t* data, uint8_t* data_length);
bool set_data_by_identifier(uint16_t identifier, const uint8_t* data, uint8_t data_length);
void send_positive_response(uint8_t original_sid, uint8_t* data, uint32_t data_length);
void send_negative_response(uint8_t original_sid, uint8_t error_code);
void send_response(uint16_t sid, uint8_t* data, uint32_t data_length);
void security_access(uint8_t sub_function, uint16_t data);
void session_control(uint8_t session_type);
void system_reset_request();

#endif