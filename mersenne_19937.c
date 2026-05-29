#include "mersenne_19937.h"


#define MT_INIT_MULT 1812433253U
#define MT_INIT_SHIFT 30

/**
 * Global state array and index tracker for Mersenne Twister MT19937
 */
alignas(16) uint32_t mt_state[MT_N];
int mti = MT_N;  // Set to MT_N so the very first request triggers mt19937_twist_neon()

/**
 * Initialize the Mersenne Twister with a seed
 * Fills the state array with initial values derived from the seed
 * @param seed The seed value to initialize with
 */
void mt19937_init(uint32_t seed) {
    mt_state[0] = seed;
    
    for (mti = 1; mti < MT_N; ++mti) {
        // x[i] = 1812433253 * (x[i-1] ^ (x[i-1] >> 30)) + i
        mt_state[mti] = (MT_INIT_MULT * (mt_state[mti - 1] ^ (mt_state[mti - 1] >> MT_INIT_SHIFT)) + mti);
    }
    
    //After filling the array, mti will equal 624.

}

/**
 * Temper and extract 4 sequential random numbers at once using NEON
 * @param state_ptr Pointer to 4 consecutive values in the state array
 * @return Vector of 4 tempered uint32_t values
 */
uint32x4_t mt19937_temper_four(const uint32_t *state_ptr) {
    uint32x4_t x = vld1q_u32(state_ptr);
    
    // y = x ^ (x >> u)
    uint32x4_t y = veorq_u32(x, vshrq_n_u32(x, MT_U));
    
    // y = y ^ ((y << s) & b)
    uint32x4_t mask_b = vdupq_n_u32(MT_B);
    y = veorq_u32(y, vandq_u32(vshlq_n_u32(y, MT_S), mask_b));
    
    // y = y ^ ((y << t) & c)
    uint32x4_t mask_c = vdupq_n_u32(MT_C);
    y = veorq_u32(y, vandq_u32(vshlq_n_u32(y, MT_T), mask_c));
    
    // y = y ^ (y >> l)
    y = veorq_u32(y, vshrq_n_u32(y, MT_L));
    
    return y;  // Contains 4 perfectly sequential standard MT19937 outputs
}

/**
 * Twist operation using NEON SIMD (4 lanes)
 * Generates 624 new values from the current state
 * This function should be called whenever mti >= N
 * @param state Pointer to the state array
 */
void mt19937_twist_neon(uint32_t *state) {
    int k = 0;
    
    // Constant vector masks
    // Upper mask: keeps the most significant bit (2^31)
    uint32x4_t UPPER_MASK = vdupq_n_u32(0x80000000U);
    // Lower mask: keeps the lower 31 bits
    uint32x4_t LOWER_MASK = vdupq_n_u32(0x7FFFFFFFU);
    // Matrix A constant
    uint32x4_t MATRIX_A   = vdupq_n_u32(MT_A);
    uint32x4_t ONE        = vdupq_n_u32(1);

    // Block 1: k from 0 to MT_N - MT_M - 1 (0 to 226)
    // Safe to load x[k], x[k+1], and x[k+MT_M] without wrap-around
    for (; k < MT_N - MT_M; k += 4) {
        uint32x4_t x_k   = vld1q_u32(&state[k]);
        uint32x4_t x_k1  = vld1q_u32(&state[k + 1]);
        uint32x4_t x_km  = vld1q_u32(&state[k + MT_M]);

        // u = (x[k] & 0x80000000) | (x[k+1] & 0x7FFFFFFF)
        uint32x4_t u = vorrq_u32(vandq_u32(x_k, UPPER_MASK), vandq_u32(x_k1, LOWER_MASK));

        // uA = u >> 1
        uint32x4_t uA = vshrq_n_u32(u, 1);

        // Vectorized conditional: if (u & 1) uA ^= MATRIX_A
        // Create a mask where all bits are set if LSB is 1
        uint32x4_t is_odd = vandq_u32(u, ONE);
        // Convert to mask: replicate bit across all 32 bits (cmpeq with is_odd gives -1 for true)
        uint32x4_t cmp_result = vceqq_u32(is_odd, ONE);
       
        uA = veorq_u32(uA, vandq_u32(cmp_result, MATRIX_A));
        //this operations avoids using if statements and 
        // branching The cmp_result will be 0xFFFFFFFF for lanes
        //  where u is odd, and 0x00000000 where u is even. By ANDing this with MATRIX_A, 
        // we get either MATRIX_A or 0, which we then XOR with uA to conditionally apply the transformation.
        // state[k] = state[k+M] ^ uA
        vst1q_u32(&state[k], veorq_u32(x_km, uA));
    }

    // Block 2: k from 227 to MT_N - 2 (227 to 622)
    // x[k+MT_M] wraps around to the beginning of the array: (k + MT_M) - MT_N
    for (; k < MT_N - 4; k += 4) {
        uint32x4_t x_k   = vld1q_u32(&state[k]);
        uint32x4_t x_k1  = vld1q_u32(&state[k + 1]);
        uint32x4_t x_km  = vld1q_u32(&state[k + MT_M - MT_N]);  // Wrapped index

        uint32x4_t u = vorrq_u32(vandq_u32(x_k, UPPER_MASK), vandq_u32(x_k1, LOWER_MASK));
        uint32x4_t uA = vshrq_n_u32(u, 1);
        
        uint32x4_t is_odd = vandq_u32(u, ONE);
        uint32x4_t cmp_result = vceqq_u32(is_odd, ONE);
        uA = veorq_u32(uA, vandq_u32(cmp_result, MATRIX_A));

        vst1q_u32(&state[k], veorq_u32(x_km, uA));
    }

    // Block 3: The Boundary Final Edge (Elements 620, 621, 622, 623)
    // x[k+1] for the very last element (623) must wrap around to state[0]
    // We handle the last 4 elements via scalar execution to safely manage the edge wrap-around.
    for (; k < MT_N; ++k) {
        uint32_t u = (state[k] & 0x80000000U) | (state[(k + 1) % MT_N] & 0x7FFFFFFFU);
        uint32_t uA = u >> 1;
        if (u & 1) {
            uA ^= MT_A;
        }
        state[k] = state[(k + MT_M) % MT_N] ^ uA;
    }
}

/**
 * Get the next random uint32_t value
 * Automatically triggers twist when the state is exhausted
 * @return A random 32-bit unsigned integer
 */
uint32_t mt19937_next(void) {
    if (mti >= MT_N) {
        mt19937_twist_neon(mt_state);
        mti = 0;
    }
    
    uint32_t y = mt_state[mti];
    
    // Tempering transformation
    y ^= (y >> MT_U);
    y ^= (y << MT_S) & MT_B;
    y ^= (y << MT_T) & MT_C;
    y ^= (y >> MT_L);
    
    mti++;
    return y;
}

/**
 * Get 4 random uint32_t values at once (vectorized)
 * Automatically triggers twist when the state is exhausted
 * @return Vector containing 4 random 32-bit values
 */
uint32x4_t mt19937_next_four(void) {
    if (mti >= MT_N - 4) {
        mt19937_twist_neon(mt_state);
        mti = 0;
    }
    
    uint32x4_t result = mt19937_temper_four(&mt_state[mti]);
    mti += 4;
    
    return result;
}
