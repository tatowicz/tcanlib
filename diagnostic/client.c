#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ctp.h"


#define CTP_ID 0x123

void send_command(uint32_t id, const char *command, const char *args) {
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "%s;%s", command, args);
    ctp_send(id, buffer, strlen(buffer), false);
}

void send_query(char *response, char *query) {
    uint32_t received_length;

    // Send a query
    send_command(CTP_ID, "QUERY", query);
    received_length = ctp_receive(response, sizeof(response) - 1, false);

    if (received_length >= 0) {
        response[received_length] = '\0';
        printf("Received: %s\n", response);
    }
}

int main() {
    char response[1024];
    uint32_t received_length;


    // Request a download
    send_command(CTP_ID, "QUERY", "SET myval val");
    received_length = ctp_receive(response, sizeof(response) - 1, false);

    if (received_length >= 0) {
        response[received_length] = '\0';
        printf("Received: %s\n", response);
    }

    return 0;
}
