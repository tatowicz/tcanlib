# CAN Transport Protocol (CTP)

## Overview

The CAN Transport Protocol (CTP) is a communication protocol designed to operate over a CAN bus. CTP provides mechanisms to send larger data payloads across multiple frames, manage flow control, and handle errors.

## Features

- **Multi-frame Transmission**: CTP can transmit data that spans multiple frames, including start frames, consecutive frames, and end frames.

- **Error Handling**: The protocol provides mechanisms to handle errors like MESSAGE_TIMEOUT and OUT_OF_ORDER.

## Frame Types

1. **START_FRAME**: Initiates a multi-frame transmission. Contains the length of data in this frame.
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

ctp_send(id, data, sizeof(data));
```

### Receiving Data

The `ctp_receive` function is used to receive a frame:

```c
uint32_t id = 456;
uint8_t received_data[512];

uint32_t data_len = ctp_receive(received_data, sizeof(received_data));
```

### CLI

The command line interface supports `PCAN` hardware

```c
$ make cli
$ ./cli send 123 "Hello World"
```

### Error Handling

If an error occurs, an ERROR_FRAME is sent with the appropriate error code.


## Testing

The codebase includes various test functions to validate the functionality of the protocol. These tests check sending, receiving, processing, and error scenarios.

```c
$ make test
```

## TODO

1. Support FD frames

## License

MIT License

