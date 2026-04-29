#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

void source_generate(uint8_t *U_k, size_t K){
	for(size_t i = 0; i < K; i++){
		U_k[i] = (uint8_t)(rand() % 2);
	}
}
void source_generate_all_zeros(uint8_t *U_K, size_t K) {
    // Fills the array with 0s instantly
    memset(U_K, 0, K * sizeof(uint8_t));
}