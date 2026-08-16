#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../inc/auncient_motzkin_engine.h"

/* Pure Virtual Hardware Motzkin Engine Utilities (0 _prover functions) */

uint64_t auncient_motzkin_prime_modpow(uint64_t base, uint64_t exponent) {
    const uint64_t motzkin_prime = 953467954114363ULL;
    uint64_t result = 1ULL;
    base %= motzkin_prime;
    while (exponent > 0) {
        if (exponent & 1) result = (__uint128_t)result * base % motzkin_prime;
        base = (__uint128_t)base * base % motzkin_prime;
        exponent >>= 1;
    }
    return result;
}

uint64_t auncient_motzkin_path_step(uint64_t current_step, uint32_t step_type) {
    const uint64_t motzkin_prime = 953467954114363ULL;
    if (step_type == 0) return (current_step + 1) % motzkin_prime; // Up
    if (step_type == 1) return (current_step + 2) % motzkin_prime; // Flat
    return (current_step > 0) ? (current_step - 1) % motzkin_prime : 0; // Down
}
