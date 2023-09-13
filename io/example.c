#include <stdint.h>
#include <stdio.h>

#include "io.h"

// This is an example driver for SPI
// It is not complete and is only meant to demonstrate the API
// It can be copied an modified to create a new driver
static DataReceivedCallback spi_data_received_callback = NULL;
static ErrorCallback spi_error_callback = NULL;
static WriteCompleteCallback spi_write_complete_callback = NULL;

IOStatus SPI_init(void* config) {
    // SPI initialization code
    printf("[DEBUG] Initializing SPI...\n");
    return IO_SUCCESS;
}

IOStatus SPI_read(uint8_t* buffer, size_t len) {
    printf("[DEBUG] Reading %zu bytes from SPI...\n", len);
    // SPI read code
    if (0) {
        printf("[ERROR] SPI read error occurred.\n");
        if (spi_error_callback) {
            spi_error_callback(IO_ERROR_READ);
        }
        return IO_ERROR_READ;
    }
    
    // After reading data
    printf("[DEBUG] Read %zu bytes from SPI successfully.\n", len);
    if (spi_data_received_callback) {
        spi_data_received_callback(buffer, len);
    }
    return IO_SUCCESS;
}

IOStatus SPI_write(const uint8_t* data, size_t len) {
    printf("[DEBUG] Writing %zu bytes to SPI...\n", len);
    // SPI write code
    if (0) {
        printf("[ERROR] SPI write error occurred.\n");
        if (spi_error_callback) {
            spi_error_callback(IO_ERROR_WRITE);
        }
        return IO_ERROR_WRITE;
    }
    
    // After write completion
    printf("[DEBUG] Wrote %zu bytes to SPI successfully.\n", len);
    if (spi_write_complete_callback) {
        spi_write_complete_callback();
    }
    return IO_SUCCESS;
}

IOStatus SPI_status(void) {
    // Check SPI status
    return IO_SUCCESS;
}

IOStatus SPI_shutdown(void) {
    printf("[DEBUG] SPI shut down successfully.\n");
    return IO_SUCCESS;
}

void SPI_register_data_received_callback(DataReceivedCallback callback) {
    spi_data_received_callback = callback;
}

void SPI_register_error_callback(ErrorCallback callback) {
    spi_error_callback = callback;
}

void SPI_register_write_complete_callback(WriteCompleteCallback callback) {
    spi_write_complete_callback = callback;
}


// This is the portion of the code that uses the IO interface
// It can be implemented with or without callbacks
const IOInterface SPI_Driver = {
    .init = SPI_init,
    .read = SPI_read,
    .write = SPI_write,
    .status = SPI_status,
    .shutdown = SPI_shutdown,
    // Callbacks can be added here
    .register_data_received_callback = SPI_register_data_received_callback,
    .register_error_callback = SPI_register_error_callback,
    .register_write_complete_callback = SPI_register_write_complete_callback
};

void data_received_handler(const uint8_t* data, size_t len) {
    // Handle received data
}

void error_handler(IOStatus error_code) {
    // Handle errors
}

void write_complete_handler() {
    // Handle write completion
}

int main() {
    const IOInterface* io_driver = &SPI_Driver;  // This can be any supported driver
    void *config = NULL;                         // This can be any configuration data
    io_driver->init(config);

    // Callbacks, these can be left out if not needed
    io_driver->register_data_received_callback(data_received_handler);
    io_driver->register_error_callback(error_handler);
    io_driver->register_write_complete_callback(write_complete_handler);

    uint8_t data[10];
    io_driver->read(data, 10);
    io_driver->write(data, 10);
    io_driver->shutdown();

    return 0;
}
