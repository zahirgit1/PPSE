#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <arm_neon.h>
#include "mersenne_19937.h"


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Precomputed reciprocal of UINT32_MAX for fast normalization: 1.0 / 4294967295
#define UINT32_MAX_RECIP 2.328306436538696289063e-10f

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

/**
 * AWGN channel with int8_t input
 * Maps int8_t symbols (±1) to float output with Gaussian noise
 * Functionally identical to int32_t version but accepts smaller input type
 */
void channel_AWGN_add_noise_i8(const int8_t *X_N, float *Y_N, size_t N, float sigma) {
    float noise = 0.0f;
    for (size_t i = 0; i < N; i++) {
        float u1 = (float)rand() / RAND_MAX;
        float u2 = (float)rand() / RAND_MAX;

        // Ensure u1 is not zero to avoid log(0)
        if (u1 < 1e-9f) u1 = 1e-9f;

        noise = sqrtf(-2.0f * logf(u1)) * cosf(2.0f * (float)M_PI * u2);

        Y_N[i] = (float)X_N[i] + (noise * sigma);
    }   
}

/**
 * AWGN channel with Mersenne Twister PRNG
 * Uses 4-lane SIMD vectorized MT19937 for superior performance
 * Box-Muller transform for Gaussian distribution
 * Processes 2 samples per mt19937_next_four() call
 */
void channel_AWGN_add_noise_mersenne(const int32_t *X_N, float *Y_N, size_t N, float sigma) {
    // Align to 8-byte boundary (process pairs of samples)
    size_t i = 0;
    
    // Vectorized loop: process 2 samples at a time using 4 random numbers
    for (; i + 1 < N; i += 2) {
        // Get 4 random uint32_t values at once (4-lane SIMD)
        uint32x4_t rands = mt19937_next_four();
        
        // Extract individual uint32_t values from the vector
        uint32_t r1 = vgetq_lane_u32(rands, 0);
        uint32_t r2 = vgetq_lane_u32(rands, 1);
        uint32_t r3 = vgetq_lane_u32(rands, 2);
        uint32_t r4 = vgetq_lane_u32(rands, 3);
        
        // Convert to normalized floats [0, 1)
        float u1 = (float)r1 / (float)UINT32_MAX;
        float u2 = (float)r2 / (float)UINT32_MAX;
        float u3 = (float)r3 / (float)UINT32_MAX;
        float u4 = (float)r4 / (float)UINT32_MAX;
        
        // Ensure u1 and u3 are not zero to avoid log(0)
        if (u1 < 1e-9f) u1 = 1e-9f;
        if (u3 < 1e-9f) u3 = 1e-9f;
        
        float noise1 = sqrtf(-2.0f * logf(u1)) * cosf(2.0f * (float)M_PI * u2);
        float noise2 = sqrtf(-2.0f * logf(u1)) * sinf(2.0f * (float)M_PI * u2);
        Y_N[i] = (float)X_N[i] + (noise1 * sigma);
        Y_N[i + 1] = (float)X_N[i + 1] + (noise2 * sigma);
        
        // Box-Muller transform for second sample (i+1)
        float noise3 = sqrtf(-2.0f * logf(u3)) * cosf(2.0f * (float)M_PI * u4);
        float noise4 = sqrtf(-2.0f * logf(u3)) * sinf(2.0f * (float)M_PI * u4);
        Y_N[i + 2] = (float)X_N[i + 2] + (noise3 * sigma);
        Y_N[i + 3] = (float)X_N[i + 3] + (noise4 * sigma);
    }
    
}



/**
 * AWGN channel with Mersenne Twister PRNG and int8_t input
 * Uses 4-lane SIMD vectorized MT19937 for superior performance
 * Box-Muller transform for Gaussian distribution
 * Processes 2 samples per mt19937_next_four() call
 */
