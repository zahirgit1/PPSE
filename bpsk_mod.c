#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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