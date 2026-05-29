#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <arm_neon.h>


void monitor_check_errors(const uint8_t *U_K, const uint8_t *V_K, size_t K, uint64_t *n_bit_errors, uint64_t *n_frame_errors){
    bool error =0;
    for(size_t i = 0; i < K; i++){
       
        if(U_K[i] != V_K[i]){
            (*n_bit_errors)++;
            error = 1;// Count only one frame error per frame
        }
    }
    if(error) {
        (*n_frame_errors)++;
        error = 0;}
}

/**
 * High-performance NEON-vectorized error monitor
 * Uses bitwise AND + popcount approach:
 * - AND(U_K, V_K) gives matching bits
 * - Error count = popcount(U_K XOR V_K)
 * - Processes 16 bytes per iteration
 */
void monitor_check_errors_neon(const uint8_t *U_K, const uint8_t *V_K, size_t K, uint64_t *n_bit_errors, uint64_t *n_frame_errors){
    size_t i = 0;
    uint32_t error_count = 0;
    
    // Process 16 bytes at a time using NEON
    const size_t chunk_size = 16;
    
    for (; i + chunk_size <= K; i += chunk_size) {
        // Load 16 bytes from both U_K and V_K
        uint8x16_t u_vals = vld1q_u8(&U_K[i]);
        uint8x16_t v_vals = vld1q_u8(&V_K[i]);
        
        // XOR to find bit differences (1s where bits differ)
        uint8x16_t errors = veorq_u8(u_vals, v_vals);
        
        // Sum all error bits across the lane - gives total error count
        error_count += vaddvq_u8(errors);
    }
    
    
    // Update statistics
    if (error_count > 0) {
        (*n_bit_errors) += error_count;
        (*n_frame_errors)++;
    }
}