void channel_AWGN_add_noise_i8_mersenne(const int8_t *X_N, float *Y_N, size_t N, float sigma) {
    // Align to 8-byte boundary (process pairs of samples)
    size_t i = 0;
    
    // Vectorized loop: process 2 samples at a time using 4 random numbers
    for (; i + 1 < N; i += 4) {
        // Get 4 random uint32_t values at once (4-lane SIMD)
        uint32x4_t rands = mt19937_next_four();
        
        // Extract individual uint32_t values from the vector
        uint32_t r1 = vgetq_lane_u32(rands, 0);
        uint32_t r2 = vgetq_lane_u32(rands, 1);
        uint32_t r3 = vgetq_lane_u32(rands, 2);
        uint32_t r4 = vgetq_lane_u32(rands, 3);
        
        // Convert to normalized floats [0, 1)
        float u1 = (float)r1 / (float)UINT32_MAX;
        float u2 = (float)r2 / (float)UINT32_MAX;
        float u3 = (float)r3 / (float)UINT32_MAX;
        float u4 = (float)r4 / (float)UINT32_MAX;
        
        // Ensure u1 and u3 are not zero to avoid log(0)
        if (u1 < 1e-9f) u1 = 1e-9f;
        if (u3 < 1e-9f) u3 = 1e-9f;
        
        // Box-Muller transform for first sample (i)
        float noise1 = sqrtf(-2.0f * logf(u1)) * cosf(2.0f * (float)M_PI * u2);
        float noise2 = sqrtf(-2.0f * logf(u1)) * sinf(2.0f * (float)M_PI * u2);
        Y_N[i] = (float)X_N[i] + (noise1 * sigma);
        Y_N[i + 1] = (float)X_N[i + 1] + (noise2 * sigma);
        
        // Box-Muller transform for second sample (i+1)
        float noise3 = sqrtf(-2.0f * logf(u3)) * cosf(2.0f * (float)M_PI * u4);
        float noise4 = sqrtf(-2.0f * logf(u3)) * sinf(2.0f * (float)M_PI * u4);
        Y_N[i + 2] = (float)X_N[i + 2] + (noise3 * sigma);
        Y_N[i + 3] = (float)X_N[i + 3] + (noise4 * sigma);
    }
    

}
/**
 * AWGN channel with Mersenne Twister + Box-Muller (vectorized NEON optimized)
 * Fully vectorized: 8 random values → 8 noise samples per iteration using cos+sin
 * Processes 8 samples at a time using two mt19937_next_four() calls
 * For each pair (u1, u2): Z0 = √(-2ln(u1)) × cos(2πu2), Z1 = √(-2ln(u1)) × sin(2πu2)
 * int32_t version
 */
