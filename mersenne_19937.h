#ifndef MERSENNE_19937_H
#define MERSENNE_19937_H

#include <stdint.h>
#include <stdalign.h>
#include <arm_neon.h>

// MT19937 Constants as per specification
#define MT_W  32        // Word size (bits)
#define MT_N  624       // State array size (n)
#define MT_M  397       // Shift parameter (m)
#define MT_R  31        // Mask bits (r)

// Twist transformation constant
#define MT_A  0x9908B0DFU  // Matrix A (a)

// Tempering transformation constants
#define MT_U  11           // Tempering shift u
#define MT_D  0xFFFFFFFFU  // Tempering mask d
#define MT_S  7            // Tempering shift s
#define MT_B  0x9D2C5680U  // Tempering mask b
#define MT_T  15           // Tempering shift t
#define MT_C  0xEFC60000U  // Tempering mask c
#define MT_L  18           // Tempering shift l

/**
 * Global state array for Mersenne Twister MT19937
 * Aligned to 16 bytes for NEON operations
 */
extern alignas(16) uint32_t mt_state[MT_N];
extern int mti;

/**
 * Initialize the Mersenne Twister with a seed
 * @param seed The seed value to initialize with
 */
void mt19937_init(uint32_t seed);

/**
 * Twist operation using NEON SIMD (4 lanes)
 * Generates 624 new values from the current state
 * @param state Pointer to the state array
 */
void mt19937_twist_neon(uint32_t *state);

/**
 * Temper and extract 4 sequential random numbers at once using NEON
 * @param state_ptr Pointer to 4 consecutive values in the state array
 * @return Vector of 4 tempered uint32_t values
 */
uint32x4_t mt19937_temper_four(const uint32_t *state_ptr);

/**
 * Get the next random uint32_t value
 * @return A random 32-bit unsigned integer
 */
uint32_t mt19937_next(void);

/**
 * Get 4 random uint32_t values at once (vectorized)
 * @return Vector containing 4 random 32-bit values
 */
uint32x4_t mt19937_next_four(void);

#endif // MERSENNE_19937_H
