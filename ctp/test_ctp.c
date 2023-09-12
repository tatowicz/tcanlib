#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#include "ctp.h"


#define MAX_MOCK_FRAMES 10

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
MockFrame mock_frames[MAX_MOCK_FRAMES];
int mock_frame_count = 0;
int mock_frame_index = 0;

// Function to enqueue a mock frame
void enqueue_mock_frame(uint32_t id, uint8_t *data, uint8_t length) {
    printf("[DEBUG] Enqueueing mock frame with ID: %u, Data: ", id);
    for (int i = 0; i < length; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");

    if (mock_frame_count < MAX_MOCK_FRAMES) {
        mock_frames[mock_frame_count].id = id;
        memcpy(mock_frames[mock_frame_count].data, data, length);
        mock_frames[mock_frame_count].length = length;
        mock_frame_count++;
    }
}

// Mock driver function to send a CAN message to the bus
bool send_can_message(uint32_t id, const uint8_t *data, uint8_t length) {
    printf("Sending CAN message with ID: %u, Data: ", id);
    for (int i = 0; i < length; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
    
    last_sent_id = id;
    memcpy(last_sent_data, data, length);
    return true; // Simulate successful send
}

// Modified Mock driver function
bool receive_can_message(uint32_t *id, uint8_t *data, uint8_t *length) {    
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
    test_frame.payload.start.length = 3;
    test_frame.payload.start.data[0] = 0xAA;
    test_frame.payload.start.data[1] = 0xBB;
    test_frame.payload.start.data[2] = 0xCC;

    ctp_send_frame(&test_frame);

    // Check if the driver_send_can_message function was called with the correct data
    assert(last_sent_id == test_frame.id);
    assert(last_sent_data[0] == test_frame.type);
    assert(last_sent_data[1] == test_frame.payload.start.length);
    assert(last_sent_data[2] == 0xAA && last_sent_data[3] == 0xBB && last_sent_data[4] == 0xCC);

    return true;
}

bool test_receive() {
    CTP_Frame received_frame;

    mock_frame_count = 0;
    mock_frame_index = 0;

    enqueue_mock_frame(123, (uint8_t[]){CTP_START_FRAME, 03, 0xAA, 0xBB, 0xCC, 0x7F, 0x00, 0x00}, 8);

    // Simulate a received message for testing
    assert(ctp_receive_frame(&received_frame));
    
    assert(received_frame.id == 123);

    // Check if the received data matches what we sent in the mock driver function
    assert(received_frame.type == CTP_START_FRAME);
    assert(received_frame.payload.start.length == 3);
    assert(received_frame.payload.start.data[0] == 0xAA);
    assert(received_frame.payload.start.data[1] == 0xBB);
    assert(received_frame.payload.start.data[2] == 0xCC);

    return true;
}

bool test_send_with_flow_control() {
    uint32_t test_id = 456;
    uint8_t data[] = {0xAA, 0xBB, 0xCC};

    ctp_send(test_id, data, sizeof(data));

    // We'll assume the function is correct if we're not waiting for flow control at the end
    return !get_flow_control_state();
}

//bool test_receive_with_flow_control() {
//    CTP_Frame received_frame;
//
//    // Simulate receiving a start frame
//    received_frame.id = 789;
//    received_frame.type = CTP_START_FRAME;
//    received_frame.payload.start.length = 3;
//    received_frame.payload.start.data[0] = 0x01;
//    received_frame.payload.start.data[1] = 0x02;
//    received_frame.payload.start.data[2] = 0x03;
//
//    ctp_receive_frame(&received_frame);
//
//    // Check if we sent a CONTINUE_SENDING flow control command
//    assert(last_sent_id != received_frame.id);
//    assert(last_sent_data[0] != CTP_FLOW_CONTROL_FRAME);
//    assert(last_sent_data[1] != CTP_CONTINUE_SENDING);
//
//    return true;
//}

bool test_processing_with_variable_ids() {
    CTP_Frame frame1, frame2;

    enqueue_mock_frame(123, (uint8_t[]){CTP_START_FRAME, 03, 0x11, 0x22, 0x33, 0x7F, 0x00, 0x00}, 8);
    enqueue_mock_frame(456, (uint8_t[]){CTP_START_FRAME, 03, 0x44, 0x55, 0x66, 0x7F, 0x00, 0x00}, 8);

    ctp_receive_frame(&frame1);
    ctp_receive_frame(&frame2);

    assert(frame1.type == CTP_START_FRAME);
    assert(frame2.type == CTP_START_FRAME);

    assert(memcmp(frame1.payload.start.data, (uint8_t[]){0x11, 0x22, 0x33}, frame1.payload.start.length) == 0);
    assert(memcmp(frame2.payload.start.data, (uint8_t[]){0x44, 0x55, 0x66}, frame2.payload.start.length) == 0);

    return true;
}

// Revised test function
bool test_send_with_end_frame() {
    mock_frame_count = 0;
    mock_frame_index = 0;

    // Enqueue expected flow control frames
    //enqueue_mock_frame(456, (uint8_t[]){CTP_FLOW_CONTROL_FRAME, CTP_CONTINUE_SENDING}, 2);
    //enqueue_mock_frame(456, (uint8_t[]){CTP_FLOW_CONTROL_FRAME, CTP_CONTINUE_SENDING}, 2);

    // Actual test
    uint32_t test_id = 456;
    uint8_t data[15] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};
    ctp_send(test_id, data, sizeof(data));

    // Check if the last sent frame is an END_FRAME
    assert(last_sent_data[0] == CTP_END_FRAME);

    return true;
}

bool test_receive_end_frame() {
    mock_frame_count = 0;
    mock_frame_index = 0;

    // Set up a mock response for an END_FRAME
    enqueue_mock_frame(123, (uint8_t[]){CTP_END_FRAME, 0xAA, 0xBB, 0xCC, 0x7F, 0x00, 0x00, 0x00}, 8);

    CTP_Frame received_frame;
    assert(ctp_receive_frame(&received_frame));

    // Check if the received frame is an END_FRAME
    assert(received_frame.type == CTP_END_FRAME);

    return true;
}

bool test_unexpected_end_frame() {
    // Reset state variables
    set_expected_sequence_number(5); // Expecting a CONSECUTIVE_FRAME with sequence 5
    mock_frame_count = 0;
    mock_frame_index = 0;

    // Set up a mock response for an END_FRAME
    enqueue_mock_frame(123, (uint8_t[]){CTP_END_FRAME, 0xAA, 0xBB, 0xCC, 0x7F, 0x00, 0x00, 0x00}, 8);

    CTP_Frame received_frame;
    assert(ctp_receive_frame(&received_frame));

    // Check if the received frame is an END_FRAME when we were expecting a CONSECUTIVE_FRAME
    // This represents an error scenario
    assert(received_frame.type == CTP_END_FRAME);
    assert(get_expected_sequence_number() == 5); // Ensure our expected sequence hasn't changed

    return true;
}

bool test_ctp_receive_data() {
    // Reset any global state
    mock_frame_count = 0;
    mock_frame_index = 0;

    // Given ID for the test
    uint32_t test_id = 123;

    // Mock receiving a START frame
    enqueue_mock_frame(test_id, (uint8_t[]){CTP_START_FRAME, 03, 0xAA, 0xBB, 0xCC, 0x00, 0x00, 0x00}, 8);

    // Mock receiving a CONSECUTIVE frame
    enqueue_mock_frame(test_id, (uint8_t[]){CTP_CONSECUTIVE_FRAME, 1, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33}, 8);

    // Mock receiving an END frame
    enqueue_mock_frame(test_id, (uint8_t[]){CTP_END_FRAME, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x11}, 8);

    // Buffer to hold received data
    uint8_t received_data[MAX_BUFFER_SIZE];
    uint32_t received_length = 0;

    // Call the function
    bool success = ctp_receive_data(test_id, received_data, &received_length);

    // Validate the result
    assert(success == true);
    assert(received_length == 16); // 3 bytes from start + 6 bytes from consecutive + 7 bytes from end

    uint8_t expected_data[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
                               0x77, 0x88, 0x99, 0x11};
    assert(memcmp(received_data, expected_data, received_length) == 0);

    return true;
}


int main() {
    if (test_send()) {
        printf("Test Send: PASSED\n");
    } else {
        printf("Test Send: FAILED\n");
    }

    if (test_receive()) {
        printf("Test Receive: PASSED\n");
    } else {
        printf("Test Receive: FAILED\n");
    }

    if (test_processing_with_variable_ids()) {
        printf("Test Processing with Variable IDs: PASSED\n");
    } else {
        printf("Test Processing with Variable IDs: FAILED\n");
    }

    if (test_send_with_end_frame()) {
        printf("Test Send with End Frame: PASSED\n");
    } else {
        printf("Test Send with End Frame: FAILED\n");
    }

    if (test_receive_end_frame()) {
        printf("Test Receive End Frame: PASSED\n");
    } else {
        printf("Test Receive End Frame: FAILED\n");
    }

    if (test_unexpected_end_frame()) {
        printf("Test Unexpected End Frame: PASSED\n");
    } else {
        printf("Test Unexpected End Frame: FAILED\n");
    }

    if (test_ctp_receive_data()) {
        printf("Test Receive Data PASSED.\n");
    } else {
        printf("Test Receive Data FAILED.\n");
    }

    return 0;
}