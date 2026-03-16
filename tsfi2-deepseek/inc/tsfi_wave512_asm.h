#ifndef TSFI_WAVE512_ASM_H
#define TSFI_WAVE512_ASM_H

#include "tsfi_wave512.h"

#define TSFI_WAVE_REG_COUNT 16

typedef struct {
    wave512 registers[TSFI_WAVE_REG_COUNT];
    char output_buffer[4096];
    int output_pos;
} Wave512State;

void tsfi_wave512_init(Wave512State *state);
void tsfi_wave512_exec(Wave512State *state, const char *code);

#endif // TSFI_WAVE512_ASM_H
