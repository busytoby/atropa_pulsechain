#include "tsfi_wavelet_arena.h"
#include <stdio.h>
#include <assert.h>

void run_level1_nand_audit() {
    printf("=== TSFi Turing Level 1 NAND Completeness Audit ===\n");
    uint8_t *mem = aligned_alloc(4096, 4096);
    TsfiWaveletArena arena;
    tsfi_wavelet_arena_init(&arena, mem, 4096);
    uint64_t P = MOTZKIN_PRIME;

    TsfiWavelet *W1 = tsfi_STAT(&arena, P); TsfiWavelet *W2 = tsfi_STAT(&arena, P);
    TsfiWavelet *W3 = tsfi_STAT(&arena, P); TsfiWavelet *W4 = tsfi_STAT(&arena, P);
    
    // 1. JIT Wiring and Bijective Connection (Standard Procedure)
    tsfi_RewireWavelet(W1); tsfi_RewireWavelet(W2);
    tsfi_RewireWavelet(W3); tsfi_RewireWavelet(W4);
    W1->Connect(W2); W3->Connect(W4);

    // 2. 9-Epoch Trilateral Handshake (Standard Procedure)
    tsfi_CREATE_YI(W1, W2, 111, 222, 333, P);
    tsfi_CREATE_YI(W3, W4, 111, 222, 333, P);

    // 3. Sealing and Activation (Standard Procedure)
    TsfiWavelet *all[4] = {W1, W2, W3, W4};
    for(int i=0; i<4; i++) {
        tsfi_SealHeader(all[i], P);
        tsfi_ActivateYang(all[i], 12345, P);
    }

    // 4. Bundling into Epoch 1 Sheaf
    TsfiSheaf S;
    tsfi_CreateSheaf(&S, all, 4);

    // --- CASE 1: RIGIDITY [0,1] ---
    printf("\n[PHASE A] Applying Pulse Sequence [0,1] (Synchronized)...\n");
    // Pulse 0
    W1->Evolve(0); W2->Evolve(0);
    W3->Evolve(0); W4->Evolve(0);
    // Pulse 1
    W1->Evolve(1); W2->Evolve(1);
    W3->Evolve(1); W4->Evolve(1);
    
    tsfi_ReduceSheaf(&S, P);
    assert(S.latch_q == 1 && S.latch_q_bar == 1);
    assert(S.Receipt > 0);
    printf("[PASS] Physical Stability (1,1) achieved for [0,1] sequence.\n");

    // --- CASE 2: NAND TRAP (Logic Fracture) ---
    printf("\n[PHASE B] Inducing Logic Fracture (Asynchronous Pulse)...\n");
    W1->Evolve(0); 
    // W2 remains at previous state -> Parity Fractured
    
    tsfi_ReduceSheaf(&S, P);
    assert(S.latch_q == 0 && S.latch_q_bar == 0);
    assert(S.Receipt == 0);
    assert(S.state == 0xFF);
    printf("[PASS] Logic Fracture detected. NAND Trap (0,0) triggered.\n");

    printf("\n[SUCCESS] Turing Level 1 is NAND complete for [0,1].\n");
    free(mem);
}

int main() {
    run_level1_nand_audit();
    return 0;
}
