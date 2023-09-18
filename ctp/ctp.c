#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "ctp.h"



void ctp_send_frame(const CTP_Frame *frame) {
    // Convert the CTP frame to raw CAN data
    uint8_t can_data[CAN_MAX_DATA_LENGTH] = {0};
    uint8_t length = 0;
    can_data[0] = frame->type;

    switch (frame->type) {
        case CTP_START_FRAME:
            can_data[1] = frame->payload.start.payload_len;
            memcpy(&can_data[2], frame->payload.start.data, frame->payload.start.frame_len);
            length = frame->payload.start.frame_len + 2; 
            break;
        case CTP_CONSECUTIVE_FRAME:
            can_data[1] = frame->payload.consecutive.sequence;
            memcpy(&can_data[2], frame->payload.consecutive.data, CTP_CONSECUTIVE_DATA_LENGTH);
            length = CTP_CONSECUTIVE_DATA_LENGTH + 2;
            break;
        case CTP_END_FRAME:
            memcpy(&can_data[1], frame->payload.end.data, frame->payload.end.bytes_left);
            length = frame->payload.end.bytes_left + 1;
            break;
        case CTP_ERROR_FRAME:
            can_data[1] = frame->payload.error.errorCode;
            length = 2;
            break;
        default:
            break;
    }
    
    send_can_message(frame->id, can_data, length);
}

void ctp_process_frame(const CTP_Frame *frame) {
    // Process the received CTP frame
     switch (frame->type) {
        case CTP_START_FRAME:
            printf("Received START FRAME with length %u and data: ", frame->payload.start.payload_len);
            for (int i = 0; i < frame->payload.start.frame_len; i++) {
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
            for (int i = 0; i < frame->payload.end.bytes_left; i++) {
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

bool ctp_receive_frame(uint8_t expected_sequence_number, CTP_Frame *frame) {
    uint8_t can_data[CAN_MAX_DATA_LENGTH];
    uint8_t length;
    uint8_t start_frame_length;
    
    if (receive_can_message(&frame->id, can_data, &length)) {
        frame->type = can_data[0];
        switch (frame->type) {
            case CTP_START_FRAME:
                start_frame_length = (can_data[1] > (CTP_START_DATA_SIZE)) ? (CTP_START_DATA_SIZE) : can_data[1];
                frame->payload.start.payload_len = can_data[1];
                frame->payload.start.frame_len = start_frame_length;
                memcpy(frame->payload.start.data, &can_data[2], start_frame_length);
                expected_sequence_number = 1; // Next expected sequence number
                break;
            
            case CTP_CONSECUTIVE_FRAME: 
                frame->payload.consecutive.sequence = can_data[1];
                if (frame->payload.consecutive.sequence == expected_sequence_number) {
                    expected_sequence_number++;  // Increment for the next expected frame
                    if (expected_sequence_number > MAX_SEQUENCE_NUMBER) { 
                        expected_sequence_number = 0;  // Wrap around
                    }

                    memcpy(frame->payload.consecutive.data, &can_data[2], CTP_CONSECUTIVE_DATA_LENGTH);
                } else {
                    printf("[DEBUG] Received unexpected sequence number: %u\n", frame->payload.consecutive.sequence);
                    return false;
                }
                break;

            case CTP_END_FRAME:
                memcpy(frame->payload.end.data, &can_data[1], frame->payload.end.bytes_left);
                break;

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

    while (ctp_receive_frame(expected_sequence_number, &frame)) {
        if (frame.id == expected_id) {
            switch (frame.type) {
                case CTP_START_FRAME:
                    if (frame.payload.start.payload_len > (MAX_BUFFER_SIZE - *received_length)) {
                        // Handle buffer overflow error
                        return false;
                    }
                    memcpy(&buffer[*received_length], frame.payload.start.data, frame.payload.start.frame_len);
                    *received_length += frame.payload.start.frame_len;
                    expected_sequence_number = 1; // Next expected sequence number
                    break;

                case CTP_CONSECUTIVE_FRAME:
                    if (frame.payload.consecutive.sequence != expected_sequence_number) {
                        // Handle out of order sequence error
                        return false;
                    }
                    if ((CTP_CONSECUTIVE_DATA_LENGTH) > (MAX_BUFFER_SIZE - *received_length)) {
                        // Handle buffer overflow error
                        return false;
                    }
                    memcpy(&buffer[*received_length], frame.payload.consecutive.data, CTP_CONSECUTIVE_DATA_LENGTH);
                    *received_length += CTP_CONSECUTIVE_DATA_LENGTH;
                    expected_sequence_number++;
                    break;

                case CTP_END_FRAME:
                    if (CAN_MAX_DATA_LENGTH > (MAX_BUFFER_SIZE - *received_length)) {
                        // Handle buffer overflow error
                        return false;
                    }
                    memcpy(&buffer[*received_length], frame.payload.end.data, frame.payload.end.bytes_left);
                    *received_length += frame.payload.end.bytes_left;
                    return true; // Completed reception

                default:
                    // Unexpected frame type
                    return false;
            }
        }
    }

    return false;
}

uint32_t ctp_send_data_sequence(uint32_t id, uint8_t *data, uint8_t length) {
    CTP_Frame frame;
    frame.id = id;
    
    uint8_t start_frame_length = (length > (CTP_START_DATA_SIZE)) ? (CTP_START_DATA_SIZE) : length;

    // Set up and send the START frame
    frame.type = CTP_START_FRAME;
    frame.payload.start.frame_len = start_frame_length;
    frame.payload.start.payload_len = length;
    memcpy(frame.payload.start.data, data, start_frame_length);
    ctp_send_frame(&frame);

    uint32_t bytes_sent = start_frame_length;
    uint8_t sequence_number = 0;

    while (bytes_sent < length) {
        uint8_t bytes_left = length - bytes_sent;

        if (bytes_left <= (CTP_END_DATA_LENGTH)) {
            frame.type = CTP_END_FRAME;
            frame.payload.end.bytes_left = bytes_left;
            memcpy(frame.payload.end.data, data + bytes_sent, bytes_left);
        } else {
            frame.type = CTP_CONSECUTIVE_FRAME;
            frame.payload.consecutive.sequence = sequence_number++;
            memcpy(frame.payload.consecutive.data, data + bytes_sent, CTP_CONSECUTIVE_DATA_LENGTH);
            bytes_left = CTP_CONSECUTIVE_DATA_LENGTH;
        }
        
        ctp_send_frame(&frame);
        bytes_sent += bytes_left;
    }

    return bytes_sent;
}

uint32_t ctp_send(uint32_t id, uint8_t *data, uint32_t length) {
    uint32_t bytes_sent = 0;
    while (length > 0) {
        uint16_t chunk_length = (length > MAX_SEQUENCE_NUMBER) ? MAX_SEQUENCE_NUMBER : length;
        bytes_sent += ctp_send_data_sequence(id, data, chunk_length);
        data += chunk_length;
        length -= chunk_length;
    }

    return bytes_sent;
}
