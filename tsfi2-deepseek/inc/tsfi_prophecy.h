#ifndef TSFI_PROPHECY_H
#define TSFI_PROPHECY_H

#include "tsfi_elektuur_issue25.h"

// Individual entry in the Prophecy response bank
typedef struct {
    float genes[8];      // The 8-gene DNA patch
    float crest_factor;  // Spectral spikiness
    float crossover_dist; // Crossover distortion count
    float v_out_peak;    // Peak output voltage
} TsfiProphecyEntry;

// Core functions
int tsfi_prophecy_generate_bank(const char *filepath, int grid_resolution);
int tsfi_prophecy_inverse_lookup(const char *filepath, float target_crest, float target_dist, TsfiBiotikaDna *out_dna);

#endif // TSFI_PROPHECY_H
