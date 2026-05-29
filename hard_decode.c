#include <stdio.h>
#include <stdint.h>
#include <arm_neon.h>

void codec_repetition_hard_decode(const float *L_N, uint8_t *V_K, size_t K, size_t n_reps)
{   
    for(size_t i = 0; i < K; i++){
        float sum = 0.0f;
        for(size_t j = 0; j < n_reps; j++){
            float symbol = L_N[j * K + i];
            float hard_decision = (symbol >= 0) ? 1.0f : -1.0f;
            sum += hard_decision;
        }
        V_K[i] = (sum >= 0) ? 0 : 1;
    }
}
void codec_repetition_hard_decode8(const int8_t *L8_N, uint8_t *V_K, size_t K, size_t n_reps){
    for(size_t i = 0; i < K; i++){
        int sum = 0;
        for(size_t j = 0; j < n_reps; j++){
            int8_t symbol = L8_N[j * K + i];
            int hard_decision = (symbol >= 0) ? 1 : -1;
            sum += hard_decision;
        }
        V_K[i] = (sum >= 0) ? 0 : 1;
    }
}
void codec_repetition_hard_decode8_neon(const int8_t *L8_N, uint8_t *V_K, size_t K, size_t n_reps){
    for(size_t i = 0; i < K; i += 16){
        int8x16_t sum_vec = vdupq_n_s8(0);  // [0, 0, ..., 0]
        
        // Accumulate hard decisions directly without storing intermediate buffer
        for(size_t j = 0; j < n_reps; j++){
            int8x16_t L_vec = vld1q_s8(L8_N + j * K + i);
            // Create hard decisions: 1 for >=0, -1 for <0
            int8x16_t hard_decision = (int8x16_t)vcltzq_s8(L_vec); // 0xFF for <0, 0x00 for >=0
            uint8x16_t hard_decision_zeros = vceqq_s8(hard_decision, vdupq_n_s8(0)); // 0xFF for ==0, 0x00 for !=0
            int8x16_t hard_decision_0to1 = vandq_s8((int8x16_t)hard_decision_zeros, vdupq_n_s8(1)); // 1 for every FF, 0 for every 00
            int8x16_t final_decision = vaddq_s8(hard_decision_0to1, hard_decision); // 1 for >=0, -1 for <0
            sum_vec = vqaddq_s8(sum_vec, final_decision);  // Accumulate
        }
        
        // Compare each lane: sum >= 0 means output 0, sum < 0 means output 1
        int8x16_t hard_decision = (int8x16_t)vcltzq_s8(sum_vec);  // 0xFF for <0, 0x00 for >=0
        int8x16_t output_vec = vandq_s8(hard_decision, vdupq_n_s8(1));  // 1 for <0, 0 for >=0
        
        vst1q_s8((int8_t*)(V_K + i), output_vec);  // Store 16 outputs
    }
}