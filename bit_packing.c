#include <arm_neon.h>
#include <stdint.h>
#include <stddef.h>

/**
 * NEON-optimized unpack - processes 16 packed bytes in parallel
 * Extracts all 8 bit positions from 16 bytes using SIMD operations
 * Layout: unpacked[i*8+bit] = bit 'bit' of packed[i]
 */
void unpack_bits_neon(const uint8_t *packed, uint8_t *unpacked, size_t num_packed) {
    size_t i = 0;
    
    // Process 16 packed bytes at a time
    for (; i + 16 <= num_packed; i += 16) {
        uint8x16_t packed_vals = vld1q_u8(&packed[i]);
        
        // Extract each bit and store interleaved
        for (int bit = 0; bit < 8; bit++) {
            // Extract bit at position 'bit' from all 16 bytes
            uint8x16_t shifted = vshrq_n_u8(packed_vals, bit);
            uint8x16_t mask = vandq_u8(shifted, vmovq_n_u8(1));
            
            // Store to temp buffer and copy to final position
            // unpacked[(i+j)*8 + bit] for j in 0..15
            uint8_t temp[16];
            vst1q_u8(temp, mask);
            
            for (int j = 0; j < 16; j++) {
                unpacked[(i + j) * 8 + bit] = temp[j];
            }
        }
    }
    
    // Handle remaining bytes with scalar
    for (; i < num_packed; i++) {
        for (int bit = 0; bit < 8; bit++) {
            unpacked[i * 8 + bit] = (packed[i] >> bit) & 1;
        }
    }
}

/**
 * NEON-optimized pack - processes 128 unpacked bytes -> 16 packed bytes
 * Combines bit planes using SIMD operations
 * Layout: unpacked[i*8+bit] = bit 'bit' of packed[i]
 */
void pack_bits_neon(const uint8_t *unpacked, uint8_t *packed, size_t num_unpacked) {
    size_t num_packed = (num_unpacked + 7) / 8;
    size_t i = 0;
    
    // Process 128 unpacked bytes (16 packed bytes) at a time
    for (; i + 128 <= num_unpacked; i += 128) {
        uint8x16_t result = vmovq_n_u8(0);
        
        // Load and combine each bit plane
        for (int bit = 0; bit < 8; bit++) {
            // Load 16 bytes: unpacked[i + j*8 + bit] for j in 0..15
            uint8_t temp[16];
            for (int j = 0; j < 16; j++) {
                temp[j] = unpacked[i + j * 8 + bit];
            }
            uint8x16_t bit_vals = vld1q_u8(temp);
            
            // Shift to bit position and OR
            uint8x16_t shifted = vshlq_n_u8(bit_vals, bit);
            result = vorrq_u8(result, shifted);
        }
        
        // Store 16 packed bytes
        vst1q_u8(&packed[i / 8], result);
    }

}

/**
 * Scalar version matching optimized NEON layout
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
 * Scalar pack matching optimized NEON layout
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
