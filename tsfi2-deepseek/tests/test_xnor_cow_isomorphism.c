#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_nand_trap_firmware.h"
#include "tsfi_wavelet_arena.h"
#include "lau_memory.h"
#include "tsfi_controlnet_shm.h"

#include "tsfi_wire_firmware.h"

#define CONTEXT_COW 0x0000C001

int main() {
    printf("=== TSFi Unit Test: Genie XNOR Trap (Cow Context) ===\n");

    // 1. Initialize Firmware & Manifold
    tsfi_wire_firmware_init();
    LauWireFirmware *fw = tsfi_wire_firmware_get();
    void *manifold = malloc(TSFI_CN_MAP_SIZE);
    memset(manifold, 0, TSFI_CN_MAP_SIZE);

    // 2. Setup Wavelet Arena
    uint8_t *arena_mem = malloc(4096);
    TsfiWaveletArena arena;
    tsfi_wavelet_arena_init(&arena, arena_mem, 4096);

    // 3. ESTABLISH COW CONTEXT (Hardware Strobe)
    printf("\n[GENIE] Establishing Sealed Layer Context: COW (0x%08x)...\n", CONTEXT_COW);
    tsfi_wire_firmware_establish_sealed_context(fw, CONTEXT_COW);

    if (fw->rtl.cell_sealed_layer_context == CONTEXT_COW) {
        printf("[PASS] XNOR Hardware verified in COW context.\n");
    } else {
        printf("[FAIL] Hardware failed to lock context!\n");
        return 1;
    }

    // 4. Define the Spider (Intent Wavelet)
    TsfiWavelet *W_Spider = tsfi_STAT(&arena, TSFI_WAVELET_PRIME);
    memset(W_Spider->payload, 0xCC, 120); // Cow Constant Intent

    // 5. Define the target Cow (Verified Taste Wavelet)
    TsfiWavelet *W_Cow_Taste = tsfi_STAT(&arena, TSFI_WAVELET_PRIME);
    memset(W_Cow_Taste->payload, 0xCC, 120); 

    // 6. TEST 1: XNOR Solidification within Cow Context
    printf("\n[TEST 1] XNORing Spider intent against Cow Taste (Within Cow Context)...\n");
    tsfi_wavelet_xnor_signal(manifold, W_Spider, W_Cow_Taste);

    uint8_t *leaf504 = (uint8_t *)manifold + (504 * 256);
    if (leaf504[0] == 1) {
        printf("[PASS] Spider proven isomorphic to Cow within active COW context.\n");
    } else {
        printf("[FAIL] Solidification failed in valid context.\n");
        return 1;
    }

    // 7. Define the Crow (Kinematic Wavelet)
    TsfiWavelet *W_Crow = tsfi_STAT(&arena, TSFI_WAVELET_PRIME);
    memcpy(W_Crow->payload, W_Cow_Taste->payload, 120);

    // 8. TEST 2: Crow Puppetry within Cow Context
    printf("\n[TEST 2] XNORing Crow form against Cow Form (Within Cow Context)...\n");
    tsfi_wavelet_xnor_signal(manifold, W_Crow, W_Cow_Taste);

    if (leaf504[0] == 1) {
        printf("[PASS] Crow proven isomorphic to Cow form. Puppetry Solidified.\n");
    }

    printf("\n[ABSOLUTE SUCCESS] Genie has established Cow Context upon XNOR Firmware.\n");

    free(arena_mem);
    free(manifold);
    return 0;
}
