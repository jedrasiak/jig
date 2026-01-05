#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include "uuid.h"

/**
 * Get current Unix timestamp in milliseconds.
 *
 * Uses gettimeofday() to obtain microsecond precision, then converts to
 * milliseconds as required by UUIDv7 specification.
 *
 * @return Unix timestamp in milliseconds, or 0 on error
 */
static uint64_t get_unix_timestamp_ms(void) {
    struct timeval tv;
    if (gettimeofday(&tv, NULL) != 0) {
        fprintf(stderr, "Failed to get current time\n");
        return 0;
    }

    // Convert to milliseconds: seconds * 1000 + microseconds / 1000
    uint64_t ms = ((uint64_t)tv.tv_sec * 1000ULL) + ((uint64_t)tv.tv_usec / 1000ULL);
    return ms;
}

/**
 * Read cryptographically secure random bytes from /dev/urandom.
 *
 * Per RFC 4122 § 6.9, UUIDs should use a cryptographically secure
 * pseudorandom number generator (CSPRNG) for unguessability.
 *
 * @param buffer Destination buffer for random bytes
 * @param size Number of bytes to read
 * @return 0 on success, 1 on error
 */
static int get_random_bytes(uint8_t *buffer, size_t size) {
    FILE *urandom = fopen("/dev/urandom", "rb");
    if (urandom == NULL) {
        fprintf(stderr, "Failed to open /dev/urandom for random data\n");
        return 1;
    }

    size_t bytes_read = fread(buffer, 1, size, urandom);
    fclose(urandom);

    if (bytes_read != size) {
        fprintf(stderr, "Failed to read sufficient random data\n");
        return 1;
    }

    return 0;
}

/**
 * Generate UUID version 7 bytes following RFC 4122 specification.
 *
 * Bit layout per RFC 4122 § 5.7:
 *   Bytes 0-5:   48-bit Unix timestamp in milliseconds (big-endian)
 *   Byte 6:      4-bit version (0b0111 = 7) + 4-bit rand_a (upper)
 *   Byte 7:      8-bit rand_a (lower)
 *   Byte 8:      2-bit variant (0b10) + 6-bit rand_b (upper)
 *   Bytes 9-15:  56-bit rand_b (lower)
 *
 * @param uuid_bytes 16-byte buffer to store the generated UUID
 * @return 0 on success, 1 on error
 */
static int generate_uuidv7_bytes(uint8_t uuid_bytes[16]) {
    // Get Unix timestamp in milliseconds
    uint64_t timestamp_ms = get_unix_timestamp_ms();
    if (timestamp_ms == 0) {
        return 1;
    }

    // Get 16 bytes of random data (we'll overwrite some with timestamp/version/variant)
    if (get_random_bytes(uuid_bytes, 16) != 0) {
        return 1;
    }

    // Set timestamp (48 bits, bytes 0-5, big-endian)
    uuid_bytes[0] = (uint8_t)((timestamp_ms >> 40) & 0xFF);
    uuid_bytes[1] = (uint8_t)((timestamp_ms >> 32) & 0xFF);
    uuid_bytes[2] = (uint8_t)((timestamp_ms >> 24) & 0xFF);
    uuid_bytes[3] = (uint8_t)((timestamp_ms >> 16) & 0xFF);
    uuid_bytes[4] = (uint8_t)((timestamp_ms >> 8) & 0xFF);
    uuid_bytes[5] = (uint8_t)(timestamp_ms & 0xFF);

    // Set version (4 bits at position 48-51, byte 6 upper nibble)
    // Version 7 = 0b0111 = 0x7
    // Preserve lower 4 bits of random data
    uuid_bytes[6] = (uuid_bytes[6] & 0x0F) | 0x70;

    // Set variant (2 bits at position 64-65, byte 8 upper 2 bits)
    // Variant 10xx = 0b10 in upper 2 bits
    // Preserve lower 6 bits of random data
    uuid_bytes[8] = (uuid_bytes[8] & 0x3F) | 0x80;

    return 0;
}

/**
 * Format UUID bytes to standard hyphenated string format.
 *
 * Standard UUID format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
 * Positions: 8-4-4-4-12 hexadecimal characters
 *
 * @param uuid_bytes 16-byte UUID to format
 * @return Newly allocated 36-character UUID string, or NULL on malloc failure
 */
static char *format_uuid_string(const uint8_t uuid_bytes[16]) {
    // Allocate 37 bytes: 32 hex chars + 4 hyphens + 1 null terminator
    char *uuid_str = malloc(37);
    if (uuid_str == NULL) {
        fprintf(stderr, "Failed to allocate memory for UUID string\n");
        return NULL;
    }

    // Format with hyphens at positions: 8-4-4-4-12
    snprintf(uuid_str, 37,
        "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        uuid_bytes[0], uuid_bytes[1], uuid_bytes[2], uuid_bytes[3],
        uuid_bytes[4], uuid_bytes[5],
        uuid_bytes[6], uuid_bytes[7],
        uuid_bytes[8], uuid_bytes[9],
        uuid_bytes[10], uuid_bytes[11], uuid_bytes[12],
        uuid_bytes[13], uuid_bytes[14], uuid_bytes[15]
    );

    return uuid_str;
}

/**
 * Generate a UUID of the specified version.
 *
 * Public API function. See uuid.h for full documentation.
 */
char *uuid(int version) {
    // Only version 7 is currently supported
    if (version != 7) {
        fprintf(stderr, "Unsupported UUID version: %d (only version 7 is supported)\n", version);
        return NULL;
    }

    // Generate UUID v7 bytes
    uint8_t uuid_bytes[16];
    if (generate_uuidv7_bytes(uuid_bytes) != 0) {
        return NULL;
    }

    // Format to string
    return format_uuid_string(uuid_bytes);
}
