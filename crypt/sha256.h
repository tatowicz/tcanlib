#ifndef SHA256_H
#define SHA256_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint8_t data[64];  // 64-byte buffer for data
    uint32_t datalen;  // Data length in bytes
    uint64_t bitlen;   // Total data length in bits
    uint32_t state[8]; // Intermediate hash states (H[0] to H[7])
} SHA256_CTX;


void sha256_compute(const uint8_t *data, uint64_t data_len, uint8_t hash[]);
void sha256_init(SHA256_CTX *ctx);
void sha256_update(SHA256_CTX *ctx, const uint8_t data[], size_t len);
void sha256_finalize(SHA256_CTX *ctx, uint8_t hash[]);
void sha256_to_hex_string(const uint8_t hash[32], char *hex_str);
void hex_string_to_byte_array(const char* hex_string, uint8_t* byte_array);

#endif 