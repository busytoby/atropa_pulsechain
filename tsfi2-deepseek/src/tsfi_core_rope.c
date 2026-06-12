#include "tsfi_core_rope.h"
#include <string.h>

void tsfi_core_rope_init(TsfiCoreRopeMemory *mem, const uint16_t *initial_words) {
    memset(mem, 0, sizeof(TsfiCoreRopeMemory));
    for (int w = 0; w < CORE_ROPE_WORDS; w++) {
        uint16_t val = initial_words ? initial_words[w] : 0;
        for (int b = 0; b < CORE_ROPE_BITS; b++) {
            // If bit b (from LSB) is set, the sense wire threads through the core (represented as 1)
            mem->rope_matrix[w][b] = (val & (1 << b)) ? 1 : 0;
        }
        // Initialize core magnetic flux state to reset level (-1.0)
        mem->core_flux[w] = -1.0f;
    }
}

uint16_t tsfi_core_rope_read(TsfiCoreRopeMemory *mem, uint8_t word_addr, float *out_voltages) {
    if (word_addr >= CORE_ROPE_WORDS) {
        return 0;
    }

    // Simulate the reset-to-set transient flux transition (from -1.0 to 1.0)
    // dPhi/dt is proportional to the difference
    float flux_change = 1.0f - mem->core_flux[word_addr];
    mem->core_flux[word_addr] = 1.0f; // Core is now set

    uint16_t word_out = 0;
    for (int b = 0; b < CORE_ROPE_BITS; b++) {
        float v_ind = 0.0f;
        if (mem->rope_matrix[word_addr][b]) {
            // Sense wire threads through the core: EMF induced (5V scale factor)
            v_ind = flux_change * 2.5f; // flux_change of 2.0 -> 5.0V peak
            word_out |= (1 << b);
        } else {
            // Sense wire bypasses the core: 0V
            v_ind = 0.0f;
        }

        if (out_voltages) {
            out_voltages[b] = v_ind;
        }
    }

    // Set core back to reset state for subsequent reads
    mem->core_flux[word_addr] = -1.0f;

    return word_out;
}
