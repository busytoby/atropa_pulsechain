#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define GRID_SIZE 4

// A single processing core on the wafer-scale engine
typedef struct {
    uint32_t core_id;
    double target_resonance_frequency;
    bool detected_active;
} wse_core_t;

// The Wafer-Scale Engine (WSE-3) structure
typedef struct {
    wse_core_t grid[GRID_SIZE][GRID_SIZE];
    bool swarmx_active_sync;
} cs3_wse_t;

// Initialize the WSE-3 core resonance grid
void init_cs3_wafer(cs3_wse_t *wse) {
    wse->swarmx_active_sync = false;
    uint32_t id = 0;
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            wse->grid[y][x].core_id = id++;
            // Map grid cells to different formant frequencies (e.g. 200Hz to 3000Hz bands)
            wse->grid[y][x].target_resonance_frequency = 200.0 + (id * 175.0);
            wse->grid[y][x].detected_active = false;
        }
    }
}

// Scan the input wave across the wafer-scale grid to locate Transfluxor words
bool scan_wse_for_transfluxor(cs3_wse_t *wse, double input_freq, double input_amp) {
    // 1. Detect Wheeler Jump (high amplitude transient spike triggers global SwarmX sync)
    if (input_amp > 4.5) {
        wse->swarmx_active_sync = true;
        printf("   [SWARMX SYNC] Wheeler Jump transient detected (%.1fV)! Wafer sync enabled.\n", input_amp);
    }

    if (!wse->swarmx_active_sync) {
        return false;
    }

    // 2. Parallel Resonant Frequency Search across WSE grid cells
    bool word_resolved = false;
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            wse_core_t *core = &wse->grid[y][x];
            // If the input frequency falls within the core's resonance band
            double diff = fabs(core->target_resonance_frequency - input_freq);
            if (diff < 100.0) {
                core->detected_active = true;
                word_resolved = true;
                printf("   [WSE-3 CORE %2u] Resonating at %.1fHz (Target: %.1fHz)\n",
                       core->core_id, input_freq, core->target_resonance_frequency);
            } else {
                core->detected_active = false;
            }
        }
    }

    return word_resolved;
}

int main(void) {
    printf("=============================================================\n");
    printf("CEREBRAS CS-3 WAFER-SCALE TRANSFLUXOR SCANNER\n");
    printf("=============================================================\n");

    cs3_wse_t wse;
    init_cs3_wafer(&wse);

    // 1. Send normal sound without Wheeler Jump: WSE should ignore it (no SwarmX sync)
    printf("[TEST] Sending carrier frequency without Wheeler Jump...\n");
    bool match = scan_wse_for_transfluxor(&wse, 550.0, 1.0);
    assert(match == false);
    assert(wse.swarmx_active_sync == false);
    printf("   ✓ Input ignored (no Wheeler Jump transient trigger).\n");

    // 2. Send Wheeler Jump: WSE should trigger SwarmX sync
    printf("[TEST] Sending Wheeler Jump setting pulse...\n");
    match = scan_wse_for_transfluxor(&wse, 550.0, 5.0);
    assert(wse.swarmx_active_sync == true);
    // 550Hz should resonate with Core 2 (200 + 3 * 175 = 725, diff is 175 -> wait, let's check values:
    // id starts at 0, increments before calculation.
    // Core 0: id=1, freq=375Hz
    // Core 1: id=2, freq=550Hz -> diff is 0.0 < 100.0, resonates!)
    assert(match == true);
    assert(wse.grid[0][1].detected_active == true);
    printf("   ✓ Transfluxor word successfully detected and resolved on Core 1.\n");

    printf("=============================================================\n");
    printf("CS-3 WAFER-SCALE SCANNER TESTS PASSED\n");
    printf("=============================================================\n");

    return 0;
}
