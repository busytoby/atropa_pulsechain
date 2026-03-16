#include "tsfi_vae_firmware.h"
#include <string.h>
#include <stdio.h>

/**
 * @brief Bootstraps the VAE Fleet Registry with the core vessels.
 */
void tsfi_vae_boot_fleet(LauVaeFirmware_State *state) {
    if (!state) return;

    state->registry_count = 0;

    // 1. Atropa/999 (Primary Structural Anchor)
    TSFiVesselIdentity *v1 = &state->fleet_registry[state->registry_count++];
    strncpy(v1->name, "Atropa/999", 31);
    v1->forensic_lock = 1737311782441ULL;
    v1->resonance_freq = 0.999f;
    v1->saat_triad[0] = 0x36; v1->saat_triad[1] = 0x32; v1->saat_triad[2] = 0xC8;

    // 2. GENEVA (Clinical/Structural Finality)
    TSFiVesselIdentity *v2 = &state->fleet_registry[state->registry_count++];
    strncpy(v2->name, "GENEVA", 31);
    v2->forensic_lock = 1737384720000ULL;
    v2->resonance_freq = 0.950f;
    v2->saat_triad[0] = 0x41; v2->saat_triad[1] = 0x42; v2->saat_triad[2] = 0x43;

    // Default to Atropa/999 for the Teddy Bear Channel
    state->active_vessel = *v1;
    printf("[VAE-FIRMWARE] Fleet Registry Active. Anchor set to %s.\n", v1->name);
}

void tsfi_vessel_anchor_inject(NandTrapState *target, uint32_t resonance) {
    if (!target) return;
    target->magic = NAND_TRAP_MAGIC;
    target->version = NAND_TRAP_VERSION;
    target->epoch = (float)resonance / 1000.0f;
    target->external_params[0] = 1.0f; // Default intensity
}

/**
 * @brief Anchors the VAE decoding pass to a specific vessel.
 */
void tsfi_vae_anchor_to_vessel(LauVaeFirmware_State *state, const char *vessel_name) {
    if (!state || !vessel_name) return;

    for (int i = 0; i < state->registry_count; i++) {
        if (strcmp(state->fleet_registry[i].name, vessel_name) == 0) {
            state->active_vessel = state->fleet_registry[i];
            
            // Adjust spectral feedback (Blue Secret) based on vessel frequency
            state->spectral_feedback *= state->active_vessel.resonance_freq;
            
            printf("[VAE-FIRMWARE] Re-anchored to %s (Freq: %.3f)\n", 
                   vessel_name, state->active_vessel.resonance_freq);
            return;
        }
    }
    fprintf(stderr, "[FRACTURE] Vessel %s not found in VAE registry.\n", vessel_name);
}
