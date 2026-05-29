#include <arm_neon.h>
#include <stdint.h>
#include <stddef.h>



/**
 * Scalar 
 * Layout: bit planes (all 8 bits of each packed byte in sequence)
 */
void unpack_bits(const uint8_t *packed, uint8_t *unpacked, size_t num_packed) {
    // Layout: unpacked[packed_index * 8 + bit] = (packed[packed_index] >> bit) & 1
    for (size_t i = 0; i < num_packed; i++) {
        for (int bit = 0; bit < 8; bit++) {
            unpacked[i * 8 + bit] = (packed[i] >> bit) & 1;
        }
    }
}

/**
 * Scalar pack 
 */
void pack_bits(const uint8_t *unpacked, uint8_t *packed, size_t num_unpacked) {
    size_t num_packed = (num_unpacked + 7) / 8;
    for (size_t i = 0; i < num_packed; i++) {
        uint8_t byte = 0;
        for (int bit = 0; bit < 8; bit++) {
            if (i * 8 + bit < num_unpacked) {
                byte |= (unpacked[i * 8 + bit] & 1) << bit;
            }
        }
        packed[i] = byte;
    }
}
