#ifndef TSFI_CORE_ROPE_H
#define TSFI_CORE_ROPE_H

#include <stdint.h>

#define CORE_ROPE_WORDS 16
#define CORE_ROPE_BITS 16

typedef struct {
    // rope_matrix[w][b] represents if bit line b threads THROUGH core w (1) or AROUND (0)
    uint8_t rope_matrix[CORE_ROPE_WORDS][CORE_ROPE_BITS];
    float core_flux[CORE_ROPE_WORDS]; // Flux level of the core (-1.0 to 1.0)
} TsfiCoreRopeMemory;

void tsfi_core_rope_init(TsfiCoreRopeMemory *mem, const uint16_t *initial_words);
uint16_t tsfi_core_rope_read(TsfiCoreRopeMemory *mem, uint8_t word_addr, float *out_voltages);

#endif /* TSFI_CORE_ROPE_H */
