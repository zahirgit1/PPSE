#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
int main() {
    size_t N = 100000; // 100,000 samples
    float sigma = 1.0f;
    
    int32_t *X = malloc(N * sizeof(int32_t));
    float *Y = malloc(N * sizeof(float));
    
    // Initialize input with 1s
    for(size_t i=0; i<N; i++) X[i] = 1;

    srand((unsigned int)time(NULL));

    // Test CLT
    clock_t start_clt = clock();
    channel_AWGN_ctl(X, Y, N, sigma);
    clock_t end_clt = clock();
    double time_clt = (double)(end_clt - start_clt) / CLOCKS_PER_SEC;

    // Test Box-Muller
    clock_t start_bm = clock();
    channel_AWGN_add_noise(X, Y, N, sigma);
    clock_t end_bm = clock();
    double time_bm = (double)(end_bm - start_bm) / CLOCKS_PER_SEC;

    printf("Results for N = %zu:\n", N);
    printf("CLT (M=20) Time:  %f seconds\n", time_clt);
    printf("Box-Muller Time:   %f seconds\n", time_bm);

    free(X);
    free(Y);
    return 0;
}