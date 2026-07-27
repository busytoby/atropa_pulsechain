#ifndef AUNCIENT_FEDERAL_WORKER_SIM_H
#define AUNCIENT_FEDERAL_WORKER_SIM_H

#include "auncient_timeline_autodin.h"
#include "auncient_edsac_firewall.h"
#include <stdint.h>
#include <stdbool.h>

// Represents the phase state of the federal worker candidate
typedef enum {
    PHASE_UNAUTHORIZED_IMPOSITION = 0, // Raw Fourier spectral input (noise/no credentials)
    PHASE_QUALIFIED = 1,               // Qualified by EDSAC via analyzer check on the peak value
    PHASE_AUDITED = 2,                 // FNV-1a DNA signature hash verified
    PHASE_CONFIRMED = 3,               // SSN derived via auncient_bridge_dna_to_ssa
    PHASE_SELECTED_WORKER = 4          // HoganAccount opened with 1M Saat, authorized for work
} worker_phase_t;

typedef struct {
    worker_phase_t phase;
    uint32_t fourier_peak_val;         // The raw spectral coordinate value (the imposition)
    uint8_t dna_payload[8];
    char ssn[16];
    char site[32];
    HoganAccount account;
} FederalWorkerSim;

// Initializes a new simulation with a raw Fourier peak value
void auncient_worker_sim_init(FederalWorkerSim *sim, uint32_t fourier_peak_val);

// Phase 0.5: EDSAC qualifies the Fourier imposition via analyzer check on its specific raw peak value
bool auncient_worker_sim_qualify(FederalWorkerSim *sim, const AuncientAnalyzer *analyzer);

// Phase 1: Audit the qualified Fourier imposition by calculating the FNV-1a DNA footprint
bool auncient_worker_sim_audit(FederalWorkerSim *sim);

// Phase 2: Confirm the citizen's identity by translating the DNA footprint to a regional SSN
bool auncient_worker_sim_confirm(FederalWorkerSim *sim);

// Phase 3: Select the confirmed citizen as an active worker, creating a Hogan account with 1M Saat
bool auncient_worker_sim_select(FederalWorkerSim *sim, uint32_t account_id, const uint32_t *pki_keys, int key_count);

#endif // AUNCIENT_FEDERAL_WORKER_SIM_H
