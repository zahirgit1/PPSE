#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void channel_AWGN_add_noise(const int32_t *X_N, float *Y_N, size_t N, float sigma) {
    float noise = 0.0f;
    for (size_t i = 0; i < N; i++) {
        float u1 = (float)rand() / RAND_MAX;
        float u2 = (float)rand() / RAND_MAX;

        // Ensure u1 is not zero to avoid log(0)
        if (u1 < 1e-9f) u1 = 1e-9f;

        noise = sqrtf(-2.0f * logf(u1)) * cosf(2.0f * (float)M_PI * u2);


        Y_N[i] = (float)X_N[i] + (noise * sigma );
    }   
}

void channel_AWGN_ctl(const int32_t *X_N, float *Y_N, size_t N, float sigma){

    srand((unsigned int)time(NULL));

    for (size_t i = 0; i < N; i++) {

        float sum =0.0f; 
        int M = 12; // Number of uniform random variables to sum for normal approximation
        for (int j = 0; j < M; j++) {
            sum += (float)rand() / RAND_MAX;
        }
        sum = sum - M / 2.0f; // Center the sum around 0
        //sum = sum / sqrtf(M / 12.0f); // Normalize to have variance 1      
        float noise = sigma * sum; // Approximate normal distribution(if M = 1 we will have sum - 6 
                                                                    //  which is uniform distribution between -0.5 and 0.5)

        Y_N[i] = (float)X_N[i] + noise;
    }

}
