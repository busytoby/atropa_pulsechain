#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// Polarized DMA and PMD Transistor state
typedef struct {
    double dma_npn_base; // Voltage on NPN Black base
    double pmd_pnp_base; // Voltage on PNP Red base
    gate_state_t dma_gate;
    gate_state_t pmd_gate;
    char dma_buffer[64];
    char pmd_buffer[64];
} dma_pmd_bjts_t;

// -------------------------------------------------------------
// Polarized Conduction Step
// -------------------------------------------------------------
void dma_pmd_conduction_step(dma_pmd_bjts_t *state, const char *raw_coax_wave, 
                             const char *dma_desc_data) {
    // 1. DMA (NPN Black) -> Active-High Conduction (Base voltage > 0.7V)
    if (state->dma_npn_base > 0.7) {
        state->dma_gate = CONDUC_STATE;
        strcpy(state->dma_buffer, dma_desc_data);
    } else {
        state->dma_gate = CUTOFF_STATE;
    }

    // 2. PMD (PNP Red) -> Active-Low Conduction (Base voltage < 0.2V)
    if (state->pmd_pnp_base < 0.2) {
        state->pmd_gate = CONDUC_STATE;
        // Demodulate raw wave to PMD buffer (simulated)
        strcpy(state->pmd_buffer, raw_coax_wave);
    } else {
        state->pmd_gate = CUTOFF_STATE;
    }
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT POLARIZED DMA AND PMD BJT VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    dma_pmd_bjts_t state = {
        .dma_npn_base = 0.0,
        .pmd_pnp_base = 5.0, // High voltage initially (NPN/PNP cutoff)
        .dma_gate = CUTOFF_STATE,
        .pmd_gate = CUTOFF_STATE,
        .dma_buffer = "",
        .pmd_buffer = ""
    };

    // 1. Assert DMA base voltage high (5.0V) and PMD high (5.0V)
    // -> DMA NPN Black should conduct, PMD PNP Red should cutoff (noisy line isolation)
    printf("[TEST] Running step with active DMA (5.0V) and noisy PMD (5.0V)...\n");
    fflush(stdout);
    state.dma_npn_base = 5.0;
    state.pmd_pnp_base = 5.0;
    dma_pmd_conduction_step(&state, "NOISY_CARRIER_WAVE", "DMA_PAYLOAD_DATA");

    assert(state.dma_gate == CONDUC_STATE);
    assert(strcmp(state.dma_buffer, "DMA_PAYLOAD_DATA") == 0);
    assert(state.pmd_gate == CUTOFF_STATE); // Isolated
    assert(strcmp(state.pmd_buffer, "") == 0);
    printf("   ✓ DMA conducted successfully. PMD isolated from noise.\n");
    fflush(stdout);

    // Reset buffers
    strcpy(state.dma_buffer, "");
    strcpy(state.pmd_buffer, "");

    // 2. Assert DMA base low (0.0V) and PMD low (0.0V) (Quiet line-level)
    // -> DMA NPN Black should cutoff, PMD PNP Red should conduct
    printf("[TEST] Running step with inactive DMA (0.0V) and quiet PMD (0.0V)...\n");
    fflush(stdout);
    state.dma_npn_base = 0.0;
    state.pmd_pnp_base = 0.0;
    dma_pmd_conduction_step(&state, "CLEAN_CARRIER_WAVE", "DMA_PAYLOAD_DATA");

    assert(state.dma_gate == CUTOFF_STATE); // Isolated
    assert(strcmp(state.dma_buffer, "") == 0);
    assert(state.pmd_gate == CONDUC_STATE);
    assert(strcmp(state.pmd_buffer, "CLEAN_CARRIER_WAVE") == 0);
    printf("   ✓ PMD conducted clean signal. DMA isolated successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("POLARIZED DMA AND PMD BJT TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
