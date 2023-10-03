#ifndef CTP_H  
#define CTP_H

#include <stdint.h>
#include <stdbool.h>

// Define maximum CAN data length
#define CAN_MAX_DATA_LENGTH 8
#define MAX_SEQUENCE_LEN 255

#define CTP_START_DATA_SIZE 6
#define CTP_CONSECUTIVE_DATA_LENGTH 6
#define CTP_END_DATA_LENGTH 7
#define CTP_ERROR_DATA_LENGTH 7

// Define CTP frame types
typedef enum {
    CTP_START_FRAME,
    CTP_CONSECUTIVE_FRAME,
    CTP_END_FRAME,
    CTP_FLOW_CONTROL_FRAME,
    CTP_ERROR_FRAME,
} CTP_FrameType;

// Define CTP flow control commands
typedef enum {
    CTP_CONTINUE_SENDING,
    CTP_WAIT,
    CTP_ABORT
} CTP_FlowControl;

// Define CTP error codes
typedef enum {
    CTP_MESSAGE_TIMEOUT,
    CTP_INVALID_SEQUENCE_NUMBER,
    CTP_INVALID_FRAME_TYPE,
    CTP_INVALID_FRAME_LENGTH
} CTP_ErrorCode;

// CTP frame structure
typedef struct {
    uint32_t id; // CAN ID
    CTP_FrameType type;
    union {
        struct {
            uint8_t payload_len;                // Length of the whole data payload
            uint8_t data[CTP_START_DATA_SIZE];  // 1 byte for length
        } start;
        struct {
            uint8_t sequence;
            uint8_t data[CTP_CONSECUTIVE_DATA_LENGTH]; // 1 byte for sequence number
        } consecutive;
        struct {
            uint8_t data[CTP_END_DATA_LENGTH]; 
        } end;
        struct {
            CTP_FlowControl control;
        } flowControl;
        struct {
            CTP_ErrorCode errorCode;
            uint8_t data[CTP_ERROR_DATA_LENGTH];
        } error;
    } payload;
} CTP_Frame;


// Protocol interface functions
void ctp_send_frame(const CTP_Frame *frame, uint8_t len);
uint32_t ctp_send(uint32_t id, uint8_t *data, uint32_t length);
int32_t ctp_receive(uint8_t* buffer, uint32_t buffer_size);
int32_t ctp_receive_bytes(uint8_t *buffer, uint32_t length);

// CAN driver interface functions, this functions must be implemented by the user
// and is used by the protocol to send and receive CAN messages
bool send_ctp_message(uint32_t id, uint8_t *data, uint8_t length);
bool receive_ctp_message(uint32_t *id, uint8_t *data, uint8_t *length);

#endif