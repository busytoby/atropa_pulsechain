#ifndef TSFI_TOTIENT_QING_TOMOGRAPHY_H
#define TSFI_TOTIENT_QING_TOMOGRAPHY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "tsfi_stekloff_duality.h"
#include "tsfi_chamberland_duality.h"

// Dynamic Totient Qing Instance Struct
typedef struct {
    char     qing_symbol[64];           // Dynamic Qing Symbol Name (e.g. "ZorseQing")
    uint64_t starting_balance_saat;     // Starting endowment in Saat (e.g. 1,000,000 Saat)
    uint32_t totient_n;                 // Modular prime field denominator n
    float    totient_qing_radius;       // Derived r_{qing} = \phi(n) * 0.125f
    tsfi_stekloff_state_t stekloff;     // Stekloff Eigenvalue State
    tsfi_chamberland_accumulator_t *acc;// Dual-Bar Chamberland Accumulator
} tsfi_totient_qing_t;

// Creates a Dynamic Totient Qing Instance with specified symbol, endowment, and prime field n
tsfi_totient_qing_t* tsfi_totient_qing_create(
    const char *symbol,
    uint64_t starting_saat,
    uint32_t totient_n
);

// Evaluates Dual-Bar Tomography over activation vector x relative to Totient Qing Instance
bool tsfi_totient_qing_eval_tomography(
    tsfi_totient_qing_t *qing,
    const float *x,
    int dim,
    uint32_t *active_stekloff_mode_out,
    float *duality_metric_out
);

// Destroys Dynamic Totient Qing Instance
void tsfi_totient_qing_destroy(tsfi_totient_qing_t *qing);

#endif // TSFI_TOTIENT_QING_TOMOGRAPHY_H
