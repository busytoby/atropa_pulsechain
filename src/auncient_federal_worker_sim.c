#include "auncient_federal_worker_sim.h"
#include "auncient_cloth_material_bridge.h"
#include <stdio.h>
#include <string.h>

static int consecutive_noise_count = 0;

void auncient_worker_sim_init(FederalWorkerSim *sim, uint32_t fourier_peak_val) {
    if (!sim) return;
    memset(sim, 0, sizeof(FederalWorkerSim));
    sim->phase = PHASE_UNAUTHORIZED_IMPOSITION;
    sim->fourier_peak_val = fourier_peak_val;
    printf("[WORKER SIM] Initialized raw Fourier imposition with peak value %u.\n", fourier_peak_val);
}

bool auncient_worker_sim_qualify(FederalWorkerSim *sim, const AuncientAnalyzer *analyzer) {
    if (!sim || sim->phase != PHASE_UNAUTHORIZED_IMPOSITION) return false;

    // Determine if raw peak is structured or noise
    uint32_t instruction = sim->fourier_peak_val;
    char op = (char)((instruction >> 24) & 0xFF);
    bool is_noise = (op < 'A' || op > 'Z');

    if (is_noise) {
        consecutive_noise_count++;
        // If the noise exceeds the threshold limit of 3, raise a DEFCON alarm through CADE_IMF_NATO
        if (consecutive_noise_count >= 3) {
            uint16_t status_word = 0;
            int is_valid = 0;
            extern int tsfi_mf_norad_encode_defcon(int defcon_level, int radar_contacts, uint16_t *out_status, int *is_valid);
            tsfi_mf_norad_encode_defcon(2, consecutive_noise_count, &status_word, &is_valid);
            printf("[DEFCON ALARM] CADE_IMF_NATO raised DEFCON 2 alarm. Noise threshold exceeded! Status: 0x%04X\n", status_word);
        }
        return false; // Noise is blocked and not qualified, audit cannot start
    }

    // Reset noise counter if valid structured opcode is processed
    consecutive_noise_count = 0;

    // Convert the raw Fourier peak into a virtual instruction stream for EDSAC analyzer evaluation
    if (analyzer && !auncient_analyzer_classify(analyzer, &instruction, 1)) {
        printf("[WORKER SIM QUALIFY REJECT] EDSAC analyzer rejected the raw Fourier peak value %u.\n", sim->fourier_peak_val);
        return false;
    }

    sim->phase = PHASE_QUALIFIED;
    printf("[WORKER SIM QUALIFY] Fourier peak value %u qualified successfully by EDSAC analyzer.\n", sim->fourier_peak_val);
    return true;
}

bool auncient_worker_sim_audit(FederalWorkerSim *sim) {
    // The audit cannot begin until the fourier imposition is qualified by EDSAC
    if (!sim || sim->phase != PHASE_QUALIFIED) {
        printf("[WORKER SIM AUDIT REJECT] Audit cannot begin. State is not qualified.\n");
        return false;
    }

    // Use the raw fourier peak value to derive the deterministic DNA payload
    memcpy(sim->dna_payload, &sim->fourier_peak_val, sizeof(sim->fourier_peak_val));
    // Additional salt to emulate a dynamic spatial footprint
    sim->dna_payload[4] = 0xAA;
    sim->dna_payload[5] = 0xBB;
    sim->dna_payload[6] = 0xCC;
    sim->dna_payload[7] = 0xDD;

    sim->phase = PHASE_AUDITED;
    printf("[WORKER SIM AUDIT] Imposition audited. Derived DNA footprint: 0x%08X\n", sim->fourier_peak_val);
    return true;
}

bool auncient_worker_sim_confirm(FederalWorkerSim *sim) {
    if (!sim || sim->phase != PHASE_AUDITED) return false;

    // Setup a temporary material block to map using the bridge
    MaterialUniformBlock temp_material;
    memset(&temp_material, 0, sizeof(temp_material));
    // Deterministically map fourier peak to the material seed
    temp_material.seed = sim->fourier_peak_val;

    auncient_bridge_dna_to_ssa(&temp_material, sim->ssn, sim->site, sizeof(sim->site));
    
    sim->phase = PHASE_CONFIRMED;
    printf("[WORKER SIM CONFIRM] Citizen identity confirmed. SSN: %s, Issuance Site: %s\n", sim->ssn, sim->site);
    return true;
}

bool auncient_worker_sim_select(FederalWorkerSim *sim, uint32_t account_id, const uint32_t *pki_keys, int key_count) {
    if (!sim || sim->phase != PHASE_CONFIRMED) return false;

    // Verify system authorization prior to opening account (default-reject)
    // We register the account with Hogan bank. Default endowment of 1,000,000 Saat.
    bool registered = auncient_hogan_register_account(account_id, sim->dna_payload, sizeof(sim->dna_payload), &sim->account);
    if (!registered) {
        printf("[WORKER SIM SELECT REJECT] Hogan Bank registration failed.\n");
        return false;
    }

    // Validate the selection via WinchesterMQ requiring PKI quorum (>= 4 keys)
    // discuss WinchesterMQ triggers return to DisplacementShader vertex sync rules.
    uint32_t selection_code = (account_id & 0xFFFF) | 0x80000000;
    if (!auncient_autodin_dispatch_wmq(selection_code, 0xAA, pki_keys, key_count)) {
        printf("[WORKER SIM SELECT REJECT] WinchesterMQ selection dispatch failed.\n");
        memset(&sim->account, 0, sizeof(HoganAccount));
        return false;
    }

    sim->phase = PHASE_SELECTED_WORKER;
    printf("[WORKER SIM SELECT] Worker selected successfully. Account ID: %u, Balance: %u Saat.\n", 
           sim->account.account_id, sim->account.balance_saat);
    return true;
}
