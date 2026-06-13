#ifndef TSFI_CRYSTAL_H
#define TSFI_CRYSTAL_H

#include "tsfi_zener.h"

// --- Compatibility Mapping: any operant BIN crystal is a Zener ---
typedef TsfiZener TsfiCrystal;
typedef TsfiZenerCircuit TsfiCrystalCircuit;

static inline void tsfi_crystal_init(TsfiCrystal *crystal, double vz0, double temp_coeff, double sample_rate) {
    tsfi_zener_init(crystal, vz0, temp_coeff, sample_rate);
}

static inline double tsfi_crystal_get_vz(const TsfiCrystal *crystal) {
    return tsfi_zener_get_vz(crystal);
}

static inline double tsfi_crystal_tick(TsfiCrystal *crystal, double vs, double rs, double *noise_out) {
    return tsfi_zener_tick(crystal, vs, rs, noise_out);
}

#endif // TSFI_CRYSTAL_H
