#ifndef CTP_H  
#define CTP_H

#include <stdint.h>

// Define maximum CAN data length
#define CAN_MAX_DATA_LENGTH 8
#define MAX_SEQUENCE_NUMBER 255
#define MAX_BUFFER_SIZE 1024


typedef struct {
    uint8_t data[MAX_BUFFER_SIZE];
    uint32_t length;
} DataBuffer;

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
    CTP_MESSAGE_TIMEOUT
} CTP_ErrorCode;

// CTP frame structure
typedef struct {
    uint32_t id; // CAN ID
    CTP_FrameType type;
    union {
        struct {
            uint8_t length;
            uint8_t data[CAN_MAX_DATA_LENGTH - 1]; // 1 byte for length
        } start;
        struct {
            uint8_t sequence;
            uint8_t data[CAN_MAX_DATA_LENGTH - 1]; // 1 byte for sequence number
        } consecutive;
        struct {
            uint8_t data[CAN_MAX_DATA_LENGTH]; 
        } end;
        struct {
            CTP_FlowControl control;
        } flowControl;
        struct {
            CTP_ErrorCode errorCode;
        } error;
    } payload;
} CTP_Frame;



// Protocol interface functions
void ctp_send_frame(const CTP_Frame *frame);
bool ctp_receive_frame(CTP_Frame *frame);
void ctp_process_frame(const CTP_Frame *frame);
void ctp_send(uint32_t id, const uint8_t *data, uint8_t length);
bool ctp_receive_data(uint32_t expected_id, uint8_t* buffer, uint32_t* received_length);
bool get_flow_control_state();
void set_expected_sequence_number(uint8_t sequence_number);
uint8_t get_expected_sequence_number();

// CAN driver interface functions, this functions must be implemented by the user
// and is used by the protocol to send and receive CAN messages
bool send_can_message(uint32_t id, const uint8_t *data, uint8_t length);
bool receive_can_message(uint32_t *id, uint8_t *data, uint8_t *length);

#endif