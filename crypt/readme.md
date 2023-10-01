# SHA-256 Implementation

## Introduction
`sha256.c` contains a C implementation of the SHA-256 cryptographic hash function, which is part of the SHA-2 (Secure Hash Algorithm 2) family.

## Features
- **Initialization (`sha256_init`)**: Prepares the SHA-256 context for hashing.
- **Data Processing (`sha256_update`)**: Feeds data into the hasher incrementally.
- **Finalization (`sha256_finalize`)**: Produces the final hash after all data has been processed.
- **Single Shot Hashing (`sha256_compute`)**: Computes the SHA-256 hash of provided data in one go.

## Usage
```c
// Example usage of sha256_compute
uint8_t hash[32];
const char *data = "Your data here!";
sha256_compute((const uint8_t *)data, strlen(data), hash);
```

For incremental hashing:
```c
SHA256_CTX ctx;
uint8_t hash[32];
sha256_init(&ctx);
sha256_update(&ctx, (const uint8_t *)data_part1, len1);
sha256_update(&ctx, (const uint8_t *)data_part2, len2);
// ... continue updating as needed
sha256_finalize(&ctx, hash);
```

## Testing
```
$ make test
```

See `SHA245ShortMsg.rsp` test vectors

## TODO
* Monte carlo testing
* Figure out failing test 0

## References
- Official [SHA-2 Standard](https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.180-4.pdf) from NIST.

