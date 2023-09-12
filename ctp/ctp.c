#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "ctp.h"


// Global State variables
bool waiting_for_flow_control = false; // Flag to indicate if we're waiting for a flow control frame
uint8_t expected_sequence_number = 0;  // Sequence number for the next expected consecutive frame

bool get_flow_control_state() {
    return waiting_for_flow_control;
}

void set_expected_sequence_number(uint8_t sequence_number) {
    expected_sequence_number = sequence_number;
}

uint8_t get_expected_sequence_number() {
    return expected_sequence_number;
}

void ctp_send_frame(const CTP_Frame *frame) {
    // Convert the CTP frame to raw CAN data
    uint8_t can_data[CAN_MAX_DATA_LENGTH];
    can_data[0] = frame->type;

    switch (frame->type) {
        case CTP_START_FRAME:
            can_data[1] = frame->payload.start.length;
            memcpy(&can_data[2], frame->payload.start.data, frame->payload.start.length);
            break;
        case CTP_CONSECUTIVE_FRAME:
            can_data[1] = frame->payload.consecutive.sequence;
            memcpy(&can_data[2], frame->payload.consecutive.data, CAN_MAX_DATA_LENGTH - 2);
            break;
        //case CTP_FLOW_CONTROL_FRAME:
        //    can_data[1] = frame->payload.flowControl.control;
        //    break;
        case CTP_END_FRAME:
            memcpy(&can_data[1], frame->payload.end.data, CAN_MAX_DATA_LENGTH - 1);
            break;
        default:
            break;
    }
    
    // Send using the mock driver function
    send_can_message(frame->id, can_data, CAN_MAX_DATA_LENGTH);
}

void ctp_process_frame(const CTP_Frame *frame) {
    // Process the received CTP frame
     switch (frame->type) {
        case CTP_START_FRAME:
            printf("Received START FRAME with data: ");
            for (int i = 0; i < frame->payload.start.length; i++) {
                printf("%02X ", frame->payload.start.data[i]);
            }
            printf("\n");
            break;
        case CTP_CONSECUTIVE_FRAME:
            printf("Received CONSECUTIVE FRAME with sequence %u and data: ", frame->payload.consecutive.sequence);
            for (int i = 0; i < CAN_MAX_DATA_LENGTH - 2; i++) {
                printf("%02X ", frame->payload.consecutive.data[i]);
            }
            printf("\n");
            break;
        case CTP_END_FRAME:
            printf("Received END FRAME with data: ");
            for (int i = 0; i < CAN_MAX_DATA_LENGTH - 1; i++) {
                printf("%02X ", frame->payload.end.data[i]);
            }
            printf("\n");
            break;
        case CTP_FLOW_CONTROL_FRAME:
            printf("Received FLOW CONTROL FRAME with control code: %u\n", frame->payload.flowControl.control);
            break;
        default:
            break;
    }
}

bool ctp_receive_frame(CTP_Frame *frame) {
    uint8_t can_data[CAN_MAX_DATA_LENGTH];
    uint8_t length;
    
    if (receive_can_message(&frame->id, can_data, &length)) {
        frame->type = can_data[0];
        switch (frame->type) {
            case CTP_START_FRAME:
                frame->payload.start.length = can_data[1];
                memcpy(frame->payload.start.data, &can_data[2], frame->payload.start.length);
                expected_sequence_number = 1; // Next expected sequence number
                break;
            
            case CTP_CONSECUTIVE_FRAME: 
                frame->payload.consecutive.sequence = can_data[1];
                if (frame->payload.consecutive.sequence == expected_sequence_number) {
                    expected_sequence_number++;  // Increment for the next expected frame
                    if (expected_sequence_number > MAX_SEQUENCE_NUMBER) { 
                        expected_sequence_number = 0;  // Wrap around
                    }

                    memcpy(frame->payload.consecutive.data, &can_data[2], CAN_MAX_DATA_LENGTH - 2);
                } else {
                    printf("[DEBUG] Received unexpected sequence number: %u\n", frame->payload.consecutive.sequence);
                }
                break;

            case CTP_END_FRAME:
                memcpy(frame->payload.end.data, &can_data[1], CAN_MAX_DATA_LENGTH - 1);
                break;

            //case CTP_FLOW_CONTROL_FRAME:
            //    frame->payload.flowControl.control = can_data[1];
            //    break;

            default:
                printf("[DEBUG] Received unknown frame type: %u\n", frame->type);
                break;
        }

        ctp_process_frame(frame);
        return true;
    }
    
    return false;
}

