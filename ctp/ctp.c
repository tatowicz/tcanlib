#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "ctp.h"



void ctp_send_frame(const CTP_Frame *frame, uint8_t len) {
    // Convert the CTP frame to raw CAN data
    uint8_t can_data[CAN_MAX_DATA_LENGTH] = {0};
    uint8_t length = 0;
    can_data[0] = frame->type;

    switch (frame->type) {
        case CTP_START_FRAME:
            can_data[1] = frame->payload.start.payload_len;
            memcpy(&can_data[2], frame->payload.start.data, len);
            length = len + 2; 
            break;
        case CTP_CONSECUTIVE_FRAME:
            can_data[1] = frame->payload.consecutive.sequence;
            memcpy(&can_data[2], frame->payload.consecutive.data, len);
            length = len + 2;
            break;
        case CTP_END_FRAME:
            memcpy(&can_data[1], frame->payload.end.data, len);
            length = len + 1;
            break;
        case CTP_ERROR_FRAME:
            can_data[1] = frame->payload.error.errorCode;
            length = 2;
            break;
        default:
            break;
    }
    
    send_ctp_message(frame->id, can_data, length);
}

int32_t ctp_receive(uint8_t* buffer, uint32_t buffer_size) {
    uint8_t can_data[CAN_MAX_DATA_LENGTH] = {0};
    uint8_t length;
    uint32_t received_length = 0;
    uint8_t expected_sequence_number = 0;
    uint32_t expected_total_length = 0;
    bool start_frame_received = false;
    uint32_t can_id;  

    //const uint32_t MAX_RETRIES = 1000;
    //uint32_t retry_count = 0;

    while (received_length < expected_total_length || !start_frame_received) {
        if (!receive_ctp_message(&can_id, can_data, &length)) {
            //retry_count++;
            // if (retry_count >= MAX_RETRIES) {
            //      return -1;  // Return error if max retry count is reached
            // }
            continue;  // Keep trying until we get a message or reach max retries
        }

        // Reset retry count on successful reception
        //retry_count = 0;

        if (!start_frame_received && can_data[0] == CTP_START_FRAME) {
            expected_total_length = can_data[1];

            if (expected_total_length > buffer_size) {
                printf("Buffer provided is not enough: expected_total_length=%u, buffer_size=%u\n", expected_total_length, buffer_size);
                return -1;  // Error: buffer provided is not enough
            }

            uint8_t start_frame_length = (can_data[1] > CTP_START_DATA_SIZE) ? CTP_START_DATA_SIZE : can_data[1];
            memcpy(buffer, &can_data[2], start_frame_length);
            received_length += start_frame_length;
            expected_sequence_number = 1;
            start_frame_received = true;

            if (expected_total_length == start_frame_length) {
                return received_length;
            }
            
            continue;
        }

        if (start_frame_received) {
            switch (can_data[0]) {
                case CTP_CONSECUTIVE_FRAME:
                    if (can_data[1] != expected_sequence_number) {
                        printf("Expected sequence number %u but received %u\n", expected_sequence_number, can_data[1]);
                        return -1;  // Error: sequence mismatch
                    }
                    if ((received_length + CTP_CONSECUTIVE_DATA_LENGTH) > buffer_size) {
                        printf("Buffer overflow: received_length=%u, buffer_size=%u\n", received_length, buffer_size);
                        return -1;  // Error: buffer overflow
                    }
                    memcpy(&buffer[received_length], &can_data[2], CTP_CONSECUTIVE_DATA_LENGTH);
                    received_length += CTP_CONSECUTIVE_DATA_LENGTH;
                    expected_sequence_number++;
                    break;

                case CTP_END_FRAME:
                    if ((received_length + CTP_END_DATA_LENGTH) > buffer_size) {
                        printf("Buffer overflow: received_length=%u, buffer_size=%u\n", received_length, buffer_size);
                        return -1;  // Error: buffer overflow
                    }
                    memcpy(&buffer[received_length], &can_data[1], CTP_END_DATA_LENGTH);
                    received_length += CTP_END_DATA_LENGTH;
                    return received_length;  // Successfully received the full frame

                default:
                    // In case of unexpected frame type, keep trying
                    break;
            }
        }
    }

    return -1;  // Should not reach here unless there's an error
}


uint32_t ctp_send_data_sequence(uint32_t id, uint8_t *data, uint8_t length) {
    CTP_Frame frame;
    frame.id = id;
    
    uint8_t start_frame_length = (length > (CTP_START_DATA_SIZE)) ? (CTP_START_DATA_SIZE) : length;

    // Set up and send the START frame
    frame.type = CTP_START_FRAME;
    frame.payload.start.payload_len = length;
    memcpy(frame.payload.start.data, data, start_frame_length);
    ctp_send_frame(&frame, start_frame_length);

    uint32_t bytes_sent = start_frame_length;
    uint8_t sequence_number = 0;

    while (bytes_sent < length) {
        uint8_t bytes_left = length - bytes_sent;

        if (bytes_left <= (CTP_END_DATA_LENGTH)) {
            frame.type = CTP_END_FRAME;
            memcpy(frame.payload.end.data, data + bytes_sent, bytes_left);
        } else {
            frame.type = CTP_CONSECUTIVE_FRAME;
            frame.payload.consecutive.sequence = sequence_number++;
            memcpy(frame.payload.consecutive.data, data + bytes_sent, CTP_CONSECUTIVE_DATA_LENGTH);
            bytes_left = CTP_CONSECUTIVE_DATA_LENGTH;
        }
        
        ctp_send_frame(&frame, bytes_left);
        bytes_sent += bytes_left;
    }

    return bytes_sent;
}

uint32_t ctp_send(uint32_t id, uint8_t *data, uint32_t length) {
    uint32_t bytes_sent = 0;
    while (length > 0) {
        uint16_t chunk_length = (length > (MAX_SEQUENCE_LEN)) ? (MAX_SEQUENCE_LEN) : length;
        bytes_sent += ctp_send_data_sequence(id, data, chunk_length);
        data += chunk_length;
        length -= chunk_length;
    }

    return bytes_sent;
}
