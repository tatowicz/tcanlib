#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "sha256.h"

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
    // TOOD: Need to add test vectors
    return 0;
}