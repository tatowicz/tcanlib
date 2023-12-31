#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "ctp.h"  
#include "PCBUSB.h"

// Global pcan channel for processes to use
uint32_t pcan_channel = PCAN_USBBUS1;


// Driver function for PCAN-USB MAC, used by the CTP to send frames to the CAN bus
bool send_ctp_message(uint32_t id, uint8_t *data, uint8_t length) {
    TPCANStatus status;
    TPCANMsg message;

    message.ID = id;
    message.LEN = length;
    message.MSGTYPE = PCAN_MESSAGE_STANDARD;
    memcpy(message.DATA, data, message.LEN);

    status = CAN_Write(pcan_channel, &message);

    if (status != PCAN_ERROR_OK) {
        printf("Failed to send CAN message: 0x%x\n", status);
        return false;
    }

    return true;
}

// Driver function for PCAN-USB MAC, used by the CTP to get frames from the CAN bus
bool receive_ctp_message(uint32_t *id, uint8_t *data, uint8_t *length) {
    TPCANMsg message;
    TPCANStatus status;

    status = CAN_Read(pcan_channel, &message, NULL);

    if (status == PCAN_ERROR_OK) {        
        *id = message.ID;
        *length = message.LEN;
        memcpy(data, message.DATA, message.LEN);
    } 
    else if (status == PCAN_ERROR_QRCVEMPTY) {
        return false;
    } 
    else {
        return false;
    }

    return true;
}

uint32_t init_pcan(uint32_t channel, uint32_t baud_rate) {
    uint32_t status = false;

    uint32_t channel_map[] = {
        PCAN_USBBUS1,
        PCAN_USBBUS2,
        PCAN_USBBUS3,
        PCAN_USBBUS4,
        PCAN_USBBUS5,
        PCAN_USBBUS6,
        PCAN_USBBUS7,
        PCAN_USBBUS8
    };

    if (channel > 7) {
        printf("Invalid channel: %d\n", channel);
        return 1;
    }

    switch(baud_rate) {
        case 1000:
            baud_rate = PCAN_BAUD_1M;
            break;

        case 500:
            baud_rate = PCAN_BAUD_500K;
            break;

        case 250:
            baud_rate = PCAN_BAUD_250K;
            break;

        default:
            printf("Invalid baud rate: %d\n", baud_rate);
            return 1;
    }

    pcan_channel = channel_map[channel];

    status = CAN_Initialize(pcan_channel, baud_rate, 0, 0, 0);
    printf("Initialize CAN, Status = 0x%x\n", status);

    if (status != PCAN_ERROR_OK) {
            printf("Failed to initialize CAN\n");
            return 1;
    }

    return 0;
}