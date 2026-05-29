#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <arm_neon.h>


void modem_BPSK_demodulate(const float *Y_N, float *L_N, size_t N, float sigma){

    for(size_t i = 0; i < N; i++){
        L_N[i] = Y_N[i];
    }
}

/**
 * High-performance NEON-vectorized BPSK demodulator
 * Processes 4 float32 samples per iteration using NEON
 * Optimal for large N (>1000)
 */
void modem_BPSK_demodulate_neon(const float *Y_N, float *L_N, size_t N, float sigma){
    size_t i = 0;
    
    // Process 4 floats at a time using NEON
    const size_t chunk_size = 4;
    
    for (; i + chunk_size <= N; i += chunk_size) {
        // Load 4 float32 values from Y_N
        float32x4_t y_vals = vld1q_f32(&Y_N[i]);
        vst1q_f32(&L_N[i], y_vals);
    }


}