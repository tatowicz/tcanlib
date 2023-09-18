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
uint8_t data[] = {0xAA, 0xBB, 0xCC, 0xDD};
ctp_send(id, data, sizeof(data));
```

### Receiving Data

The `ctp_receive_frame` function is used to receive a frame:

```c
uint32_t id = 456;
uint8_t received_data[MAX_BUFFER_SIZE];
uint32_t received_length = 0;

bool success = ctp_receive_data(id, received_data, &received_length);
```

### Error Handling

If an error occurs, an ERROR_FRAME is sent with the appropriate error code.



---

## Testing

The codebase includes various test functions to validate the functionality of the protocol. These tests check sending, receiving, processing, and error scenarios.

```c
$ make test
```

---

## License

MIT License

