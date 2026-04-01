#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdint.h>
#include <stddef.h>

// --- SOURCE ---
void source_generate(uint8_t *U_k, size_t K);

// --- CODEC ---
void codec_repetition_encode(const uint8_t *U_k, uint8_t *C_N, size_t K, size_t n_reps);

// --- MODEM ---
void modem_bpsk_modulate(const uint8_t *C_N, int32_t *X_N, size_t N);

// --- CHANNEL ---
void channel_AWGN_add_noise(const int32_t *X_N, float *Y_N, size_t N, float sigma);

#endif