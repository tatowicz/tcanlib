#include <stdint.h>
#include <stdio.h>
#include <string.h>

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

// Initial hash values (first 32 bits of the fractional parts of the square roots of the first eight prime numbers)
static uint32_t H[8] = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
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

// Main SHA-256 transformation
void sha256_transform(const uint8_t data[64]) {
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
    a = H[0]; b = H[1]; c = H[2]; d = H[3];
    e = H[4]; f = H[5]; g = H[6]; h = H[7];

    // Main loop
    for (t = 0; t < 64; t++) {
        T1 = h + BIGSIG1(e) + CH(e, f, g) + K[t] + W[t];
        T2 = BIGSIG0(a) + MAJ(a, b, c);
        h = g; g = f; f = e; e = d + T1;
        d = c; c = b; b = a; a = T1 + T2;
    }

    // Add the compressed chunk to the current hash value
    H[0] += a; H[1] += b; H[2] += c; H[3] += d;
    H[4] += e; H[5] += f; H[6] += g; H[7] += h;
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

void sha256_compute(const uint8_t *data, uint64_t data_len) {
    uint8_t padded_data[((data_len + 8 + 64) & ~((uint64_t)63)) - 8];
    sha256_pad(data, data_len, padded_data);
    
    size_t num_chunks = (data_len + 8 + 64) / 64;
    for (size_t i = 0; i < num_chunks; i++) {
        sha256_transform(padded_data + i * 64, H);
    }
}

void sha256_to_hex_string(char *hex_str) {
    for (int i = 0; i < 8; i++) {
        sprintf(hex_str + i * 8, "%08x", H[i]);
    }
}

int main() {
    const char *data = "this is a test";

    char hex_str[65] = {0};

    sha256_compute((const uint8_t *)data, strlen(data));
    sha256_to_hex_string(hex_str);

    printf("SHA-256 hash: %s\n", hex_str);

    return 0;
}