#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include "chaine_com.h"

void quantizer_transform8(const float *L_N, int8_t *L8_N, size_t N, size_t s, size_t f) {
    // Calculate the range for s bits (signed)
    // For s=8, min is -128, max is 127
    int8_t max_val = (int8_t)((1 << (s - 1)) - 1);
    int8_t min_val = (int8_t)(-(1 << (s - 1)));
    
    float factor = (float)(1 << f);

    for (size_t i = 0; i < N; i++) {
        // Scale the floating point value
        float scaled = L_N[i] * factor;
        
        // Round to nearest integer
        int32_t rounded = (int32_t)roundf(scaled);

        // Preserve sign - don't let rounding flip it
        if (L_N[i] >= 0.0f && rounded < 0) {
            rounded = 0;  // Don't go negative if input was positive
        } else if (L_N[i] < 0.0f && rounded >= 0) {
            rounded = -1;  // Don't go positive if input was negative
        }

        // Saturation (Clamping) logic
        if (rounded > max_val) rounded = max_val;
        else if (rounded < min_val) rounded = min_val;

        L8_N[i] = (int8_t)rounded;
    }
}