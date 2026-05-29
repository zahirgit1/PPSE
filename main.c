#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include "chaine_com.h"
#include "mersenne_19937.h"



int src_all_zeros = 0;
int mod_all_ones = 0;

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

#ifdef ENABLE_STATS 

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
int use_neon_i8_mod = 0; // Flag to use int8_t NEON modulator
int use_neon_demod = 0;  // Flag to use NEON demodulator
int use_neon_mon = 0;    // Flag to use NEON monitor
int use_neon_enc = 0;    // Flag to use NEON encoder
int use_muller = 0;      // Flag to use vectorized Box-Muller channel
int use_packed = 0;      // Flag to use packed source with scalar unpack
int use_packed_neon = 0; // Flag to use packed source with NEON unpack
int use_packed_mersenne = 0; // Flag to use packed source with vectorized Mersenne Twister
static struct option long_options[] = {
    {"src-all-zeros", no_argument, 0, 'z'}, // We'll map it to 'z' internally
    {"mod-all-ones", no_argument, 0, 'y'},    // We'll map it to 'y' internally
    {"neonmod", no_argument, 0, 'n'},         // Use int8_t NEON modulator
    {"neondemod", no_argument, 0, 'd'},       // Use NEON demodulator
    {"neonmon", no_argument, 0, 1001},        // Use NEON monitor (long-only)
    {"neonenc", no_argument, 0, 1005},        // Use NEON encoder (long-only)
    {"packed", no_argument, 0, 1002},         // Use packed source with scalar unpack (long-only)
    {"packedneon", no_argument, 0, 1003},     // Use packed source with NEON unpack (long-only)
    {"mersenne", no_argument, 0, 1004}, 
    {"muller", no_argument, 0, 1007},         // Use vectorized Box-Muller channel (long-only)      // Use packed source with vectorized Mersenne Twister (long-only)
    {"qf", required_argument, 0, 'f'},
    {"qs", required_argument, 0, 'q'},

