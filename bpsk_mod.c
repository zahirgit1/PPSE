#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <arm_neon.h>

void modem_bpsk_modulate(const uint8_t *C_N, int32_t *X_N, size_t N){
    for (size_t i = 0; i < N; i++){
        X_N[i] = (C_N[i] == 0) ? 1 : -1;
    }
}
void modem_bpsk_modulate_all_ones(const uint8_t *C_N, int32_t *X_N, size_t N){
    for (size_t i = 0; i < N; i++){
        X_N[i] = 1; 
    }
}

/**
 * NEON-vectorized BPSK modulator (int32_t version)
 * Processes 16 samples per iteration for maximum throughput
 */
void modem_bpsk_modulate_neon_i32(const uint8_t *C_N, int32_t *X_N, size_t N){
    size_t i = 0;
    
    //constants
    const int32x4_t ones = vdupq_n_s32(1);
    const int32x4_t neg_ones = vdupq_n_s32(-1);
    const uint8x16_t zero_vec = vdupq_n_u8(0);
    
    // Process 16 bytes
    const size_t chunk_size = 16;
    
    for (; i + chunk_size <= N; i += chunk_size) {
        // Load 16 uint8_t input bits
        uint8x16_t bits = vld1q_u8(&C_N[i]);
        
        // Compare with 0: creates 0xFF for zero bits, 0x00 for one bits
        uint8x16_t is_zero = vceqq_u8(bits, zero_vec);
        
        // Process first 4 elements (bytes 0-3)
        uint8x8_t low_8 = vget_low_u8(is_zero);
        uint16x8_t low_16 = vmovl_u8(low_8);
        uint32x4_t low_32_0 = vmovl_u16(vget_low_u16(low_16));
        int32x4_t out0 = vbslq_s32(low_32_0, ones, neg_ones);
        vst1q_s32(&X_N[i], out0);
        
        // Process second 4 elements (bytes 4-7)
        uint32x4_t low_32_1 = vmovl_u16(vget_high_u16(low_16));
        int32x4_t out1 = vbslq_s32(low_32_1, ones, neg_ones);
        vst1q_s32(&X_N[i + 4], out1);
        
        // Process third 4 elements (bytes 8-11)
        uint8x8_t high_8 = vget_high_u8(is_zero);
        uint16x8_t high_16 = vmovl_u8(high_8);
        uint32x4_t high_32_0 = vmovl_u16(vget_low_u16(high_16));
        int32x4_t out2 = vbslq_s32(high_32_0, ones, neg_ones);
        vst1q_s32(&X_N[i + 8], out2);
        
        // Process fourth 4 elements (bytes 12-15)
        uint32x4_t high_32_1 = vmovl_u16(vget_high_u16(high_16));
        int32x4_t out3 = vbslq_s32(high_32_1, ones, neg_ones);
        vst1q_s32(&X_N[i + 12], out3);
    }
    
    // Handle remaining bytes with scalar code
    for (; i < N; i++) {
        X_N[i] = (C_N[i] == 0) ? 1 : -1;
    }
}

/**
 * NEON-vectorized BPSK modulator (int8_t version)
 * Processes 16 samples per iteration directly in int8_t precision
 * 4x memory efficient compared to int32_t version
 */
void modem_bpsk_modulate_neon(const uint8_t *C_N, int8_t *X_N, size_t N){
    size_t i = 0;
    
    // NEON constants for int8_t
    const int8x16_t ones = vdupq_n_s8(1);
    const int8x16_t neg_ones = vdupq_n_s8(-1);
    const uint8x16_t zero_vec = vdupq_n_u8(0);
    
    // Process 16 bytes at a time - optimal for int8_t
    const size_t chunk_size = 16;
    
    for (; i + chunk_size <= N; i += chunk_size) {
        // Load 16 uint8_t input bits
        uint8x16_t bits = vld1q_u8(&C_N[i]);
        
        // Compare with 0: creates 0xFF for zero bits, 0x00 for one bits
        uint8x16_t is_zero = vceqq_u8(bits, zero_vec);
        int8x16_t result = vbslq_s8(is_zero, ones, neg_ones);
        
        // Store 16 int8_t results directly
        vst1q_s8(&X_N[i], result);
    }
}