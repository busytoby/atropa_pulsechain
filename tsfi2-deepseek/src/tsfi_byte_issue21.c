#include "tsfi_byte_issue21.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Simple 8x8 text character font representation
static const uint8_t font_5x7[128][8] = {
    [' '] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['A'] = {0x18, 0x24, 0x42, 0x7e, 0x42, 0x42, 0x42, 0x00},
    ['B'] = {0x7c, 0x42, 0x42, 0x7c, 0x42, 0x42, 0x7c, 0x00},
    ['C'] = {0x3c, 0x42, 0x40, 0x40, 0x40, 0x42, 0x3c, 0x00},
    ['D'] = {0x78, 0x44, 0x42, 0x42, 0x42, 0x44, 0x78, 0x00},
    ['E'] = {0x7e, 0x40, 0x40, 0x78, 0x40, 0x40, 0x7e, 0x00},
    ['F'] = {0x7e, 0x40, 0x40, 0x78, 0x40, 0x40, 0x40, 0x00},
    // Minimal subset; other ASCII default to solid block if printable
};

static uint8_t get_font_pixel(char character, int col, int row) {
    uint8_t c = (uint8_t)character;
    if (c >= 128) c = 0;
    uint8_t pattern = font_5x7[c][row & 7];
    if (pattern == 0 && c != ' ') {
        // Fallback pattern for non-blank unmapped characters
        return (row > 0 && row < 7 && col > 0 && col < 6) ? 1 : 0;
    }
    return (pattern & (0x80 >> col)) ? 1 : 0;
}

// Apple-II Video Controller Implementation
void tsfi_apple2_video_init(TsfiApple2VideoController *vid) {
    if (!vid) return;
    memset(vid, 0, sizeof(TsfiApple2VideoController));
    memset(vid->vram, ' ', APPLE2_VRAM_SIZE);
}

void tsfi_apple2_video_step(TsfiApple2VideoController *vid, uint8_t *pixel_out, bool *h_sync, bool *v_sync) {
    if (!vid) return;

    // Apple-II NTSC timing approximation:
    // 455 dots per line, 262 lines per frame.
    // Active text area: 40 columns * 7 dots/char = 280 dots wide.
    // Active vertical: 24 rows * 8 scanlines/row = 192 scanlines high.
    
    vid->h_blank = (vid->h_pos >= 280);
    vid->v_blank = (vid->v_pos >= 192);

    *h_sync = (vid->h_pos >= 290 && vid->h_pos < 340);
    *v_sync = (vid->v_pos >= 220 && vid->v_pos < 224);

    if (pixel_out) {
        if (!vid->h_blank && !vid->v_blank) {
            int col = vid->h_pos / 7;
            int dot = vid->h_pos % 7;
            int row = vid->v_pos / 8;
            int line = vid->v_pos % 8;

            int vram_addr = row * APPLE2_COLS + col;
            if (vram_addr < APPLE2_VRAM_SIZE) {
                char ch = (char)vid->vram[vram_addr];
                *pixel_out = get_font_pixel(ch, dot, line);
            } else {
                *pixel_out = 0;
            }
        } else {
            *pixel_out = 0;
        }
    }

    vid->h_pos++;
    if (vid->h_pos >= 455) {
        vid->h_pos = 0;
        vid->v_pos++;
        if (vid->v_pos >= 262) {
            vid->v_pos = 0;
        }
    }
    vid->dot_clock_cycles++;
}

// Joseph Carr DAC implementation
void tsfi_carr_dac_init(TsfiCarrDac *dac, float v_ref, float slew_rate) {
    if (!dac) return;
    dac->v_ref = v_ref;
    dac->v_offset = 0.0f;
    dac->last_v_out = 0.0f;
    dac->slew_rate_v_us = slew_rate;
    dac->r_ladder_ohms = 10000.0f;
}

float tsfi_carr_dac_convert(TsfiCarrDac *dac, uint8_t digital_value, float dt_us) {
    if (!dac) return 0.0f;

    // Ideal voltage calculation
    float target_v = dac->v_ref * ((float)digital_value / 255.0f) + dac->v_offset;

    // Inject Carr's simulated dither (avalanche noise ±0.006V)
    float noise = (((float)rand() / (float)RAND_MAX) * 0.012f) - 0.006f;
    target_v += noise;

    // Slew rate simulation
    float max_delta = dac->slew_rate_v_us * dt_us;
    float current_delta = target_v - dac->last_v_out;

    if (current_delta > max_delta) {
        dac->last_v_out += max_delta;
    } else if (current_delta < -max_delta) {
        dac->last_v_out -= max_delta;
    } else {
        dac->last_v_out = target_v;
    }

    return dac->last_v_out;
}

// Wimble Evolutionary Brain implementation
void tsfi_wimble_brain_init(TsfiWimbleBrain *brain) {
    if (!brain) return;
    brain->generation = 0;
    for (int i = 0; i < 16; i++) {
        for (int w = 0; w < 8; w++) {
            brain->population[i].weights[w] = (((float)rand() / (float)RAND_MAX) * 2.0f) - 1.0f;
        }
        brain->population[i].fitness = -9999.0f;
    }
}

int tsfi_wimble_brain_evaluate(TsfiWimbleBrain *brain, const float *inputs, const float *targets) {
    if (!brain || !inputs || !targets) return -1;

    int best_idx = 0;
    float best_fitness = -999999.0f;

    for (int i = 0; i < 16; i++) {
        // Compute output: simple dot product
        float out = 0.0f;
        for (int w = 0; w < 8; w++) {
            out += brain->population[i].weights[w] * inputs[w];
        }

        // Fitness is negative absolute error (or MSE)
        float diff = out - targets[0];
        brain->population[i].fitness = -(diff * diff);

        if (brain->population[i].fitness > best_fitness) {
            best_fitness = brain->population[i].fitness;
            best_idx = i;
        }
    }
    return best_idx;
}

static int compare_organisms(const void *a, const void *b) {
    float fa = ((TsfiWimbleOrganism *)a)->fitness;
    float fb = ((TsfiWimbleOrganism *)b)->fitness;
    return (fa < fb) ? 1 : ((fa > fb) ? -1 : 0);
}

void tsfi_wimble_brain_breed(TsfiWimbleBrain *brain, const float *targets) {
    if (!brain) return;

    // Sort population by fitness (highest first)
    qsort(brain->population, 16, sizeof(TsfiWimbleOrganism), compare_organisms);

    // Keep top 4 organisms, breed remaining 12
    for (int i = 4; i < 16; i++) {
        int parent1 = rand() % 4;
        int parent2 = rand() % 4;

        for (int w = 0; w < 8; w++) {
            // Crossover
            if (rand() % 2 == 0) {
                brain->population[i].weights[w] = brain->population[parent1].weights[w];
            } else {
                brain->population[i].weights[w] = brain->population[parent2].weights[w];
            }

            // Mutation: 10% chance
            if ((rand() % 100) < 10) {
                float mutation = (((float)rand() / (float)RAND_MAX) * 0.2f) - 0.1f;
                brain->population[i].weights[w] += mutation;
            }
        }
        brain->population[i].fitness = -9999.0f;
    }
    brain->generation++;
    (void)targets; // Suppress unused parameter warning cleanly
}
