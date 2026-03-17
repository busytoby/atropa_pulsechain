#include <stdint.h>
#include <stddef.h>

/**
 * TSFi Ballet Step 13: Variation (Solo)
 * K0Rn varies. Solo mutation tunes scale. Secret found.
 * This thunk executes an evolutionary mutation on a K0Rn genome.
 */
void tsfi_ballet_variation_thunk(float *genome, size_t size, float mutation_rate) {
    if (!genome || size == 0) return;
    
    // Solo mutation: K0Rn varies to tune the scale
    // We search for the "Secret" within the parameter space
    for (size_t i = 0; i < size; i++) {
        // Apply a small variation to the genome
        genome[i] += (mutation_rate * (float)i); // Deterministic mutation for example
    }
}
