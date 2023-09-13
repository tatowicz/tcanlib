#pragma once
#include <stdint.h>
#include <stddef.h>

typedef enum {
    IO_SUCCESS,
    IO_ERROR_INIT,
    IO_ERROR_READ,
    IO_ERROR_WRITE,
    // ... other error codes
} IOStatus;

typedef void (*DataReceivedCallback)(const uint8_t* data, size_t len);
typedef void (*ErrorCallback)(IOStatus error_code);
typedef void (*WriteCompleteCallback)(void);

typedef struct {
    IOStatus (*init)(void* config);
    IOStatus (*read)(uint8_t* buffer, size_t len);
    IOStatus (*write)(const uint8_t* data, size_t len);
    IOStatus (*status)(void);
    IOStatus (*shutdown)(void);
    void (*register_data_received_callback)(DataReceivedCallback callback);
    void (*register_error_callback)(ErrorCallback callback);
    void (*register_write_complete_callback)(WriteCompleteCallback callback);
} IOInterface;