void channel_AWGN_add_noise_mersenne_box_muller(const int32_t *X_N, float *Y_N, size_t N, float sigma) {
    float32x4_t inv_max_vec = vmovq_n_f32(UINT32_MAX_RECIP);
    float32x4_t min_threshold = vmovq_n_f32(1e-9f);
    float32x4_t sigma_vec_splat = vmovq_n_f32(sigma);
    float sigma_vec = sigma;
    
    size_t i = 0;
    
    // Process 8 samples at a time (using 8 random numbers from 2 mt19937_next_four() calls)
    for (; i + 7 < N; i += 8) {
        // Get 8 random uint32 values
        uint32x4_t rands1 = mt19937_next_four();  // u1, u2, u3, u4
        uint32x4_t rands2 = mt19937_next_four();  // u5, u6, u7, u8
        
        // Convert all 8 to normalized floats [0, 1)
        float32x4_t u_vec1 = vcvtq_f32_u32(rands1);
        u_vec1 = vmulq_f32(u_vec1, inv_max_vec);
        u_vec1 = vmaxq_f32(u_vec1, min_threshold);
        
        float32x4_t u_vec2 = vcvtq_f32_u32(rands2);
        u_vec2 = vmulq_f32(u_vec2, inv_max_vec);
        u_vec2 = vmaxq_f32(u_vec2, min_threshold);
        
        // Extract all 8 uniform random values
        float u1 = vgetq_lane_f32(u_vec1, 0);
        float u2 = vgetq_lane_f32(u_vec1, 1);
        float u3 = vgetq_lane_f32(u_vec1, 2);
        float u4 = vgetq_lane_f32(u_vec1, 3);
        float u5 = vgetq_lane_f32(u_vec2, 0);
        float u6 = vgetq_lane_f32(u_vec2, 1);
        float u7 = vgetq_lane_f32(u_vec2, 2);
        float u8 = vgetq_lane_f32(u_vec2, 3);
        
        // Apply Box-Muller transform to all 4 pairs, using both cos and sin for 8 values
        float sqrt_term1 = sqrtf(-2.0f * logf(u1));
        float sqrt_term2 = sqrtf(-2.0f * logf(u3));
        float sqrt_term3 = sqrtf(-2.0f * logf(u5));
        float sqrt_term4 = sqrtf(-2.0f * logf(u7));
        
        float theta1 = 2.0f * (float)M_PI * u2;
        float theta2 = 2.0f * (float)M_PI * u4;
        float theta3 = 2.0f * (float)M_PI * u6;
        float theta4 = 2.0f * (float)M_PI * u8;
        
        // Generate 8 noise values using cos and sin
        float noise1 = sqrt_term1 * cosf(theta1);
        float noise2 = sqrt_term1 * sinf(theta1);
        float noise3 = sqrt_term2 * cosf(theta2);
        float noise4 = sqrt_term2 * sinf(theta2);
        float noise5 = sqrt_term3 * cosf(theta3);
        float noise6 = sqrt_term3 * sinf(theta3);
        float noise7 = sqrt_term4 * cosf(theta4);
        float noise8 = sqrt_term4 * sinf(theta4);
        
        // Create first result vector (samples 0-3)
        float32x4_t noise_vec1 = vmovq_n_f32(0.0f);
        noise_vec1 = vsetq_lane_f32(noise1, noise_vec1, 0);
        noise_vec1 = vsetq_lane_f32(noise2, noise_vec1, 1);
        noise_vec1 = vsetq_lane_f32(noise3, noise_vec1, 2);
        noise_vec1 = vsetq_lane_f32(noise4, noise_vec1, 3);
        
        // Create second result vector (samples 4-7)
        float32x4_t noise_vec2 = vmovq_n_f32(0.0f);
        noise_vec2 = vsetq_lane_f32(noise5, noise_vec2, 0);
        noise_vec2 = vsetq_lane_f32(noise6, noise_vec2, 1);
        noise_vec2 = vsetq_lane_f32(noise7, noise_vec2, 2);
        noise_vec2 = vsetq_lane_f32(noise8, noise_vec2, 3);
        
        // Scale by sigma
        float32x4_t sigma_noise1 = vmulq_f32(noise_vec1, sigma_vec_splat);
        float32x4_t sigma_noise2 = vmulq_f32(noise_vec2, sigma_vec_splat);
        
        // Load input samples (first 4)
        int32x4_t x_int1 = vld1q_s32(&X_N[i]);
        float32x4_t x_float1 = vcvtq_f32_s32(x_int1);
        
        // Load input samples (second 4)
        int32x4_t x_int2 = vld1q_s32(&X_N[i + 4]);
        float32x4_t x_float2 = vcvtq_f32_s32(x_int2);
        //these are used to convert from int8 to float 32
        // Add noise to input
        float32x4_t y_vec1 = vaddq_f32(x_float1, sigma_noise1);
        float32x4_t y_vec2 = vaddq_f32(x_float2, sigma_noise2);
        
        // Store results
        vst1q_f32(&Y_N[i], y_vec1);
        vst1q_f32(&Y_N[i + 4], y_vec2);
    }
    
}

/**
 * AWGN channel with Mersenne Twister + Box-Muller (vectorized NEON optimized)
 * Fully vectorized: 8 random values → 8 noise samples per iteration using cos+sin
 * Processes 8 samples at a time using two mt19937_next_four() calls
 * For each pair (u1, u2): Z0 = √(-2ln(u1)) × cos(2πu2), Z1 = √(-2ln(u1)) × sin(2πu2)
 * int8_t version
 */
