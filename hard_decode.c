#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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