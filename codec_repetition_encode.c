#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void codec_repetition_encode(uint8_t *U_k,  uint8_t *C_N, size_t K, size_t n_reps){
    for(size_t i=0; i < K*n_reps; i++){
        C_N[i] = U_k[i % K];
    }
}