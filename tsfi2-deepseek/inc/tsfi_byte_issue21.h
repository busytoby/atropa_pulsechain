#ifndef TSFI_BYTE_ISSUE21_H
#define TSFI_BYTE_ISSUE21_H

#include <stdint.h>
#include <stdbool.h>

#define APPLE2_VRAM_SIZE 1024
#define APPLE2_COLS 40
#define APPLE2_ROWS 24

// Apple-II Video Controller State
typedef struct {
    uint8_t vram[APPLE2_VRAM_SIZE];
    int h_pos;      // Horizontal beam position (0 to 454 dots, including blanking)
    int v_pos;      // Vertical beam position (0 to 261 lines)
    bool h_blank;
    bool v_blank;
    uint32_t dot_clock_cycles;
} TsfiApple2VideoController;

// Joseph Carr R-2R DAC and Op-Amp State
typedef struct {
    float v_ref;
    float v_offset;
    float last_v_out;
    float slew_rate_v_us; // Slew rate in Volts per microsecond
    float r_ladder_ohms;
} TsfiCarrDac;

// Wimble Evolutionary Brain Structure
typedef struct {
    float weights[8];
    float fitness;
} TsfiWimbleOrganism;

typedef struct {
    TsfiWimbleOrganism population[16];
    int generation;
} TsfiWimbleBrain;

// --- API Declarations ---

// Video Controller
void tsfi_apple2_video_init(TsfiApple2VideoController *vid);
void tsfi_apple2_video_step(TsfiApple2VideoController *vid, uint8_t *pixel_out, bool *h_sync, bool *v_sync);

// Carr DAC
void tsfi_carr_dac_init(TsfiCarrDac *dac, float v_ref, float slew_rate);
float tsfi_carr_dac_convert(TsfiCarrDac *dac, uint8_t digital_value, float dt_us);

// Wimble Evolutionary Brain
void tsfi_wimble_brain_init(TsfiWimbleBrain *brain);
void tsfi_wimble_brain_breed(TsfiWimbleBrain *brain, const float *targets);
int tsfi_wimble_brain_evaluate(TsfiWimbleBrain *brain, const float *inputs, const float *targets);

#endif // TSFI_BYTE_ISSUE21_H
