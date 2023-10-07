#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#include "ctp.h"


// Flag to capture what the mock driver send function sends
// Think of these as testing probes
uint8_t last_sent_data[CAN_MAX_DATA_LENGTH];
uint32_t last_sent_id;

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
    
    last_sent_id = id;
    memcpy(last_sent_data, data, length);
    enqueue_mock_frame(id, data, length);
    return true; // Simulate successful send
}

void ctp_process_frame(const CTP_Frame *frame) {
    // Process the received CTP frame
     switch (frame->type) {
        case CTP_START_FRAME:
            printf("Received START FRAME with length %u and data: ", frame->payload.start.payload_len);
            for (int i = 0; i < CTP_START_DATA_SIZE; i++) {
                printf("%02X ", frame->payload.start.data[i]);
            }
            printf("\n");
            break;
        case CTP_CONSECUTIVE_FRAME:
            printf("Received CONSECUTIVE FRAME with sequence %u and data: ", frame->payload.consecutive.sequence);
            for (int i = 0; i < CTP_CONSECUTIVE_DATA_LENGTH; i++) {
                printf("%02X ", frame->payload.consecutive.data[i]);
            }
            printf("\n");
            break;
        case CTP_END_FRAME:
            printf("Received END FRAME with data: ");
            for (int i = 0; i < CTP_END_DATA_LENGTH; i++) {
                printf("%02X ", frame->payload.end.data[i]);
            }
            printf("\n");
            break;
        default:
            break;
    }
}

