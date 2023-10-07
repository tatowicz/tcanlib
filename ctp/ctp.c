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
            can_data[1] = (uint8_t)(frame->payload.start.payload_len >> 8);
            can_data[2] = (uint8_t)(frame->payload.start.payload_len & 0xFF);
            memcpy(&can_data[3], &frame->payload.start.data, len);
            length = len + 3; 
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

// RX up to 255 consecutive frames.
int32_t ctp_receive(uint8_t* buffer, uint32_t buffer_size, bool fd) {
    uint8_t can_data[CAN_MAX_DATA_LENGTH] = {0};
    uint8_t length;
    uint32_t received_length = 0;
    uint8_t expected_sequence_number = 0;
    uint32_t expected_total_length = 0;
    bool start_frame_received = false;
    uint32_t can_id;
    uint8_t frame_type;

    uint8_t start_data_size;
    uint8_t con_data_size;

    if (fd) {
        start_data_size = CTP_FD_START_DATA_SIZE;
        con_data_size = CTP_FD_CONSECUTIVE_DATA_LENGTH;
    }
    else {
        start_data_size = CTP_START_DATA_SIZE;
        con_data_size = CTP_CONSECUTIVE_DATA_LENGTH;
    }

    while (received_length < expected_total_length || !start_frame_received) {
        if (!receive_ctp_message(&can_id, can_data, &length)) {
            continue;  // Keep trying until we get a message or reach timeout
        }

        frame_type = can_data[0];

        if (!start_frame_received && frame_type == CTP_START_FRAME) {
            expected_total_length = (can_data[1] << 8) | can_data[2];

            if (expected_total_length > buffer_size) {
                printf("Buffer provided is not enough: expected_total_length=%u, buffer_size=%u\n", 
                        expected_total_length, buffer_size);
                return -1;  // Error: buffer provided is not enough
            }

            uint8_t start_frame_length = (expected_total_length > start_data_size) ? start_data_size : expected_total_length;

            memcpy(buffer, &can_data[3], start_frame_length);
            received_length += start_frame_length;
            start_frame_received = true;

            if (expected_total_length == start_frame_length) {
                return received_length;
            }
            continue;
        }

        if (start_frame_received) {
            uint32_t bytes_left = expected_total_length - received_length;
            
            switch (frame_type) {
                case CTP_CONSECUTIVE_FRAME:
                    if (can_data[1] != expected_sequence_number) {
                        printf("Expected sequence number %u but received %u\n", expected_sequence_number, can_data[1]);
                        return -1;  // Error: sequence mismatch
                    }
                    if ((received_length + con_data_size) > buffer_size) {
                        printf("Buffer overflow: received_length=%u, buffer_size=%u\n", received_length, buffer_size);
                        return -1;  // Error: buffer overflow
                    }
                    memcpy(&buffer[received_length], &can_data[2], con_data_size);
                    received_length += con_data_size;
                    expected_sequence_number++;
                    break;

                case CTP_END_FRAME:
                    if ((received_length + bytes_left) > buffer_size) {
                        printf("Buffer overflow: received_length=%u, buffer_size=%u\n", received_length, buffer_size);
                        return -1;  // Error: buffer overflow
                    }
                    memcpy(&buffer[received_length], &can_data[1], bytes_left);
                    received_length += bytes_left;
                    return received_length;  // Successfully received the full frame

                default:
                    // In case of unexpected frame type, keep trying
                    break;
            }
        }
    }

    return -1;  // Should not reach here unless there's an error
}

// Receive length bytes, and store it in the buffer.
int32_t ctp_receive_bytes(uint8_t *buffer, uint32_t length, bool fd) {
    uint32_t bytes_received = 0;
    
    while (bytes_received < length) {
         bytes_received += ctp_receive(buffer + bytes_received, length - bytes_received, fd);

         if (bytes_received < 0) {
             return -1;
         }
    }

    return bytes_received;
}

uint32_t ctp_send_data_sequence(uint32_t id, uint8_t *data, uint16_t length, bool fd) {
    CTP_Frame frame;
    frame.id = id;
    uint8_t start_data_size;
    uint8_t end_data_size;
    uint8_t con_data_size;

    if (fd) {
        start_data_size = CTP_FD_START_DATA_SIZE;
        end_data_size = CTP_FD_END_DATA_LENGTH;
        con_data_size = CTP_FD_CONSECUTIVE_DATA_LENGTH;
    }
    else {
        start_data_size = CTP_START_DATA_SIZE;
        end_data_size = CTP_END_DATA_LENGTH;
        con_data_size = CTP_CONSECUTIVE_DATA_LENGTH;
    }
    
    uint16_t start_frame_length = (length > (start_data_size)) ? (start_data_size) : length;

    // Set up and send the START frame
    frame.type = CTP_START_FRAME;
    frame.payload.start.payload_len = length;
    memcpy(frame.payload.start.data, data, start_frame_length);
    ctp_send_frame(&frame, (uint8_t)start_frame_length);

    uint32_t bytes_sent = start_frame_length;
    uint8_t sequence_number = 0;

    while (bytes_sent < length) {
        uint32_t bytes_left = length - bytes_sent;

        if (bytes_left <= (end_data_size)) {
            frame.type = CTP_END_FRAME;
            memcpy(frame.payload.end.data, data + bytes_sent, bytes_left);
        } else {
            frame.type = CTP_CONSECUTIVE_FRAME;
            frame.payload.consecutive.sequence = sequence_number++;
            memcpy(frame.payload.consecutive.data, data + bytes_sent, con_data_size);
            bytes_left = con_data_size;
        }
        
        ctp_send_frame(&frame, bytes_left);
        bytes_sent += bytes_left;
    }

    return bytes_sent;
}

uint32_t ctp_send(uint32_t id, uint8_t *data, uint32_t length, bool fd) {
    uint32_t bytes_sent = 0;
    uint32_t max_len;

    if (fd) {
        max_len = CTP_FD_START_DATA_SIZE + MAX_SEQUENCE_NUM * CTP_FD_CONSECUTIVE_DATA_LENGTH + CTP_FD_END_DATA_LENGTH;
    } 
    else {
        max_len = CTP_START_DATA_SIZE + MAX_SEQUENCE_NUM * CTP_CONSECUTIVE_DATA_LENGTH + CTP_END_DATA_LENGTH; 
    }

    while (length > 0) {
        uint16_t chunk_length = (length > (max_len)) ? (max_len) : length;
        bytes_sent += ctp_send_data_sequence(id, data, chunk_length, fd);
        data += chunk_length;
        length -= chunk_length;
    }

    return bytes_sent;
}
