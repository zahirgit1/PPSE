#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>




void monitor_check_errors(const uint8_t *U_K, const uint8_t *V_K, size_t K, uint64_t *n_bit_errors, uint64_t *n_frame_errors){
    bool error =0;
    for(size_t i = 0; i < K; i++){
       
        if(U_K[i] != V_K[i]){
            (*n_bit_errors)++;
            error = 1;// Count only one frame error per frame
        }
    }
    if(error) {
        (*n_frame_errors)++;
        error = 0;}
}