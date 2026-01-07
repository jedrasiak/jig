#ifndef UUID_H
#define UUID_H

/**
 * Generate a UUID of the specified version.
 *
 * Currently only version 7 (Unix Epoch time-based) is supported.
 *
 * UUIDv7 format follows RFC 4122 specification:
 *   - 48-bit Unix timestamp in milliseconds (bits 0-47)
 *   - 4-bit version field set to 0b0111 (7) (bits 48-51)
 *   - 12-bit random data for uniqueness (bits 52-63)
 *   - 2-bit variant field set to 0b10 (bits 64-65)
 *   - 62-bit random data for uniqueness (bits 66-127)
 *
 * The generated UUID has the format: xxxxxxxx-xxxx-7xxx-Nxxx-xxxxxxxxxxxx
 * where 7 indicates version 7 and N is 8, 9, a, or b (variant bits).
 *
 * @param version UUID version to generate (only 7 is supported)
 * @return Newly allocated 36-character UUID string in standard format
 *         Returns NULL on error (with error message written to stderr)
 *         Caller is responsible for freeing the returned string
 *
 * Example usage:
 *   char *id = uuid(7);
 *   if (id != NULL) {
 *       printf("Generated UUID: %s\n", id);
 *       free(id);
 *   }
 */
char *uuid(int version);

#endif /* UUID_H */
