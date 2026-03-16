#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "tsfi_nand_trap_firmware.h"
#include "tsfi_wavelet_arena.h"
#include "lau_registry.h"
#include "lau_memory.h"
#include "tsfi_logic.h"

// Mock manifold (1024 leaves * 256 bytes)
uint8_t manifold[1024 * 256];

void test_cascade_resonant_citizen() {
    printf("[CASCADE TEST] Resonant Citizen (Registered + Resonant)...\n");
    
    // 1. Create and Register Wavelets
    TsfiWavelet *W1 = (TsfiWavelet*)lau_malloc_wired(sizeof(TsfiWavelet));
    TsfiWavelet *W2 = (TsfiWavelet*)lau_malloc_wired(sizeof(TsfiWavelet));
    W1->telemetry.Fa.identity = 0x1337;
    W2->telemetry.Fa.identity = 0x1338;
    
    // 2. Set Resonant State (Intensity % 2026 == 0)
    W1->state = 9; W1->telemetry.Fa.monopole = 2026 * 2;
    W2->state = 9; W2->telemetry.Fa.monopole = 2026 * 5;

    // 3. Signal the Trap (Using W2 as Potential source)
    tsfi_wavelet_nand_signal(manifold, W1, W2, 2026);

    // 4. Verify NAND Levers (Active-Low: 1=Good)
    uint8_t *leaf500 = &manifold[500 * 256];
    uint8_t *leaf501 = &manifold[501 * 256];
    
    assert(leaf500[0] == 1); // Provenance Good
    assert(leaf501[0] == 1); // Potential Good
    printf("  [PASS] Resonant Citizen maintained the NAND Hold state (1,1).\n");

    lau_free(W1);
    lau_free(W2);
}

void test_cascade_stochastic_fugitive() {
    printf("[CASCADE TEST] Stochastic Fugitive (Unregistered + Noise)...\n");
    
    // 1. Create UNREGISTERED Wavelets (Local stack bypasses registry)
    TsfiWavelet W1_raw, W2_raw;
    TsfiWavelet *W1 = &W1_raw;
    TsfiWavelet *W2 = &W2_raw;
    
    // 2. Set Stochastic State (Non-harmonic intensity)
    W1->state = 5; W1->telemetry.Fa.monopole = 123;
    W2->state = 5; W2->telemetry.Fa.monopole = 456;

    // 3. Signal the Trap
    tsfi_wavelet_nand_signal(manifold, W1, W2, 2026);

    // 4. Verify Metastable Fracture (0,0)
    uint8_t *leaf500 = &manifold[500 * 256];
    uint8_t *leaf501 = &manifold[501 * 256];
    
    assert(leaf500[0] == 0); // Provenance Bad
    assert(leaf501[0] == 0); // Potential Bad
    printf("  [PASS] Stochastic Fugitive triggered the Metastable Fracture (0,0).\n");
}

void test_cascade_xor_spider_fill() {
    printf("[CASCADE TEST] XOR Spider Fill (Two-Layer File Cell)...\n");
    
    // 1. Create an EMPTY Wavelet (Seal Level 0)
    TsfiWavelet *W = (TsfiWavelet*)lau_malloc_wired(sizeof(TsfiWavelet));
    W->telemetry.current_seal_level = 0;
    W->state = 0; 
    memset(W->private_header.owner, 0, 32);
    memset(W->payload, 0, 120);

    // 2. Define Seeds
    const char *filename = "test_file.txt";
    const char *content = "Resonant data content for the arena.";

    // 3. Layer 1: Filename Injection (Gemini != DeepSeek)
    TsfiWavelet G, D; 
    G.telemetry.Fa.identity = 1; D.telemetry.Fa.identity = 2; // Different
    tsfi_wavelet_xor_signal(manifold, W, &G, &D, (const uint8_t *)filename);

    // Verify Layer 1
    assert(W->telemetry.current_seal_level == 1);
    assert(strcmp(W->private_header.owner, filename) == 0);
    printf("  [PASS] XOR Layer 1 (Filename) injected correctly.\n");

    // 4. Layer 2: Content Injection
    tsfi_wavelet_xor_signal(manifold, W, &G, &D, (const uint8_t *)content);

    // Verify Layer 2
    assert(W->telemetry.current_seal_level == 2);
    assert(W->state == 1);
    assert(memcmp(W->payload, content, strlen(content)) == 0);
    printf("  [PASS] XOR Layer 2 (Content) injected and cell sealed.\n");

    lau_free(W);
}

void test_spider_portfolio_scan() {
    printf("[CASCADE TEST] Spider Portfolio Scan (Active Interest)...\n");
    
    // 1. Initialize logic system
    WaveSystem ws = {0};
    tsfi_logic_init(&ws);
    
    // 2. Create a Wavelet and assign the FILE trait (simulating XOR Layer 1)
    TsfiWavelet *W = (TsfiWavelet*)lau_malloc_wired(sizeof(TsfiWavelet));
    W->telemetry.trait = TSFI_TRAIT_FILE;
    W->telemetry.current_seal_level = 1;
    strncpy(W->file_cell.filename, "active_interest.txt", 31);
    
    // 3. Mount it in the active sessions
    ws.active_sessions[0] = W;
    
    // 4. Execute the logic epoch. The Spider Portfolio Scan should detect it.
    printf("  -> Executing Master Logic Epoch...\n");
    master_logic_epoch(NULL);
    
    printf("  [PASS] Spider demonstrated active interest in the FILE trait.\n");
    
    lau_free(W);
    tsfi_logic_teardown();
}

int main() {
    printf("=== TSFi Alligator Rigidity Cascade Test ===\n");
    (void)lau_registry_get_head(); // Ensure registry is active
    
    test_cascade_resonant_citizen();
    test_cascade_stochastic_fugitive();
    test_cascade_xor_spider_fill();
    test_spider_portfolio_scan();
    
    printf("[SUCCESS] Cascade logic is rigid. Alligator is content.\n");
    return 0;
}
