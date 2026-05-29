#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <arm_neon.h>

/**
 * Scalar version of repetition encoder
 * Each input bit/byte is repeated n_reps times
 */
void codec_repetition_encode(uint8_t *U_k,  uint8_t *C_N, size_t K, size_t n_reps){
    for(size_t i=0; i < K*n_reps; i++){
        C_N[i] = U_k[i % K];
    }
}

/**
 * NEON-vectorized repetition encoder
 * Processes 16 input bytes at a time for each repetition
 */
void codec_repetition_encode_neon(uint8_t *U_k, uint8_t *C_N, size_t K, size_t n_reps){
    // Process each repetition, then vectorize within it
    for (size_t rep = 0; rep < n_reps; rep++) {
        size_t out_offset = rep * K;
        size_t i = 0;
        
        // Process 16 input bytes at a time
        for (; i + 16 <= K; i += 16) {
            uint8x16_t input = vld1q_u8(&U_k[i]);
            vst1q_u8(&C_N[out_offset + i], input);
        }        
    }
}