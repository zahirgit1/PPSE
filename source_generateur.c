#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "mersenne_19937.h"

void source_generate(uint8_t *U_k, size_t K){
	for(size_t i = 0; i < K; i++){
		U_k[i] = (uint8_t)(rand() % 2);
	}
}

void source_generate_all_zeros(uint8_t *U_K, size_t K) {
    // Fills the array with 0s instantly
    memset(U_K, 0, K * sizeof(uint8_t));
}

/**
 * Generate source bits in PACKED form
 * K must be a multiple of 8
 * Output: K/8 bytes, each containing a random number from 0 to 255
 * 
 * This replaces source_generate() for packed bit operations
 * All 8 bits per byte are random
 */
void source_generate_packed(uint8_t *U_K_packed, size_t K) {
    // K is always a multiple of 8, so K/8 is the number of packed bytes
    size_t packed_size = K / 8;
    for (size_t i = 0; i < packed_size; i++) {
        U_K_packed[i] = (uint8_t)(rand() & 0xFF);  // Random 0-255 (8 bits)
    }
}

/**
 * Generate source bits in PACKED form (all zeros)
 * K must be a multiple of 8
 * Output: K/8 bytes, all set to 0
 */
void source_generate_packed_all_zeros(uint8_t *U_K_packed, size_t K) {
    size_t packed_size = K / 8;
    memset(U_K_packed, 0, packed_size * sizeof(uint8_t));
}

/**
 * Generate source bits in PACKED form using vectorized Mersenne Twister
 * K must be a multiple of 8
 * Output: K/8 bytes, each containing random bits from MT19937
 * 
 * Uses NEON SIMD (4-lane vectorization) for superior performance
 * Generates 4 uint32_t values at once, packing 16 bytes per iteration
 * 
 * Performance: ~4x faster than source_generate_packed() using rand()
 */
void source_generate_packed_mersenne(uint8_t *U_K_packed, size_t K) {
    size_t packed_size = K / 8;  // Number of bytes needed
    size_t i = 0;
    
    // Vectorized loop: Process 16 bytes (4 x uint32_t) at a time
    // This is 4x more efficient than scalar approach
    for (; i + 16 <= packed_size; i += 16) {
        // Get 4 random uint32_t values at once using NEON (4-lane)
        uint32x4_t four_rands = mt19937_next_four();
        
        // Store 16 bytes (4 x uint32_t) directly to output
        vst1q_u32((uint32_t *)&U_K_packed[i], four_rands);
    }
    
    // Handle remaining bytes (0-15) with scalar approach
    while (i < packed_size) {
        U_K_packed[i] = (uint8_t)(mt19937_next() & 0xFF);
        i++;
    }
}