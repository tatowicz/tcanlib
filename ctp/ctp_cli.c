#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "ctp.h"  
#include "PCBUSB.h"



#define PCAN_CHANNEL PCAN_USBBUS1


bool send_ctp_message(uint32_t id, const uint8_t *data, uint8_t length) {
    TPCANStatus status;
    TPCANMsg message;

    message.ID = id;
    message.LEN = length;
    message.MSGTYPE = PCAN_MESSAGE_STANDARD;
    memcpy(message.DATA, data, message.LEN);

    status = CAN_Write(PCAN_CHANNEL, &message);

    if (status != PCAN_ERROR_OK) {
        printf("Failed to send CAN message: 0x%x\n", status);
        return false;
    }

    return true;
}

bool receive_ctp_message(uint32_t *id, uint8_t *data, uint8_t *length) {
    TPCANMsg message;
    TPCANStatus status;

    status = CAN_Read(PCAN_CHANNEL, &message, NULL);

    if (status == PCAN_ERROR_OK) {
        if (!(message.MSGTYPE & PCAN_MESSAGE_STATUS)) {
            printf("  - R ID:%4x LEN:%1x DATA:%02x %02x %02x %02x %02x %02x %02x %02x\n",
                (int) message.ID, (int) message.LEN, (int) message.DATA[0],
                (int) message.DATA[1], (int) message.DATA[2],
                (int) message.DATA[3], (int) message.DATA[4],
                (int) message.DATA[5], (int) message.DATA[6],
                (int) message.DATA[7]);
        }
    } 
    else if (status == PCAN_ERROR_QRCVEMPTY) {
        printf("Receive queue is empty.\n");
        return false;
    } 
    else {
        printf("Error 0x%x\n", status);
        return false;
    }

    return true;
}

void send_data(uint32_t id, const char* data) {
    if (ctp_send(id, (uint8_t *)data, strlen(data)) <= 0) {
        printf("Failed to send data!\n");
    } else {
        printf("Data sent successfully.\n");
    }
}

void receive_data() {
    uint8_t buffer[512];  // Adjust the buffer size as needed.
    int32_t received_bytes = ctp_receive(buffer, sizeof(buffer));
    
    if (received_bytes == -1) {
        printf("Error receiving data.\n");
    } else {
        printf("Received data: %.*s\n", received_bytes, buffer);
    }
}

int main(int argc, char *argv[]) {
    TPCANStatus status;

    if (argc < 2) {
        printf("Send data over CAN using CTP protocol.\n");
        printf("Usage: %s <send> <CAN_ID> [data]\n", argv[0]);
        printf("Usage: %s <dump>\n", argv[0]);
        printf("Example: %s send 0x123 \"Hello World!\"\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "send") == 0) {
        if (argc < 4) {
            printf("Please provide CAN ID and data to send.\n");
            return 1;
        }

        status = CAN_Initialize(PCAN_CHANNEL, PCAN_BAUD_250K, 0, 0, 0);
        printf("Initialize CAN, Status = 0x%x\n", status);
        if (status != PCAN_ERROR_OK) {
            printf("Failed to initialize CAN\n");
            return 1;
        }

        uint32_t can_id = (uint32_t)strtoul(argv[2], NULL, 0);
        send_data(can_id, argv[3]);
    } else if (strcmp(argv[1], "dump") == 0) {
        receive_data();
    } else {
        printf("Invalid command. Use 'send' or 'dump'.\n");
    }

    return 0;
}
