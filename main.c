#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "chaine_com.h"

int main(){

	size_t K = 10;
	size_t n_reps = 4;
	float sigma = 0.4f;
	printf("sigma %f \n", sigma);
	uint8_t U_k[K];
	uint8_t C_N[K * n_reps];
	int32_t X_N[K * n_reps];
	float Y_N[K * n_reps];
	srand((unsigned int)time(NULL));
	source_generate(U_k, K);
	codec_repetition_encode(U_k, C_N, K, n_reps);
	modem_bpsk_modulate(C_N, X_N, K * n_reps);
	channel_AWGN_add_noise(X_N, Y_N, K * n_reps, 0.6f);
	
	printf("print U_k \n");	
	for(int i = 0; i < K; i++){
		printf("%d ", U_k[i]);
	}	
	printf("\n");
	printf("print C_N \n");
	for(int i = 0; i < K * n_reps; i++){
		printf("%d ", C_N[i]);
		if((i+1) % K == 0) printf("\n");
    }	
	printf("print X_N \n");
	for(int i = 0; i < K * n_reps; i++){
		printf("%d ", X_N[i]);
		if((i+1) % K == 0) printf("\n");
    }	
	printf("print Y_N \n");
	for(int i = 0; i < K * n_reps; i++){
		printf("%f ", Y_N[i]);
		if((i+1) % K == 0) printf("\n");
    }
}

