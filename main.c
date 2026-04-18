#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include "chaine_com.h"
#ifdef ENABLE_STATS
typedef struct {
    double total_time;
    double min_latency;
    double max_latency;
    uint64_t total_bits;
    const char* name;
} block_stats_t;

void init_stats(block_stats_t *s, const char* name) {
    s->total_time = 0;
    s->min_latency = 1e10; 
    s->max_latency = 0;
    s->total_bits = 0;
    s->name = name;
}

double get_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

#define MEASURE_BLOCK(stats, block_call, bits) { \
    struct timespec ts_s, ts_e; \
    clock_gettime(CLOCK_MONOTONIC, &ts_s); \
    block_call; \
    clock_gettime(CLOCK_MONOTONIC, &ts_e); \
    double lat = get_diff(ts_s, ts_e); \
    stats.total_time += lat; \
    stats.total_bits += (bits); \
    if (lat < stats.min_latency) stats.min_latency = lat; \
    if (lat > stats.max_latency) stats.max_latency = lat; \
}
#else
// If stats disabled, just call the function normally

#define MEASURE_BLOCK(stats, block_call, bits) block_call
#endif
int main(int argc, char *argv[]) {

    float min_SNR = 0.0f, max_SNR = 0.0f, step_val = 1.0f;
    uint32_t f_max = 100, K = 0, N = 0;
    char decoder_type[20] = "rep-hard";
    // Init Stats if enabled
    #ifdef ENABLE_STATS
        block_stats_t s_src, s_enc, s_mod, s_chn, s_dem, s_mon, s_dec;
        init_stats(&s_src, "Source");
        init_stats(&s_enc, "Encoder");
        init_stats(&s_mod, "Modulator");
        init_stats(&s_chn, "Channel");
        init_stats(&s_dem, "Demodulator");
        init_stats(&s_dec, "Decoder");
        init_stats(&s_mon, "Monitor");
        
    #endif
    int opt;
    // The ":" after a letter means that flag requires an argument
    while ((opt = getopt(argc, argv, "m:M:s:e:K:N:D:")) != -1) {
        switch (opt) {
            case 'm': min_SNR = atof(optarg); break;
            case 'M': max_SNR = atof(optarg); break;
            case 's': step_val = atof(optarg); break;
            case 'e': f_max = (uint32_t)atoi(optarg); break;
            case 'K': K = (uint32_t)atoi(optarg); break;
            case 'N': N = (uint32_t)atoi(optarg); break;
            case 'D': 
                strncpy(decoder_type, optarg, sizeof(decoder_type) - 1); 
                break;
            default:
                fprintf(stderr, "Usage: %s -m min -M max -s step -e errors -K bits -N codeword -D type\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // --- Validation Logic ---

    if (K == 0 || N == 0) {
        fprintf(stderr, "Error: K and N are mandatory and must be greater than 0.\n");
        exit(EXIT_FAILURE);
    }

    if (N % K != 0) {
        fprintf(stderr, "Error: Codeword size N (%u) must be a multiple of info bits K (%u).\n", N, K);
        exit(EXIT_FAILURE);
    }

    if (strcmp(decoder_type, "rep-hard") != 0 && strcmp(decoder_type, "rep-soft") != 0) {
        fprintf(stderr, "Error: Decoder must be 'rep-hard' or 'rep-soft'.\n");
        exit(EXIT_FAILURE);
    }

    printf("Simulation Params: SNR [%.2f : %.2f] Step: %.2f, K: %u, N: %u, Decoder: %s\n", 
            min_SNR, max_SNR, step_val, K, N, decoder_type);
    FILE *csv_file = fopen("../simulations/sim.csv", "w");
    if (csv_file == NULL) {
        fprintf(stderr, "Error opening CSV file!\n");
        return 1;
    }
    fprintf(csv_file, "Eb/N0(dB),Es/N0(dB),Sigma,BE,FE,FN,BER,FER,Time_Total(s),Time_Avg(s),Sim_thr\n");

    float R = (float)K / (float)N;
	float b_s = 1.0f; // BPSK 
	uint8_t U_k[K];
	uint8_t C_N[N];
	int32_t X_N[N];
	float Y_N[N];
	float L_N[N];
	uint8_t V_K[K];
    float Sim_thr=0;
	float ber, fer = 0.0f;
	for (float ebno_db = min_SNR; ebno_db <= max_SNR; ebno_db += step_val) {
        struct timespec start, end;
        
		float esno_db = ebno_db + 10.0f * log10f(R * b_s);
		

		float esno_linear = powf(10.0f, esno_db / 10.0f);

		float sigma = sqrtf(1.0f / (2.0f * esno_linear));
        long  frames = 0;
        uint64_t n_bit_errors =0;
		uint64_t n_frame_errors= 0;
        size_t n_reps = N/K;
        #ifdef ENABLE_STATS
        // Reset stats for each SNR point 
            init_stats(&s_src, "Source"); init_stats(&s_enc, "Encoder");
            init_stats(&s_mod, "Modulator"); init_stats(&s_chn, "Channel");
            init_stats(&s_dem, "Demodulator"); init_stats(&s_mon, "Monitor");
            init_stats(&s_dec, "Decoder");
        #endif

		printf("EbN0: %.2f dB -> EsN0: %.2f dB (Sigma: %.4f)\n", ebno_db, esno_db, sigma);
        clock_gettime(CLOCK_MONOTONIC, &start);
	    if(strcmp(decoder_type, "rep-hard") == 0) {
            while(n_frame_errors<100){				
                MEASURE_BLOCK(s_src, source_generate(U_k, K), K);
                MEASURE_BLOCK(s_enc, codec_repetition_encode(U_k, C_N, K, n_reps), K);
                MEASURE_BLOCK(s_mod, modem_bpsk_modulate(C_N, X_N, N), N);
                MEASURE_BLOCK(s_chn, channel_AWGN_add_noise(X_N, Y_N, N, sigma), N);
                MEASURE_BLOCK(s_dem, modem_BPSK_demodulate(Y_N, L_N, N, sigma), N);
                MEASURE_BLOCK(s_dec, codec_repetition_hard_decode(L_N, V_K, K, n_reps), K);
                MEASURE_BLOCK(s_mon, monitor_check_errors(U_k, V_K, K, &n_bit_errors, &n_frame_errors), K);
                frames++;
            }
        }
        else {
            while(n_frame_errors<100){
                MEASURE_BLOCK(s_src, source_generate(U_k, K), K);
                MEASURE_BLOCK(s_enc, codec_repetition_encode(U_k, C_N, K, n_reps), K);
                MEASURE_BLOCK(s_mod, modem_bpsk_modulate(C_N, X_N, N), N);
                MEASURE_BLOCK(s_chn, channel_AWGN_add_noise(X_N, Y_N, N, sigma), N);
                MEASURE_BLOCK(s_dem, modem_BPSK_demodulate(Y_N, L_N, N, sigma), N);
                MEASURE_BLOCK(s_dec, codec_repetition_soft_decode(L_N, V_K, K, n_reps), K);
                MEASURE_BLOCK(s_mon, monitor_check_errors(U_k, V_K, K, &n_bit_errors, &n_frame_errors), K);
                frames++;
            }
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        fer = (float)n_frame_errors/ frames;
        ber = (float)n_bit_errors / (frames * K);
        double time_spent = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        Sim_thr = frames * K / time_spent;
        double time_per_frame = time_spent / frames;
        printf("Time taken for SNR %.8f dB: %.8f seconds\n", ebno_db, time_spent);
        printf("Average time per frame: %.8f seconds\n", time_per_frame);
        printf("%s: Bit Errors = %lu, Total Frames = %ld Frame Errors = %lu, BER = %.8f, FER = %.8f, Sim_thr = %.8f\n",decoder_type, n_bit_errors,frames, n_frame_errors, ber, fer, Sim_thr);
        #ifdef ENABLE_STATS
        printf("\n--- Block Statistics for SNR %.2f dB ---\n", ebno_db);
        printf("%-12s | %-10s | %-10s | %-10s | %-10s | %-6s\n", "Block", "Avg Lat(s)", "Min", "Max", "Thr(Mbps)", "%%");
        block_stats_t* all_s[] = {&s_src, &s_enc, &s_mod, &s_chn, &s_dem,&s_dec ,&s_mon };
        for(int i=0; i<7; i++) {
            double avg_lat = all_s[i]->total_time / frames;
            double thr = (all_s[i]->total_bits / 1e6) / all_s[i]->total_time;
            double perc = (all_s[i]->total_time / time_spent) * 100.0;
            printf("%-12s | %.2e | %.2e | %.2e | %-10.2f | %.1f%%\n", 
                   all_s[i]->name, avg_lat, all_s[i]->min_latency, all_s[i]->max_latency, thr, perc);
        }
        #endif
        fprintf(csv_file, "%.8f,%.8f,%.8f,%lu,%lu,%lu,%.8f,%.8f,%.8f,%.8f,%.8f\n", ebno_db, esno_db, sigma, n_bit_errors, n_frame_errors,frames, ber, fer, time_spent, time_per_frame, Sim_thr);
        fflush(csv_file);
    }

    return 0;
}