bool ctp_receive_data(uint32_t expected_id, uint8_t* buffer, uint32_t* received_length) {
    CTP_Frame frame;
    uint8_t expected_sequence_number = 0;
    *received_length = 0;

    while (ctp_receive_frame(&frame)) {
        if (frame.id == expected_id) {
            switch (frame.type) {
                case CTP_START_FRAME:
                    if (frame.payload.start.length > (MAX_BUFFER_SIZE - *received_length)) {
                        // Handle buffer overflow error
                        return false;
                    }
                    memcpy(&buffer[*received_length], frame.payload.start.data, frame.payload.start.length);
                    *received_length += frame.payload.start.length;
                    expected_sequence_number = 1; // Next expected sequence number
                    break;

                case CTP_CONSECUTIVE_FRAME:
                    if (frame.payload.consecutive.sequence != expected_sequence_number) {
                        // Handle out of order sequence error
                        return false;
                    }
                    if ((CAN_MAX_DATA_LENGTH - 1) > (MAX_BUFFER_SIZE - *received_length)) {
                        // Handle buffer overflow error
                        return false;
                    }
                    memcpy(&buffer[*received_length], frame.payload.consecutive.data, CAN_MAX_DATA_LENGTH - 1);
                    *received_length += CAN_MAX_DATA_LENGTH - 2;
                    expected_sequence_number++;
                    break;

                case CTP_END_FRAME:
                    if (CAN_MAX_DATA_LENGTH > (MAX_BUFFER_SIZE - *received_length)) {
                        // Handle buffer overflow error
                        return false;
                    }
                    memcpy(&buffer[*received_length], frame.payload.end.data, CAN_MAX_DATA_LENGTH);
                    *received_length += CAN_MAX_DATA_LENGTH - 1;
                    return true; // Completed reception

                default:
                    // Unexpected frame type
                    return false;
            }
        }
    }

    return false; // Failed to receive the complete data or an error occurred
}

void ctp_send(uint32_t id, const uint8_t *data, uint8_t length) {
    CTP_Frame frame;
    frame.id = id;
    
    uint8_t start_frame_length = (length > (CAN_MAX_DATA_LENGTH - 2)) ? (CAN_MAX_DATA_LENGTH - 2) : length;

    // Set up and send the START frame
    frame.type = CTP_START_FRAME;
    frame.payload.start.length = start_frame_length;
    memcpy(frame.payload.start.data, data, start_frame_length);
    ctp_send_frame(&frame);
    //waiting_for_flow_control = true;

    uint32_t bytes_sent = start_frame_length;
    uint8_t sequence_number = 0;
    //int timeout_counter = 1000;  // Arbitrary number, represents the max number of loops we wait for flow control

    while (bytes_sent < length) {
        //while (waiting_for_flow_control && timeout_counter > 0) {
        //    if (ctp_receive_frame(&frame) && frame.type == CTP_FLOW_CONTROL_FRAME && frame.id == id) {
        //        if (frame.payload.flowControl.control == CTP_CONTINUE_SENDING) {
        //            waiting_for_flow_control = false;
        //        } else if (frame.payload.flowControl.control == CTP_WAIT) {
        //            continue;
        //        } else if (frame.payload.flowControl.control == CTP_ABORT) {
        //            return;  // Abort the transmission
        //        }
        //    }
        //    timeout_counter--;
        //}

        //if (timeout_counter == 0) {
        //    // Handle timeout scenario
        //    return;
        //}

        uint8_t bytes_left = length - bytes_sent;

        if (bytes_left <= (CAN_MAX_DATA_LENGTH - 1)) {
            frame.type = CTP_END_FRAME;
            memcpy(frame.payload.end.data, data + bytes_sent, bytes_left);
        } else {
            frame.type = CTP_CONSECUTIVE_FRAME;
            frame.payload.consecutive.sequence = sequence_number++;
            memcpy(frame.payload.consecutive.data, data + bytes_sent, CAN_MAX_DATA_LENGTH - 1);
            bytes_left = CAN_MAX_DATA_LENGTH - 1;
        }
        
        ctp_send_frame(&frame);
        bytes_sent += bytes_left;
        //waiting_for_flow_control = true;  // Wait for flow control again after sending a frame
        //timeout_counter = 1000;  // Reset the timeout counter for the next wait
    }

    //waiting_for_flow_control = false;
}

