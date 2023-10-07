#ifndef CTP_H  
#define CTP_H

#include <stdint.h>
#include <stdbool.h>

// Define maximum CAN data length
#define CAN_MAX_DATA_LENGTH 64
#define MAX_SEQUENCE_NUM 255

#define CTP_START_DATA_SIZE 5
#define CTP_CONSECUTIVE_DATA_LENGTH 6
#define CTP_END_DATA_LENGTH 7
#define CTP_ERROR_DATA_LENGTH 7

#define CTP_FD_START_DATA_SIZE 61
#define CTP_FD_CONSECUTIVE_DATA_LENGTH 62
#define CTP_FD_END_DATA_LENGTH 63
#define CTP_FD_ERROR_DATA_LENGTH 63

#define CTP_START_FRAME_HEADER_SIZE 3
#define CTP_CONSECUTIVE_FRAME_HEADER_SIZE 2
#define CTP_END_FRAME_HEADER_SIZE 1


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
            uint16_t payload_len;                   // Length of the whole data payload
            uint8_t data[CTP_FD_START_DATA_SIZE];   // 2 bytes for length
        } start;
        struct {
            uint8_t sequence;
            uint8_t data[CTP_FD_CONSECUTIVE_DATA_LENGTH]; // 1 byte for sequence number
        } consecutive;
        struct {
            uint8_t data[CTP_FD_END_DATA_LENGTH]; 
        } end;
        struct {
            CTP_ErrorCode errorCode;
            uint8_t data[CTP_FD_ERROR_DATA_LENGTH];
        } error;
    } payload;
} CTP_Frame;


// Protocol interface functions
void ctp_send_frame(const CTP_Frame *frame, uint8_t len);
uint32_t ctp_send(uint32_t id, uint8_t *data, uint32_t length, bool fd);
int32_t ctp_receive(uint8_t* buffer, uint32_t buffer_size, bool fd);
int32_t ctp_receive_bytes(uint8_t *buffer, uint32_t length, bool fd);

// CAN driver interface functions, this functions must be implemented by the user
// and is used by the protocol to send and receive CAN messages
// Don't pass all CAN messages to the protocol, only the ones with the correct ID
// or the ids/messages set aside for the protocol
bool send_ctp_message(uint32_t id, uint8_t *data, uint8_t length);
bool receive_ctp_message(uint32_t *id, uint8_t *data, uint8_t *length);

#endif