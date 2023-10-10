# CAN Transport Protocol (CTP)

## Overview

The CAN Transport Protocol (CTP) is a communication protocol designed to operate over a CAN bus. CTP provides mechanisms to send larger data payloads across multiple frames, manage flow control, and handle errors.

```plaintext
+-------------------------------------------------+  
|                     CTP_Frame                   |  
+-------------------+-----------------------------+  
|        id         |             type            |  
|    (32 bits)      |          (8 bits)           |  
+-------------------+-----------------------------+  
|                    payload                      |  
|     +-------------------------------------+     |  
|     |             start payload           |     |  
|     | +--------------+------------------+ |     |
|     | | payload_len  |       data       | |     |
|     | | (16 bits)    | (61 bytes)       | |     |
|     | +--------------+------------------+ |     |
|     +-------------------------------------+     |
|     +-------------------------------------+     |
|     |        consecutive payload          |     |
|     | +--------------+------------------+ |     |
|     | |  sequence    |       data       | |     |
|     | |  (8 bits)    | (62 bytes)       | |     |
|     | +--------------+------------------+ |     |
|     +-------------------------------------+     |
|     +-------------------------------------+     |
|     |            end payload              |     |
|     | +------------------+                |     |
|     | |       data       |                |     |
|     | | (63 bytes)       |                |     |
|     | +------------------+                |     |
|     +-------------------------------------+     |
|     +-------------------------------------+     |
|     |           error payload             |     |
|     | +--------------+------------------+ |     |
|     | |  errorCode   |       data       | |     |
|     | | (8 bits)     | (63 bytes)       | |     |
|     | +--------------+------------------+ |     |
|     +-------------------------------------+     |
+-------------------------------------------------+
```

## Features

- **Multi-frame Transmission**: CTP can transmit data that spans multiple frames, including start frames, consecutive frames, and end frames.

- **Error Handling**: The protocol provides mechanisms to handle errors like OUT_OF_ORDER.
- **CAN FD Support**

## Frame Types

1. **START_FRAME**: Initiates a multi-frame transmission. Contains the length of data for the entire message up to `0xFFFF` bytes.
2. **CONSECUTIVE_FRAME**: Part of a multi-frame sequence that follows a START_FRAME. Contains a sequence number to ensure data is transmitted in order.
3. **END_FRAME**: Indicates the end of a multi-frame transmission.
4. **ERROR_FRAME**: Used to transmit error codes.

## Usage

### Sending Data

To send a sequence of data:

```c
uint32_t id = 456;
uint8_t data[] = {
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99
};

uint32_t bytes_sent = ctp_send(id, data, sizeof(data), false);
```
for FD support set the FD flag = true

```c
uint32_t bytes_sent = ctp_send(id, data, sizeof(data), true);
```

### Receiving Data

The `ctp_receive` function is used to receive a frame:

```c
uint32_t id = 456;
uint8_t received_data[512];

int32_t data_len = ctp_receive(received_data, sizeof(received_data), false);
```

for FD support set the FD flag = true

```c
int32_t data_len = ctp_receive(received_data, sizeof(received_data), true);
```

## CLI

The command line interface supports `PCAN` hardware

```c
$ make cli
```

```plaintext
Usage: cli [OPTIONS] COMMAND [ARGS]

Options:
  -i, --interface <num>   Specify the interface number.
  -h, --help              Show this help message and exit.

Commands:
  send                    Send a command.
    --id <num>            Specify the ID for the send command.
    --data <string>       Specify the data for the send command.
  dump                    Dump the data.

Args for both send and dump:
    --baud <num>          Specify the baud rate.

Examples:
  cli -i 0 send --id 123 --data "hello" --baud 250
  cli -i 1 dump --baud 250
```

**TODO**
* FD Support on MAC
* File support

### Error Handling

If an error occurs, an ERROR_FRAME is sent with the appropriate error code.


## Testing

The codebase includes various test functions to validate the functionality of the protocol. These tests check sending, receiving, processing, and error scenarios.

```c
$ make test
```

## License

MIT License

