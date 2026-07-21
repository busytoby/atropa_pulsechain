#ifndef TSFI_NATHER_PHOTOMETRY_H
#define TSFI_NATHER_PHOTOMETRY_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint64_t total_photon_counts; // Total photon pulses counted
    uint64_t observation_ns;      // Observation duration in nanoseconds
    double photon_flux_rate;      // Photons per second (Hz)
    uint32_t evm_gas_cost;        // 1,200 Gas Cost per Data Block
    double fet_power_watts;       // Power in Watts under 78.2% FET Drop
    char tape_dat_bin[128];       // Rule 13 Filename (.DAT.BIN)
} tsfi_nather_photometry_summary_t;

/* Process photon pulse arrival timestamps using Nather Synchronous Accumulation */
int tsfi_nather_process_photometry(
    const uint64_t *timestamps_ns,
    size_t count,
    tsfi_nather_photometry_summary_t *out_summary
);

#endif // TSFI_NATHER_PHOTOMETRY_H