void channel_AWGN_add_noise_i8_mersenne_box_muller(const int8_t *X_N, float *Y_N, size_t N, float sigma) {
    float32x4_t inv_max_vec = vmovq_n_f32(UINT32_MAX_RECIP);
    float32x4_t min_threshold = vmovq_n_f32(1e-9f);
    float32x4_t sigma_vec_splat = vmovq_n_f32(sigma);
    float sigma_vec = sigma;
    
    size_t i = 0;
    
    // Process 8 samples at a time (using 8 random numbers from 2 mt19937_next_four() calls)
    for (; i + 7 < N; i += 8) {
        // Get 8 random uint32 values
        uint32x4_t rands1 = mt19937_next_four();  // u1, u2, u3, u4
        uint32x4_t rands2 = mt19937_next_four();  // u5, u6, u7, u8
        
        // Convert all 8 to normalized floats [0, 1)
        float32x4_t u_vec1 = vcvtq_f32_u32(rands1);
        u_vec1 = vmulq_f32(u_vec1, inv_max_vec);
        u_vec1 = vmaxq_f32(u_vec1, min_threshold);
        
        float32x4_t u_vec2 = vcvtq_f32_u32(rands2);
        u_vec2 = vmulq_f32(u_vec2, inv_max_vec);
        u_vec2 = vmaxq_f32(u_vec2, min_threshold);
        
        // Extract all 8 uniform random values
        float u1 = vgetq_lane_f32(u_vec1, 0);
        float u2 = vgetq_lane_f32(u_vec1, 1);
        float u3 = vgetq_lane_f32(u_vec1, 2);
        float u4 = vgetq_lane_f32(u_vec1, 3);
        float u5 = vgetq_lane_f32(u_vec2, 0);
        float u6 = vgetq_lane_f32(u_vec2, 1);
        float u7 = vgetq_lane_f32(u_vec2, 2);
        float u8 = vgetq_lane_f32(u_vec2, 3);
        
        // Apply Box-Muller transform to all 4 pairs, using both cos and sin for 8 values
        float sqrt_term1 = sqrtf(-2.0f * logf(u1));
        float sqrt_term2 = sqrtf(-2.0f * logf(u3));
        float sqrt_term3 = sqrtf(-2.0f * logf(u5));
        float sqrt_term4 = sqrtf(-2.0f * logf(u7));
        
        float theta1 = 2.0f * (float)M_PI * u2;
        float theta2 = 2.0f * (float)M_PI * u4;
        float theta3 = 2.0f * (float)M_PI * u6;
        float theta4 = 2.0f * (float)M_PI * u8;
        
        // Generate 8 noise values using cos and sin
        float noise1 = sqrt_term1 * cosf(theta1);
        float noise2 = sqrt_term1 * sinf(theta1);
        float noise3 = sqrt_term2 * cosf(theta2);
        float noise4 = sqrt_term2 * sinf(theta2);
        float noise5 = sqrt_term3 * cosf(theta3);
        float noise6 = sqrt_term3 * sinf(theta3);
        float noise7 = sqrt_term4 * cosf(theta4);
        float noise8 = sqrt_term4 * sinf(theta4);
        
        // Create first result vector (samples 0-3)
        float32x4_t noise_vec1 = vmovq_n_f32(0.0f);
        noise_vec1 = vsetq_lane_f32(noise1, noise_vec1, 0);
        noise_vec1 = vsetq_lane_f32(noise2, noise_vec1, 1);
        noise_vec1 = vsetq_lane_f32(noise3, noise_vec1, 2);
        noise_vec1 = vsetq_lane_f32(noise4, noise_vec1, 3);
        
        // Create second result vector (samples 4-7)
        float32x4_t noise_vec2 = vmovq_n_f32(0.0f);
        noise_vec2 = vsetq_lane_f32(noise5, noise_vec2, 0);
        noise_vec2 = vsetq_lane_f32(noise6, noise_vec2, 1);
        noise_vec2 = vsetq_lane_f32(noise7, noise_vec2, 2);
        noise_vec2 = vsetq_lane_f32(noise8, noise_vec2, 3);
        
        // Scale by sigma
        float32x4_t sigma_noise1 = vmulq_f32(noise_vec1, sigma_vec_splat);
        float32x4_t sigma_noise2 = vmulq_f32(noise_vec2, sigma_vec_splat);
        
        // Load input samples (first 4)
        int8x8_t x_int8 = vld1_s8(&X_N[i]);
        int16x8_t x_int16 = vmovl_s8(x_int8);  // Extend to int16
        int32x4_t x_int32_lo = vmovl_s16(vget_low_s16(x_int16));   // Get low 4
        float32x4_t x_float1 = vcvtq_f32_s32(x_int32_lo);
        
        // Load input samples (second 4)
        int32x4_t x_int32_hi = vmovl_s16(vget_high_s16(x_int16));  // Get high 4
        float32x4_t x_float2 = vcvtq_f32_s32(x_int32_hi);
        //these are used to convert from int8 to float 32
        
        // Add noise to input
        float32x4_t y_vec1 = vaddq_f32(x_float1, sigma_noise1);
        float32x4_t y_vec2 = vaddq_f32(x_float2, sigma_noise2);
        
        // Store results
        vst1q_f32(&Y_N[i], y_vec1);
        vst1q_f32(&Y_N[i + 4], y_vec2);
    }
    

}

