#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "sha256.h"



// SHA-256 constants table (first 32 bits of the fractional parts of the cube roots of the first 64 prime numbers)
static const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

// Logical functions and operations used in SHA-256
#define CH(x,y,z) ((x & y) ^ (~x & z))
#define MAJ(x,y,z) ((x & y) ^ (x & z) ^ (y & z))
#define ROTR(x,n) ((x >> n) | (x << (32 - n)))
#define SHR(x,n) (x >> n)
#define SIG0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3))
#define SIG1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10))
#define BIGSIG0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define BIGSIG1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))


void sha256_init(SHA256_CTX *ctx) {
    ctx->datalen = 0; 
    ctx->bitlen = 0;
    // Initialize state values (Based on SHA-256 specifications)
    ctx->state[0] = 0x6a09e667;
    ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f;
    ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab;
    ctx->state[7] = 0x5be0cd19;
}

// Main SHA-256 transformation
void sha256_transform(const uint8_t data[64], uint32_t state[8]) {
    uint32_t W[64];
    uint32_t a, b, c, d, e, f, g, h, T1, T2;
    int t;

    // Prepare the message schedule
    for (t = 0; t < 16; t++) {
        W[t] = (data[t * 4] << 24) | (data[t * 4 + 1] << 16) | (data[t * 4 + 2] << 8) | (data[t * 4 + 3]);
    }
    for (; t < 64; t++) {
        W[t] = SIG1(W[t - 2]) + W[t - 7] + SIG0(W[t - 15]) + W[t - 16];
    }

    // Initialize working variables with current hash value
    a = state[0]; b = state[1]; c = state[2]; d = state[3];
    e = state[4]; f = state[5]; g = state[6]; h = state[7];

    // Main loop
    for (t = 0; t < 64; t++) {
        T1 = h + BIGSIG1(e) + CH(e, f, g) + K[t] + W[t];
        T2 = BIGSIG0(a) + MAJ(a, b, c);
        h = g; g = f; f = e; e = d + T1;
        d = c; c = b; b = a; a = T1 + T2;
    }

    // Add the compressed chunk to the current hash value
    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

void sha256_pad(uint8_t *data, uint64_t data_len, uint8_t *padded_data) {
    uint64_t bit_len = data_len * 8;
    size_t padded_len = ((data_len + 8 + 64) & ~((uint64_t)63)) - 8;
    
    memcpy(padded_data, data, data_len);
    padded_data[data_len] = 0x80;
    memset(padded_data + data_len + 1, 0x00, padded_len - data_len - 1);
    for (int i = 0; i < 8; i++) {
        padded_data[padded_len + i] = (bit_len >> ((7 - i) * 8)) & 0xFF;
    }
}

void sha256_update(SHA256_CTX *ctx, const uint8_t data[], size_t len) {
    size_t i;

    // If there's leftover data from the previous update, try to form a full block
    while(ctx->datalen > 0 && len > 0) {
        ctx->data[ctx->datalen] = data[0];
        ctx->datalen++;
        data++;
        len--;

        // If we have a full block, process it
        if(ctx->datalen == 64) {
            sha256_transform(ctx->data, ctx->state);
            ctx->bitlen += 512;  // Update the bit count by 64 bytes * 8
            ctx->datalen = 0;  // Reset the data length
        }
    }

    // Process full blocks directly from the input data
    while(len >= 64) {
        sha256_transform(data, ctx->state);
        ctx->bitlen += 512;  // Update the bit count by 64 bytes * 8
        data += 64;
        len -= 64;
    }

    // Buffer any remaining data for the next update
    for(i = 0; i < len; i++) {
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
    }
}

void sha256_finalize(SHA256_CTX *ctx, uint8_t hash[]) {
    uint32_t i;

    // Determine padding bytes
    i = ctx->datalen;
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;  // Append the 1 bit
        while (i < 56) {
            ctx->data[i++] = 0x00;  // Append zeros
        }
    } else {
        ctx->data[i++] = 0x80;  // Append the 1 bit
        while (i < 64) {
            ctx->data[i++] = 0x00;  // Append zeros
        }
        sha256_transform(ctx->data, ctx->state);  // Process this block
        memset(ctx->data, 0, 56);  // Zero out data
    }

    // Append the total bit length (big endian)
    ctx->bitlen += ctx->datalen * 8;
    ctx->data[63] = ctx->bitlen;
    ctx->data[62] = ctx->bitlen >> 8;
    ctx->data[61] = ctx->bitlen >> 16;
    ctx->data[60] = ctx->bitlen >> 24;
    ctx->data[59] = ctx->bitlen >> 32;
    ctx->data[58] = ctx->bitlen >> 40;
    ctx->data[57] = ctx->bitlen >> 48;
    ctx->data[56] = ctx->bitlen >> 56;
    
    sha256_transform(ctx->data, ctx->state);  // Process the final block

    // Store the final hash values in the output hash
    for (i = 0; i < 8; i++) {
        hash[i*4] = (ctx->state[i] >> 24) & 0xFF;
        hash[i*4 + 1] = (ctx->state[i] >> 16) & 0xFF;
        hash[i*4 + 2] = (ctx->state[i] >> 8) & 0xFF;
        hash[i*4 + 3] = ctx->state[i] & 0xFF;
    }
}

void sha256_compute(const uint8_t *data, uint64_t data_len, uint8_t hash[]) {
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, data, data_len);
    sha256_finalize(&ctx, hash);
}

void sha256_to_hex_string(const uint8_t hash[32], char *hex_str) {
    for (int i = 0; i < 32; i++) {
        sprintf(hex_str + i * 2, "%02x", hash[i]);
    }
    hex_str[64] = '\0';  // Null terminate the string
}

void test_sha256_incremental() {
    char hex_str[65];
    uint8_t hash1[32];
    uint8_t hash2[32];
    const char *test_data = "Hello, World!";
    const char *part1 = "Hello, ";
    const char *part2 = "World!";
    int i;

    // Hash data in one go
    sha256_compute((const uint8_t *)test_data, strlen(test_data), hash1);

    // Hash data in parts
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, (const uint8_t *)part1, strlen(part1));
    sha256_update(&ctx, (const uint8_t *)part2, strlen(part2));
    sha256_finalize(&ctx, hash2);

    // Compare the hashes
    int are_equal = 1;
    for (i = 0; i < 32; i++) {
        if (hash1[i] != hash2[i]) {
            are_equal = 0;
            break;
        }
    }

    if (are_equal) {
        printf("Test passed! Hashes are identical.\n");
    } else {
        printf("Test failed! Hashes are different.\n");
    }

    sha256_to_hex_string(hash1, hex_str);
    printf("Input: %s\n", test_data);
    printf("Hash: %s\n", hex_str);
}

int main() {
    test_sha256_incremental();
    return 0;
}
