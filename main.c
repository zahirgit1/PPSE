#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include "chaine_com.h"



int src_all_zeros = 0;
int mod_all_ones = 0;
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
    s->min_latency = 1e18; 
    s->max_latency = 0;
    s->total_bits = 0;
    s->name = name;
}

double get_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}// function for better readability 

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
}// Macro to measure block latency and update stats
#else
// If stats disabled, just call the function normally

#define MEASURE_BLOCK(stats, block_call, bits) block_call
#endif

uint32_t qf = 9; // Initialize out of range [0;8] to detect if set
uint32_t qs = 8; // Default value
static struct option long_options[] = {
    {"src-all-zeros", no_argument, 0, 'z'}, // We'll map it to 'z' internally
    {"mod-all-ones", no_argument, 0, 'y'},    // We'll map it to 'y' internally
    {"qf", required_argument, 0, 'f'},
    {"qs", required_argument, 0, 'q'},

    {0, 0, 0, 0}
};
int main(int argc, char *argv[]) {

    float min_SNR = 0.0f, max_SNR = 0.0f, step_val = 1.0f;
    uint32_t f_max = 100, K = 0, N = 0;
    char decoder_type[20] = "rep-hard";
    // Init Stats if enabled
    #ifdef ENABLE_STATS
        block_stats_t s_src, s_enc, s_mod, s_chn, s_dem, s_mon, s_dec, s_qua;
        init_stats(&s_src, "Source");
        init_stats(&s_enc, "Encoder");
        init_stats(&s_mod, "Modulator");
        init_stats(&s_chn, "Channel");
        init_stats(&s_dem, "Demodulator");
        init_stats(&s_dec, "Decoder");
        init_stats(&s_mon, "Monitor");
        init_stats(&s_qua, "Quantizer");
        
    #endif
    int opt;
    int option_index = 0;
    // The ":" after a letter means that flag requires an argument
    while ((opt = getopt_long(argc, argv, "m:M:s:e:K:N:D:", long_options, &option_index)) != -1) {
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
            case 'z':
                src_all_zeros = 1;
                break;
            case 'y':
                mod_all_ones = 1;
                break;
            case 'f':
                qf = (uint32_t)atoi(optarg);
                if (qf > 8) { fprintf(stderr, "Error: qf must be [0;8]\n"); exit(1); }
                break;
            case 'q':
                qs = (uint32_t)atoi(optarg);
                 if (qs < 1 || qs > 8 || qs < qf) { fprintf(stderr, "Error: qs must be [1;8] and greater than or equal to qf\n"); exit(1); }
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

    if (strcmp(decoder_type, "rep-hard") != 0 && strcmp(decoder_type, "rep-soft") != 0 &&
        strcmp(decoder_type, "rep-hard8") != 0 && strcmp(decoder_type, "rep-soft8") != 0) {
        fprintf(stderr, "Error: Decoder must be 'rep-hard', 'rep-soft', 'rep-hard8' or 'rep-soft8'.\n");
        exit(EXIT_FAILURE);
    }

    printf("Simulation Params: SNR [%.2f : %.2f] Step: %.2f, K: %u, N: %u, Decoder: %s\n", 
            min_SNR, max_SNR, step_val, K, N, decoder_type);

    // Open CSV file 
    FILE *csv_file = fopen("../simulations/sim.csv", "w");
    if (csv_file == NULL) {
        fprintf(stderr, "Error opening CSV file!\n");
        return 1;
    }
    //fprintf(csv_file, "Eb/N0(dB),Es/N0(dB),Sigma,BE,FE,FN,BER,FER,Time_Total(s),Time_Avg(s),Sim_thr\n");
    fprintf(csv_file, "Eb/N0(dB),Es/N0(dB),Sigma,BE,FE,FN,BER,FER,Time_Total(s),Time_Avg(s),Sim_thr,Dec_Lat(s),Dec_Thr(Mbps)\n");

    float R = (float)K / (float)N;
	float b_s = 1.0f; // BPSK 
	uint8_t U_k[K];
	uint8_t C_N[N];
	int32_t X_N[N];
	float Y_N[N];
	float L_N[N];
    int8_t L8_N[N];
	uint8_t V_K[K];
    float Sim_thr=0;
	float ber, fer = 0.0f;
	for (float ebno_db = min_SNR; ebno_db <= max_SNR; ebno_db += step_val) {
        struct timespec start, end;
        // Calculate Es/N0 from Eb/N0
		float esno_db = ebno_db + 10.0f * log10f(R * b_s);
		

		float esno_linear = powf(10.0f, esno_db / 10.0f);
        // calculate noise sigma from Es/N0
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
            init_stats(&s_dec, "Decoder"); init_stats(&s_qua, "Quantizer");
        #endif

		printf("EbN0: %.2f dB -> EsN0: %.2f dB (Sigma: %.4f)\n", ebno_db, esno_db, sigma);
        // statring the timer for the snr point
        clock_gettime(CLOCK_MONOTONIC, &start);


            while(n_frame_errors<100){		
                if(src_all_zeros) {
                    MEASURE_BLOCK(s_src, source_generate_all_zeros(U_k, K), K);
                }
                else {		
                    MEASURE_BLOCK(s_src, source_generate(U_k, K), K);
                }
                
                MEASURE_BLOCK(s_enc, codec_repetition_encode(U_k, C_N, K, n_reps), K);
                if(mod_all_ones) {
                    MEASURE_BLOCK(s_mod, modem_bpsk_modulate_all_ones(C_N, X_N, N), N);
                }
                else {  
                    MEASURE_BLOCK(s_mod, modem_bpsk_modulate(C_N, X_N, N), N);
                }
                
                MEASURE_BLOCK(s_chn, channel_AWGN_add_noise(X_N, Y_N, N, sigma), N);
                MEASURE_BLOCK(s_dem, modem_BPSK_demodulate(Y_N, L_N, N, sigma), N);
                if (qf <= 8) {// Only call quantizer if qf is set to a valid value
                    MEASURE_BLOCK(s_qua, quantizer_transform8(L_N, L8_N, N, qs, qf), N);
                }
                if (strcmp(decoder_type, "rep-hard") == 0) {
                    MEASURE_BLOCK(s_dec, codec_repetition_hard_decode(L_N, V_K, K, n_reps), K);
                } else if (strcmp(decoder_type, "rep-soft") == 0) {
                    MEASURE_BLOCK(s_dec, codec_repetition_soft_decode(L_N, V_K, K, n_reps), K);
                }else if (strcmp(decoder_type, "rep-hard8") == 0) {
                    MEASURE_BLOCK(s_dec, codec_repetition_hard_decode8(L8_N, V_K, K, n_reps), K);
                } else if (strcmp(decoder_type, "rep-soft8") == 0) {
                    MEASURE_BLOCK(s_dec, codec_repetition_soft_decode8(L8_N, V_K, K, n_reps), K);
                }
                MEASURE_BLOCK(s_mon, monitor_check_errors(U_k, V_K, K, &n_bit_errors, &n_frame_errors), K);
                frames++;
            }




        // stopping the timer for the snr point
        clock_gettime(CLOCK_MONOTONIC, &end);
        fer = (float)n_frame_errors/ frames;
        ber = (float)n_bit_errors / (frames * K);
        double time_spent = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        Sim_thr = frames * K / time_spent;
        double dec_avg_lat = 0;
        double dec_thr = 0;

        #ifdef ENABLE_STATS
            dec_avg_lat = s_dec.total_time / frames;
            // Throughput = (Total bits K / 1e6) / Total time spent in decoder in mbps 
            if (s_dec.total_time > 0) {
                dec_thr = (s_dec.total_bits / 1e6) / s_dec.total_time;
            }
        #endif
        // Calculate average time per frame
        double time_per_frame = time_spent / frames;
        
        #ifdef ENABLE_STATS
        printf("\n--- Block Statistics for SNR %.2f dB ---\n", ebno_db);
        printf("%-12s | %-10s | %-10s | %-10s | %-10s | %-6s\n", "Block", "Avg Lat(s)", "Min", "Max", "Thr(Mbps)", "%%");
        block_stats_t* all_s[] = {&s_src, &s_enc, &s_mod, &s_chn, &s_dem,&s_dec ,&s_mon, &s_qua};
        for(int i=0; i<8; i++) {
            double avg_lat = all_s[i]->total_time / frames;
            double thr = (all_s[i]->total_bits / 1e6) / all_s[i]->total_time;
            double perc = (all_s[i]->total_time / time_spent) * 100.0;
            printf("%-12s | %.2e | %.2e | %.2e | %-10.2f | %.1f%%\n", 
                   all_s[i]->name, avg_lat, all_s[i]->min_latency, all_s[i]->max_latency, thr, perc);
        }
        printf("---------------------------------------------\n");  
        printf("dec_avg_lat: %.4e seconds, dec_thr: %.4e Mbps\n", dec_avg_lat, dec_thr);
        #endif
        printf("Time taken for SNR %.8f dB: %.8f seconds\n", ebno_db, time_spent);
        printf("Average time per frame: %.4e seconds\n", time_per_frame);
        printf("%s: Bit Errors = %lu, Total Frames = %ld Frame Errors = %lu, BER = %.4e, FER = %.4e, Sim_thr = %.4e\n",decoder_type, n_bit_errors,frames, n_frame_errors, ber, fer, Sim_thr);
        fprintf(csv_file, "%.2f,%.2f,%.4e,%lu,%lu,%lu,%.4e,%.4e,%.4e,%.4e,%.4e,%.4e,%.4e\n", 
                ebno_db,         
                esno_db,         
                sigma,           
                n_bit_errors,    
                n_frame_errors,  
                frames,          
                ber,             
                fer,             
                time_spent,      
                time_per_frame, 
                Sim_thr,         
                dec_avg_lat,     
                dec_thr);        
                fflush(csv_file);
    }

    return 0;
}
