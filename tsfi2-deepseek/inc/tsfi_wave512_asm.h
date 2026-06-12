#ifndef TSFI_WAVE512_ASM_H
#define TSFI_WAVE512_ASM_H

#include "tsfi_wave512.h"

#define TSFI_WAVE_REG_COUNT 16
#define TSFI_WAVE_VAR_COUNT 32
#define TSFI_WAVE_LABEL_COUNT 32
#define TSFI_WAVE_STATEMENT_COUNT 64

typedef struct {
    char name[32];
    int reg_idx;
} Wave512Symbol;

typedef struct {
    char name[32];
    int statement_idx;
} Wave512Label;

typedef struct {
    char label[32];
    char cmd[32];
    char args[256];
} Wave512Statement;

typedef struct {
    wave512 registers[TSFI_WAVE_REG_COUNT];
    char output_buffer[524288];
    int output_pos;

    // Wimble APL Part 2 extension
    Wave512Symbol symbols[TSFI_WAVE_VAR_COUNT];
    int symbol_count;
    Wave512Label labels[TSFI_WAVE_LABEL_COUNT];
    int label_count;
    Wave512Statement statements[TSFI_WAVE_STATEMENT_COUNT];
    int statement_count;
    int pc;
} Wave512State;

void tsfi_wave512_init(Wave512State *state);
void tsfi_wave512_exec(Wave512State *state, const char *code);
int tsfi_wave512_resolve_symbol(Wave512State *state, const char *name, int allocate_new);

#endif // TSFI_WAVE512_ASM_H
