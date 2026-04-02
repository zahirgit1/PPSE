#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "chaine_com.h"

int main(int argc, char *argv[]) {
	if (argc != 5) {
		printf("Usage: %s <K> <n_reps> <sigma> <frames>\n", argv[0]);
		return 1;
	}
	size_t K = atoi(argv[1]);
	size_t n_reps = atoi(argv[2]);
	float sigma = atof(argv[3]);
	int frames = atoi(argv[4]);
	printf("sigma %f \n", sigma);
	uint8_t U_k[K];
	uint8_t C_N[K * n_reps];
	int32_t X_N[K * n_reps];
	float Y_N[K * n_reps];
	float L_N[K * n_reps];
	uint8_t V_K_hard[K];
	uint8_t V_K_soft[K];
	uint64_t n_bit_errors_hard = 0;		
	uint64_t n_frame_errors_hard = 0;
	uint64_t n_bit_errors_soft = 0;		
	uint64_t n_frame_errors_soft = 0;
	srand((unsigned int)time(NULL));
	for (int i = 0; i < frames; i++) {
	
		source_generate(U_k, K);
		codec_repetition_encode(U_k, C_N, K, n_reps);
		modem_bpsk_modulate(C_N, X_N, K * n_reps);
		channel_AWGN_add_noise(X_N, Y_N, K * n_reps, sigma);
		modem_BPSK_demodulate(Y_N, L_N, K * n_reps, sigma);
		codec_repetition_hard_decode(L_N, V_K_hard, K, n_reps);
		codec_repetition_soft_decode(L_N, V_K_soft, K, n_reps);
		monitor_check_errors(U_k, V_K_hard, K, &n_bit_errors_hard, &n_frame_errors_hard);
		monitor_check_errors(U_k, V_K_soft, K, &n_bit_errors_soft, &n_frame_errors_soft);
	}
	float fer_hard = (float)n_frame_errors_hard / frames;
	float ber_hard = (float)n_bit_errors_hard / (frames * K);
	float fer_soft = (float)n_frame_errors_soft / frames;
	float ber_soft = (float)n_bit_errors_soft / (frames * K);
	printf("Hard Decode: Bit Errors = %lu, Frame Errors = %lu, BER = %f, FER = %f\n", n_bit_errors_hard, n_frame_errors_hard, ber_hard, fer_hard);
	printf("Soft Decode: Bit Errors = %lu, Frame Errors = %lu, BER = %f, FER = %f\n", n_bit_errors_soft, n_frame_errors_soft, ber_soft, fer_soft);
	return 0;				

}
