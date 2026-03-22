#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

void source_generate(uint8_t *U_k, size_t K){
	for(size_t i = 0; i < K; i++){
		U_k[i] = (uint8_t)(rand() % 2);
	}
}