// Modified Mock driver function
bool receive_ctp_message(uint32_t *id, uint8_t *data, uint8_t *length) {    
    // If we have no more frames to dequeue, return false
    if (mock_frame_index >= mock_frame_count) {
        printf("[DEBUG] No more frames to dequeue\n");
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


bool test_send() {
    CTP_Frame test_frame;
    test_frame.id = 123;
    test_frame.type = CTP_START_FRAME;
    test_frame.payload.start.payload_len = 3;
    test_frame.payload.start.data[0] = 0xAA;
    test_frame.payload.start.data[1] = 0xBB;
    test_frame.payload.start.data[2] = 0xCC;

    ctp_send_frame(&test_frame, 3);

    // Check if the driver_send_can_message function was called with the correct data
    assert(last_sent_id == test_frame.id);
    assert(last_sent_data[0] == test_frame.type);
    assert(last_sent_data[1] == (test_frame.payload.start.payload_len >> 8));
    assert(last_sent_data[2] == (test_frame.payload.start.payload_len & 0xFF));
    assert(last_sent_data[3] == 0xAA && last_sent_data[4] == 0xBB && last_sent_data[5] == 0xCC);

    return true;
}

// Revised test function
bool test_send_long() {
    mock_frame_count = 0;
    mock_frame_index = 0;

    // Actual test
    uint32_t test_id = 456;
    uint8_t data[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};
    uint32_t bytes_sent = ctp_send(test_id, data, sizeof(data), false);

    // Check if the last sent frame is an END_FRAME
    assert(last_sent_data[0] == CTP_END_FRAME);
    assert(bytes_sent == sizeof(data));
    printf("SEQ: 1 Passed\n");

    mock_frame_count = 0;
    mock_frame_index = 0;

    uint8_t data2[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    bytes_sent = ctp_send(test_id, data2, sizeof(data2), false);

    // Check if the last sent frame is an END_FRAME
    assert(last_sent_data[0] == CTP_END_FRAME);
    assert(bytes_sent == sizeof(data2));
    printf("SEQ: 2 Passed\n");

    mock_frame_count = 0;
    mock_frame_index = 0;

    uint8_t data3[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
    bytes_sent = ctp_send(test_id, data3, sizeof(data3), false);

    // Check if the last sent frame is an END_FRAME
    assert(last_sent_data[0] == CTP_END_FRAME);
    assert(bytes_sent == sizeof(data3));
    printf("SEQ: 3 Passed\n");

    uint8_t data4[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};

    
    bytes_sent = ctp_send(test_id, data4, sizeof(data4), false);
    printf("Bytes sent: %d\n", bytes_sent);

    // Check if the last sent frame is an END_FRAME
    assert(last_sent_data[0] == CTP_END_FRAME);
    assert(bytes_sent == sizeof(data4));
    printf("SEQ: 4 Passed\n");

    return true;
}

bool test_send_long_fd() {
    mock_frame_count = 0;
    mock_frame_index = 0;

    // Actual test
    uint32_t test_id = 456;
    uint8_t data[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};
    uint32_t bytes_sent = ctp_send(test_id, data, sizeof(data), true);

    assert(bytes_sent == sizeof(data));
    printf("SEQ: 1 Passed\n");

    mock_frame_count = 0;
    mock_frame_index = 0;

    uint8_t data2[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    bytes_sent = ctp_send(test_id, data2, sizeof(data2), true);

    assert(bytes_sent == sizeof(data2));
    printf("SEQ: 2 Passed\n");

    mock_frame_count = 0;
    mock_frame_index = 0;

    uint8_t data3[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
    bytes_sent = ctp_send(test_id, data3, sizeof(data3), true);

    assert(bytes_sent == sizeof(data3));
    printf("SEQ: 3 Passed\n");

    uint8_t data4[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};

    
    bytes_sent = ctp_send(test_id, data4, sizeof(data4), true);
    printf("Bytes sent: %d\n", bytes_sent);

    // Check if the last sent frame is an END_FRAME
    assert(last_sent_data[0] == CTP_END_FRAME);
    assert(bytes_sent == sizeof(data4));
    printf("SEQ: 4 Passed\n");

    return true;
}

bool test_send_with_end_frame() {
    mock_frame_count = 0;
    mock_frame_index = 0;

    // Actual test
    uint32_t test_id = 456;
    uint8_t data[15] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};
    ctp_send(test_id, data, sizeof(data), false);

    // Check if the last sent frame is an END_FRAME
    assert(last_sent_data[0] == CTP_END_FRAME);

    return true;
}

bool test_ctp_receive() {
    // Reset any global state
    mock_frame_count = 0;
    mock_frame_index = 0;

    // Given ID for the test
    uint32_t test_id = 123;

    uint8_t expected_data[] = {0xAA, 0xBB, 0xCC, 0x00, 0x00, 
                               0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 
                               0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33,
                               0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33,
                               0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33,
                               0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33,
                               0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33,
                               0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x11};

    uint8_t len = sizeof(expected_data);

    // Mock receiving a START frame
    enqueue_mock_frame(test_id, (uint8_t[]){CTP_START_FRAME, 0x00, len, 0xAA, 0xBB, 0xCC, 0x00, 0x00}, 8);

    // Mock receiving a CONSECUTIVE frame
    enqueue_mock_frame(test_id, (uint8_t[]){CTP_CONSECUTIVE_FRAME, 0, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33}, 8);
    enqueue_mock_frame(test_id, (uint8_t[]){CTP_CONSECUTIVE_FRAME, 1, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33}, 8);
    enqueue_mock_frame(test_id, (uint8_t[]){CTP_CONSECUTIVE_FRAME, 2, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33}, 8);
    enqueue_mock_frame(test_id, (uint8_t[]){CTP_CONSECUTIVE_FRAME, 3, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33}, 8);
    enqueue_mock_frame(test_id, (uint8_t[]){CTP_CONSECUTIVE_FRAME, 4, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33}, 8);
    enqueue_mock_frame(test_id, (uint8_t[]){CTP_CONSECUTIVE_FRAME, 5, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33}, 8);

    // Mock receiving an END frame
    enqueue_mock_frame(test_id, (uint8_t[]){CTP_END_FRAME, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x11}, 8);

    // Buffer to hold received data
    uint8_t received_data[1024];

    // Call the function
    uint32_t data_len = ctp_receive(received_data, sizeof(received_data), false);

    assert(data_len == sizeof(expected_data));
    assert(memcmp(received_data, expected_data, data_len) == 0);
    printf("SEQ: 1 Passed\n");

    return true;
}

bool test_ctp_receive_self() {
    mock_frame_count = 0;
    mock_frame_index = 0;
    uint8_t received_data[1024];
    uint32_t test_id = 123;

    uint8_t expected_data2[] = {0xAA, 0xBB, 0xCC, 0x00, 0x00, 0x00, 
                               0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 
                               0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33,
                               0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33,
                               0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33,
                               0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33,
                               0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33,
                               0x44, 0x55, 0x66, 0x77, 0x88, 0x99};

    uint32_t bytes_sent = ctp_send(test_id, expected_data2, sizeof(expected_data2), false);
    uint32_t data_len = ctp_receive(received_data, sizeof(received_data), false);

    printf("Bytes sent: %d\n", bytes_sent);
    printf("Data len: %d\n", data_len);
    assert(bytes_sent == data_len);
    assert(data_len == sizeof(expected_data2));
    assert(memcmp(received_data, expected_data2, data_len) == 0);
    printf("SEQ: 0 Passed\n");

        uint8_t data3[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};

    bytes_sent = ctp_send(test_id, data3, sizeof(data3), false);
    data_len = ctp_receive_bytes(received_data, bytes_sent, false);

    printf("Bytes sent: %d\n", bytes_sent);
    printf("Data len: %d\n", data_len);
    assert(bytes_sent == data_len);
    assert(data_len == sizeof(data3));
    assert(memcmp(received_data, data3, sizeof(data3)) == 0);
    printf("SEQ: 1 Passed\n");

    return true;
}

bool ctp_test_first_frame() {
    mock_frame_count = 0;
    mock_frame_index = 0;

    uint32_t test_id = 123;
    uint8_t received_data[16];
    uint8_t expected_data[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
    uint8_t len = sizeof(expected_data);
    enqueue_mock_frame(test_id, (uint8_t[]){CTP_START_FRAME, 0x00, len, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE}, 8);

    uint32_t data_len = ctp_receive(received_data, sizeof(received_data), false);

    assert(data_len == sizeof(expected_data));
    assert(memcmp(received_data, expected_data, data_len) == 0);

    return true;
}

bool ctp_test_small_first_frame() {
    mock_frame_count = 0;
    mock_frame_index = 0;

    uint32_t test_id = 123;
    uint8_t received_data[16];
    uint8_t expected_data[] = {0xAA, 0xBB, 0xCC};
    uint8_t len2 = sizeof(expected_data);
    enqueue_mock_frame(test_id, (uint8_t[]){CTP_START_FRAME, 0x00, len2, 0xAA, 0xBB, 0xCC}, 6);

    uint32_t data_len = ctp_receive(received_data, sizeof(received_data), false);

    assert(data_len == sizeof(expected_data));
    assert(memcmp(received_data, expected_data, data_len) == 0);

    return true;
}


int main() {
    if (test_send()) {
        printf("Test Send: PASSED\n");
    } else {
        printf("Test Send: FAILED\n");
    }

    if (test_send_with_end_frame()) {
        printf("Test Send with End Frame: PASSED\n");
    } else {
        printf("Test Send with End Frame: FAILED\n");
    }

    if (test_send_long()) {
        printf("Test Send Long: PASSED\n");
    } else {
        printf("Test Send Long: FAILED\n");
    }

    if (test_send_long_fd()) {
        printf("Test Send Long FD: PASSED\n");
    } else {
        printf("Test Send Long FD: FAILED\n");
    }

    if (ctp_test_first_frame()) {
        printf("Test First Frame: PASSED\n");
    } else {
        printf("Test First Frame: FAILED\n");
    }

    if (ctp_test_small_first_frame()) {
        printf("Test Small First Frame: PASSED\n");
    } else {
        printf("Test Small First Frame: FAILED\n");
    }

    if (test_ctp_receive()) {
        printf("Test Receive Data PASSED.\n");
    } else {
        printf("Test Receive Data FAILED.\n");
    }

    if (test_ctp_receive_self()) {
        printf("Test Receive Self PASSED.\n");
    } else {
        printf("Test Receive Self FAILED.\n");
    }

    return 0;
}