# Application Layer for CAN Transport Protocol

## Overview

The Application layer is built on top of the CAN Transport protocol (CTP) and provides various services to facilitate communication over the CAN bus. These services allow for data reading, writing, security access, routine control, error handling, and more.

## Features

- **Security Access:** Provides mechanisms for secure communication.
- **Read Data By Identifier:** Allows reading data from the system using a unique identifier.
- **Write Data By Identifier:** Enables writing data to the system using a unique identifier.
- **Routine Control:** Calls a general routine.
- **Request Download:** Initiates the download of a file.
- **Negative Response:** Notifies the sender of issues or errors.
- **Read Error Code Information:** Retrieves error code information from the system.
- **System Reset Request:** Requests the system to perform a reset.
- **Session Control:** Enables the categorization of various functions into groups.

## Usage

### Initialization

Before utilizing the Application layer, ensure that the underlying CAN hardware and the CAN Transport Protocol (CTP) are properly initialized.

### Sending and Receiving Messages

Use the `handle_message` function to process incoming application layer messages. This function decodes the service identifier (SID) and delegates the request to the appropriate service handler.

Example:

```c
ApplicationMessage app_msg;
// Fill app_msg with the received data
handle_message(&app_msg);
```

### Error Handling

The Application layer provides comprehensive error handling through negative responses. If a request cannot be fulfilled, the system sends a negative response indicating the reason.

### Security Access

For services that require secure access, the `handle_security_access` function manages security challenges and key generation.

## Testing

A suite of tests is available to ensure the functionality of the Application layer services. Run these tests periodically, especially after making changes to the codebase, to ensure that the system is functioning correctly.

## Dependencies

- CAN Transport Protocol (CTP)
- CAN hardware driver

---

This README provides a brief overview and introduction to the Application layer. Depending on the specifics of the implementation and additional features or nuances, you may want to expand or modify this document.