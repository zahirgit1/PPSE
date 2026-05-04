#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdint.h>
#include <stddef.h>

// --- SOURCE ---
void source_generate(uint8_t *U_k, size_t K);
void source_generate_all_zeros(uint8_t *U_K, size_t K) ;

// --- CODEC ---
void codec_repetition_encode(const uint8_t *U_k, uint8_t *C_N, size_t K, size_t n_reps);

// --- MODEM ---
void modem_bpsk_modulate(const uint8_t *C_N, int32_t *X_N, size_t N);
void modem_bpsk_modulate_all_ones(const uint8_t *C_N, int32_t *X_N, size_t N);

// --- CHANNEL ---
void channel_AWGN_add_noise(const int32_t *X_N, float *Y_N, size_t N, float sigma);

// --- DEMODULATOR ---
void modem_BPSK_demodulate(const float *Y_N, float *L_N, size_t N, float sigma);
// --- QUANTIZER ---
void quantizer_transform8(const float *L_N, int8_t *L8_N, size_t N, size_t s, size_t f);
// --- DECODER ---
void codec_repetition_hard_decode(const float *L_N, uint8_t *V_K, size_t K, size_t n_reps);
void codec_repetition_soft_decode(const float *L_N, uint8_t *V_K, size_t K, size_t n_reps);
void codec_repetition_hard_decode8(const int8_t *L8_N, uint8_t *V_K, size_t K, size_t n_reps);
void codec_repetition_soft_decode8(const int8_t *L8_N, uint8_t *V_K, size_t K, size_t n_reps);
void codec_repetition_hard_decode8_neon(const int8_t *L8_N, uint8_t *V_K, size_t K, size_t n_reps);
void codec_repetition_soft_decode8_neon(const int8_t *L8_N, uint8_t *V_K, size_t K, size_t n_reps);

// --- MONITOR ---
void monitor_check_errors(const uint8_t *U_K, const uint8_t *V_K, size_t K, uint64_t *n_bit_errors, uint64_t *n_frame_errors);
#endif
