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
bool send_ctp_message(uint32_t id, const uint8_t *data, uint8_t length) {
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

void send_data(uint32_t id, const char* data) {
    if (ctp_send(id, (uint8_t *)data, strlen(data), false) <= 0) {
        printf("Failed to send data!\n");
    } else {
        printf("Data sent successfully.\n");
    }
}

void receive_data() {
    uint8_t buffer[512];  // Adjust the buffer size as needed.

    while (1) {
        int32_t received_bytes = ctp_receive(buffer, sizeof(buffer), false);
        
        if (received_bytes > 0) {
            printf("Received data: %.*s\n", received_bytes, buffer);
            
            for (uint32_t i = 0; i < received_bytes; i++) {
                printf("%02x ", buffer[i]);
            }
            printf("\n");
        } 
    }
}

uint32_t init_can(uint32_t channel, uint32_t baud_rate) {
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

void print_help() {
    printf("Usage: cli [OPTIONS] COMMAND [ARGS]\n\n");

    printf("Options:\n");
    printf("  -i, --interface <num>   Specify the interface number.\n");
    printf("  -h, --help              Show this help message and exit.\n\n");

    printf("Commands:\n");
    printf("  send                    Send a command.\n");
    printf("    --id <num>            Specify the ID for the send command.\n");
    printf("    --data <string>       Specify the data for the send command.\n");
    printf("  dump                    Dump the data.\n\n");

    printf("Args for both send and dump:\n");
    printf("    --baud <num>          Specify the baud rate.\n\n");

    printf("Examples:\n");
    printf("  cli -i 1 send --id 123 --data \"hello\" --baud 250\n");
    printf("  cli -i 1 dump --baud 250\n");
}

int main(int argc, char *argv[]) {
    int interface = -1;
    int baud_rate = -1;
    int id = -1;
    char *data = NULL;

    if (argc < 2) {
        print_help();
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help();
            return 0;
        } 
        else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--interface") == 0) {
            if (++i < argc) {
                interface = atoi(argv[i]);
            }
        } 
        else if (strcmp(argv[i], "send") == 0) {
            i++;
            while (i < argc) {
                if (strcmp(argv[i], "--id") == 0) {
                    if (++i < argc) {
                        id = atoi(argv[i]);
                    }
                } else if (strcmp(argv[i], "--data") == 0) {
                    if (++i < argc) {
                        data = argv[i];
                    }
                } else if (strcmp(argv[i], "--baud") == 0) {
                    if (++i < argc) {
                        baud_rate = atoi(argv[i]);
                    }
                } else {
                    break;
                }
                i++;
            }
            if (id != -1 && data != NULL) {
                uint32_t status = init_can(interface, baud_rate);

                if (status != 0) {
                    printf("Failed to initialize CAN\n");
                    return 1;
                }

                printf("Send command with id=%d, data=%s, baud_rate=%dk\n", id, data, baud_rate);
                send_data(id, data);
            } 
            else {
                printf("Send command is missing required arguments.\n");
            }
        } 
        else if (strcmp(argv[i], "dump") == 0) {
            if (++i < argc && (strcmp(argv[i], "--baud") == 0)) {
                if (++i < argc) {
                    baud_rate = atoi(argv[i]);
                }
            }

            uint32_t status = init_can(interface, baud_rate);

            if (status != 0) {
                printf("Failed to initialize CAN\n");
                return 1;
            }

            printf("Dump command with baud_rate=%dk\n", baud_rate);   
            receive_data();
        }
    }

    return 0;
}