    {0, 0, 0, 0}
};
int main(int argc, char *argv[]) {

    float min_SNR = 0.0f, max_SNR = 0.0f, step_val = 1.0f;
    uint32_t f_max = 100, K = 0, N = 0;
    char decoder_type[20] = "rep-hard";
    
    // Declare stats variables (used for CSV output even if ENABLE_STATS is off)
    block_stats_t s_src,s_unpack ,s_enc, s_mod, s_chn, s_dem, s_mon, s_dec, s_qua ;
    
    #ifdef ENABLE_STATS
        init_stats(&s_src, "Source");
        init_stats(&s_enc, "Encoder");
        init_stats(&s_mod, "Modulator");
        init_stats(&s_chn, "Channel");
        init_stats(&s_dem, "Demodulator");
        init_stats(&s_dec, "Decoder");
        init_stats(&s_mon, "Monitor");
        init_stats(&s_qua, "Quantizer");
        init_stats(&s_unpack, "Unpack");
    #else
        memset(&s_src, 0, sizeof(block_stats_t));
        memset(&s_unpack, 0, sizeof(block_stats_t));
        memset(&s_enc, 0, sizeof(block_stats_t));
        memset(&s_mod, 0, sizeof(block_stats_t));
        memset(&s_chn, 0, sizeof(block_stats_t));
        memset(&s_dem, 0, sizeof(block_stats_t));
        memset(&s_dec, 0, sizeof(block_stats_t));
        memset(&s_mon, 0, sizeof(block_stats_t));
        memset(&s_qua, 0, sizeof(block_stats_t));
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
            case 'n':
                use_neon_i8_mod = 1;
                break;
            case 'd':
                use_neon_demod = 1;
                break;
            case 1001:  // --neonmon (long option only)
                use_neon_mon = 1;
                break;
            case 1002:  // --packed (long option only, scalar unpack)
                use_packed = 1;
                break;
            case 1003:  // --packedneon (long option only, NEON unpack)
                use_packed_neon = 1;
                break;
            case 1004:  // --mersenne (long option only, vectorized Mersenne Twister)
                use_packed_mersenne = 1;
                break;
            case 1005:  // --neonenc (long option only, NEON encoder)
                use_neon_enc = 1;
                break;
            case 1007:  // --muller (long option only, vectorized Box-Muller channel)
                use_muller = 1;
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

    if (use_packed && K % 8 != 0) {
        fprintf(stderr, "Error: When using --packed, K (%u) must be a multiple of 8.\n", K);
        exit(EXIT_FAILURE);
    }

    if (use_packed_neon && K % 8 != 0) {
        fprintf(stderr, "Error: When using --packedneon, K (%u) must be a multiple of 8.\n", K);
        exit(EXIT_FAILURE);
    }
    if (use_packed_mersenne && K % 8 != 0) {
        fprintf(stderr, "Error: When using --mersenne, K (%u) must be a multiple of 8.\n", K);
        exit(EXIT_FAILURE);
    }
    if ((use_packed + use_packed_neon + use_packed_mersenne) > 1) {
        fprintf(stderr, "Error: Cannot use --packed, --packedneon, and --mersenne together. Use only one.\n");
        exit(EXIT_FAILURE);
    }
    if (strcmp(decoder_type, "rep-hard") != 0 && strcmp(decoder_type, "rep-soft") != 0 &&
        strcmp(decoder_type, "rep-hard8") != 0 && strcmp(decoder_type, "rep-soft8") != 0 &&
        strcmp(decoder_type, "rep-hard8-neon") != 0 && strcmp(decoder_type, "rep-soft8-neon") != 0) {
        fprintf(stderr, "Error: Decoder must be 'rep-hard', 'rep-soft', 'rep-hard8', 'rep-soft8', 'rep-hard8-neon' or 'rep-soft8-neon'.\n");
        exit(EXIT_FAILURE);
    }

    printf("Simulation Params: SNR [%.2f : %.2f] Step: %.2f, K: %u, N: %u, Decoder: %s\n", 
            min_SNR, max_SNR, step_val, K, N, decoder_type);

    // Initialize Mersenne Twister PRNG with time-based seed
   // mt19937_init((uint32_t)time(NULL));
    mt19937_init(1000);
    // Open CSV file 
    FILE *csv_file = fopen("../simulations/sim.csv", "w");
    if (csv_file == NULL) {
        fprintf(stderr, "Error opening CSV file!\n");
        return 1;
    }
    //fprintf(csv_file, "Eb/N0(dB),Es/N0(dB),Sigma,BE,FE,FN,BER,FER,Time_Total(s),Time_Avg(s),Sim_thr\n");
    fprintf(csv_file, "Eb/N0(dB),Es/N0(dB),Sigma,BE,FE,FN,BER,FER,Time_Total(s),Time_Avg(s),Sim_thr,Dec_Thr(Mbps),");
    fprintf(csv_file, "Src_Thr(Mbps),Unp_Thr(Mbps),Enc_Thr(Mbps),");
    fprintf(csv_file, "Mod_Thr(Mbps),Chn_Thr(Mbps),Dem_Thr(Mbps),");
    fprintf(csv_file, "Dec_Thr_Stats(Mbps),Mon_Thr(Mbps),Qua_Thr(Mbps)\n");

    float R = (float)K / (float)N;
	float b_s = 1.0f; // BPSK 
	uint8_t U_k[K];
	uint8_t C_N[N];
	int32_t X_N[N];
	int8_t X_N_i8[N];  // For NEON int8_t modulator
	float Y_N[N];
	float L_N[N];
    int8_t L8_N[N];
	uint8_t V_K[K];
	// Packed source buffer (when --packed is used)
	uint8_t U_k_packed[K/8];
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
            init_stats(&s_unpack, "Unpack");
        #endif

		printf("EbN0: %.2f dB -> EsN0: %.2f dB (Sigma: %.4f)\n", ebno_db, esno_db, sigma);
        // statring the timer for the snr point
        clock_gettime(CLOCK_MONOTONIC, &start);


            while(n_frame_errors<100){
                if(mod_all_ones ) {
                    memset(U_k, 0, K); // All zeros at source will lead to all ones after modulation, so we can skip source generation
                }		
                else if(src_all_zeros) {
                    if (use_packed) {
                        MEASURE_BLOCK(s_src, source_generate_packed_all_zeros(U_k_packed, K), K/8);
                        MEASURE_BLOCK(s_unpack, unpack_bits(U_k_packed, U_k, K/8), K);
                    } else if (use_packed_neon) {
                        MEASURE_BLOCK(s_src, source_generate_packed_all_zeros(U_k_packed, K), K/8);
                        MEASURE_BLOCK(s_unpack, unpack_bits_neon(U_k_packed, U_k, K/8), K);
                    } else if (use_packed_mersenne) {
                        MEASURE_BLOCK(s_src, source_generate_packed_all_zeros(U_k_packed, K), K/8);
                        MEASURE_BLOCK(s_unpack, unpack_bits(U_k_packed, U_k, K/8), K);
                    } else {
                        MEASURE_BLOCK(s_src, source_generate_all_zeros(U_k, K), K);
                    }
                }
                else {
                    if (use_packed) {
                        MEASURE_BLOCK(s_src, source_generate_packed(U_k_packed, K), K/8);
                        MEASURE_BLOCK(s_unpack, unpack_bits(U_k_packed, U_k, K/8), K);
                    } else if (use_packed_neon) {
                        MEASURE_BLOCK(s_src, source_generate_packed(U_k_packed, K), K/8);
                        MEASURE_BLOCK(s_unpack, unpack_bits_neon(U_k_packed, U_k, K/8), K);
                    } else if (use_packed_mersenne) {
                        MEASURE_BLOCK(s_src, source_generate_packed_mersenne(U_k_packed, K), K/8);
                        MEASURE_BLOCK(s_unpack, unpack_bits(U_k_packed, U_k, K/8), K);
                    } else {
                        MEASURE_BLOCK(s_src, source_generate(U_k, K), K);
                    }
                }
                
                if (use_neon_enc) {
                    MEASURE_BLOCK(s_enc, codec_repetition_encode_neon(U_k, C_N, K, n_reps), K);
                } else {
                    MEASURE_BLOCK(s_enc, codec_repetition_encode(U_k, C_N, K, n_reps), K);
                }
                if(mod_all_ones) {
                    MEASURE_BLOCK(s_mod, modem_bpsk_modulate_all_ones(C_N, X_N, N), N);
                    if (use_packed_mersenne) {
                        if (use_muller) {
                            MEASURE_BLOCK(s_chn, channel_AWGN_add_noise_mersenne_box_muller(X_N, Y_N, N, sigma), N);
                        } else {
                            MEASURE_BLOCK(s_chn, channel_AWGN_add_noise_mersenne(X_N, Y_N, N, sigma), N);
                        }
                    } else {
                        MEASURE_BLOCK(s_chn, channel_AWGN_add_noise(X_N, Y_N, N, sigma), N);
                    }
                }
                else if(use_neon_i8_mod) {
                    MEASURE_BLOCK(s_mod, modem_bpsk_modulate_neon(C_N, X_N_i8, N), N);
                    // Direct int8_t path - no conversion overhead!
                    if (use_packed_mersenne) {
                        if (use_muller) {
                            MEASURE_BLOCK(s_chn, channel_AWGN_add_noise_i8_mersenne_box_muller(X_N_i8, Y_N, N, sigma), N);
                        } else {
                            MEASURE_BLOCK(s_chn, channel_AWGN_add_noise_i8_mersenne(X_N_i8, Y_N, N, sigma), N);
                        }
                    } else {
                        MEASURE_BLOCK(s_chn, channel_AWGN_add_noise_i8(X_N_i8, Y_N, N, sigma), N);
                    }
                }
                else {  
                    MEASURE_BLOCK(s_mod, modem_bpsk_modulate(C_N, X_N, N), N);
                    if (use_packed_mersenne) {
                        if (use_muller) {
                            MEASURE_BLOCK(s_chn, channel_AWGN_add_noise_mersenne_box_muller(X_N, Y_N, N, sigma), N);
                        } else {
                            MEASURE_BLOCK(s_chn, channel_AWGN_add_noise_mersenne(X_N, Y_N, N, sigma), N);
                        }
                    } else {
                        MEASURE_BLOCK(s_chn, channel_AWGN_add_noise(X_N, Y_N, N, sigma), N);
                    }
                }
                
                if (use_neon_demod) {
                    MEASURE_BLOCK(s_dem, modem_BPSK_demodulate_neon(Y_N, L_N, N, sigma), N);
                } else {
                    MEASURE_BLOCK(s_dem, modem_BPSK_demodulate(Y_N, L_N, N, sigma), N);
                }
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
                } else if (strcmp(decoder_type, "rep-hard8-neon") == 0) {
                    MEASURE_BLOCK(s_dec, codec_repetition_hard_decode8_neon(L8_N, V_K, K, n_reps), K);
                } else if (strcmp(decoder_type, "rep-soft8-neon") == 0) {
                    MEASURE_BLOCK(s_dec, codec_repetition_soft_decode8_neon(L8_N, V_K, K, n_reps), K);
                }
                if (use_neon_mon) {
                    MEASURE_BLOCK(s_mon, monitor_check_errors_neon(U_k, V_K, K, &n_bit_errors, &n_frame_errors), K);
                } else {
                    MEASURE_BLOCK(s_mon, monitor_check_errors(U_k, V_K, K, &n_bit_errors, &n_frame_errors), K);
                }
                frames++;
            }




        // stopping the timer for the snr point
        clock_gettime(CLOCK_MONOTONIC, &end);
        fer = (float)n_frame_errors/ frames;
        ber = (float)n_bit_errors / (frames * K);
        double time_spent = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        Sim_thr = frames * K / time_spent / 1e6; // in Mbps
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
        
        // Initialize all_s array for block statistics
        block_stats_t* all_s[] = {&s_src, &s_unpack, &s_enc, &s_mod, &s_chn, &s_dem, &s_dec, &s_mon, &s_qua};
        
        #ifdef ENABLE_STATS
        printf("\n--- Block Statistics for SNR %.2f dB ---\n", ebno_db);
        printf("%-12s | %-10s | %-10s | %-10s | %-10s | %-6s\n", "Block", "Avg Lat(s)", "Min", "Max", "Thr(Mbps)", "%%");
        for(int i=0; i<9; i++) {
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
        
        // Prepare block statistics for CSV
        double block_lat[9], block_thr[9];
        
        for(int i=0; i<9; i++) {
            block_lat[i] = all_s[i]->total_time / frames;
            block_thr[i] = (all_s[i]->total_bits / 1e6) / (all_s[i]->total_time > 0 ? all_s[i]->total_time : 1e-12);
        }
        
        fprintf(csv_file, "%.2f,%.2f,%.4e,%lu,%lu,%lu,%.4e,%.4e,%.4e,%.4e,%.4e,%.4e,", 
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
                dec_thr);
        
        // Write block throughput only (no latency)
        for(int i=0; i<9; i++) {
            fprintf(csv_file, "%.4e", block_thr[i]);
            if(i < 8) fprintf(csv_file, ",");
        }
        fprintf(csv_file, "\n");
        fflush(csv_file);
    }

    return 0;
}
