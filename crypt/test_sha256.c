#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <stdlib.h>

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

void run_test(const char* hex_input, const char* hex_expected_output, const char* length) {
    uint32_t len = atoi(length) / 8;
    unsigned char input[len];
    unsigned char expected_output[32];
    unsigned char output[32];
    char output_hex_string[65];
    static uint32_t counter = 0;

    hex_string_to_byte_array(hex_input, input);
    hex_string_to_byte_array(hex_expected_output, expected_output);

    sha256_compute(input, len, output);
    
    if (memcmp(output, expected_output, 32) == 0) {
        printf("Test %d passed!\n", counter);
    } else {
        printf("Test %d failed!\n", counter);
    }
    
    printf("Input: %s\n", hex_input);
    sha256_to_hex_string(output, output_hex_string);
    printf("Hash: %s\n", output_hex_string);
    counter++;
}

int main() {
    test_sha256_incremental();

    FILE *file = fopen("SHA256ShortMsg.rsp", "r");
    if (file == NULL) {
        printf("Error opening file.\n");
        return 1;
    }

    char line[256];
    char hex_input[256];
    char hex_expected_output[65];
    char length[16];

    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "Len =")) {
            sscanf(line, "Len = %s", length);
        }
        if (strstr(line, "Msg =")) {
            sscanf(line, "Msg = %s", hex_input);
        }
        if (strstr(line, "MD =")) {
            sscanf(line, "MD = %s", hex_expected_output);
            run_test(hex_input, hex_expected_output, length);
        }
    }

    fclose(file);

    return 0;
}