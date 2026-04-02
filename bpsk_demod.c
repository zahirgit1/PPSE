#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


void modem_BPSK_demodulate(const float *Y_N, float *L_N, size_t N, float sigma){

    for(size_t i = 0; i < N; i++){
        L_N[i] = Y_N[i];
    }
}