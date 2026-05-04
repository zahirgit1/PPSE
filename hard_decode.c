#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <arm_neon.h>

void codec_repetition_hard_decode(const float *L_N, uint8_t *V_K, size_t K, size_t n_reps)
{   
    float *L_copy = malloc(K * n_reps * sizeof(float));
    L_copy = memcpy(L_copy, L_N, K * n_reps * sizeof(float));
    for(size_t i = 0; i < K * n_reps; i++){
        L_copy[i] = (L_copy[i] >= 0) ? 1.0f : -1.0f; // Hard decision on each symbol
    }
    for(size_t i = 0; i < K; i++){
        float sum = 0.0f;
        for(size_t j = 0; j < n_reps; j++){
            sum += L_copy[j * K + i];
        }
        V_K[i] = (sum >= 0) ? 0 : 1; // Decision based on the sign of the sum
    }
    free(L_copy);
}
void codec_repetition_hard_decode8(const int8_t *L8_N, uint8_t *V_K, size_t K, size_t n_reps){
    int8_t *L_copy = malloc(K * n_reps * sizeof(int8_t));
    L_copy = memcpy(L_copy, L8_N, K * n_reps * sizeof(int8_t));
    for(size_t i = 0; i < K * n_reps; i++){
        L_copy[i] = (L_copy[i] >= 0) ? 1 : -1; // Hard decision on each symbol
    }
    for(size_t i = 0; i < K; i++){
        int sum = 0;
        for(size_t j = 0; j < n_reps; j++){
            sum += L_copy[j * K + i];
        }
        V_K[i] = (sum >= 0) ? 0 : 1; // Decision based on the sign of the sum
    }
    free(L_copy);
}
void codec_repetition_hard_decode8_neon(const int8_t *L8_N, uint8_t *V_K, size_t K, size_t n_reps){
    int8_t *L_copy = malloc(K * n_reps * sizeof(int8_t));
    for(size_t i = 0; i < K * n_reps; i += 16){
        int8x16_t L_vec = vld1q_s8(L8_N + i);
        int8x16_t hard_decision = (int8x16_t)vcltzq_s8(L_vec); // 0X00 for >=0, 0XFF for <0
        uint8x16_t hard_decision_zeros = vceqq_s8(hard_decision, vdupq_n_s8(0)); // 0xFF for ==0, 0x00 for !=0
        int8x16_t hard_decision_0to1 = vandq_s8((int8x16_t)hard_decision_zeros, vdupq_n_s8(1)); // 1 for evey FF, 0 for every 00
        int8x16_t final_decision = vaddq_s8(hard_decision_0to1, hard_decision); // 1 for >=0, -1 for <0
        vst1q_s8(L_copy + i, final_decision);
    }
    for(size_t i = 0; i < K; i += 16){
        int8x16_t sum_vec = vdupq_n_s8(0);  // [0, 0, ..., 0]
        
        // Accumulate n_reps values into each lane
        for(size_t j = 0; j < n_reps; j++){
            int8x16_t vals = vld1q_s8(L_copy + j * K + i);  // Load 16 symbols
            sum_vec = vqaddq_s8(sum_vec, vals);  // Add to each lane
        }
        
        // Compare each lane: sum >= 0 means output 0, sum < 0 means output 1
        int8x16_t hard_decision = (int8x16_t)vcltzq_s8(sum_vec);  // 0xFF for <0, 0x00 for >=0
        int8x16_t output_vec = vandq_s8(hard_decision, vdupq_n_s8(1));  // 1 for <0, 0 for >=0
        
        vst1q_s8((int8_t*)(V_K + i), output_vec);  // Store 16 outputs
    }
    free(L_copy);
}