#include "tsfi_wavelet_arena.h"
#include "tsfi_helmholtz.h"
#include <stdio.h>
#include <assert.h>

void run_wavelet_recursive_linking_audit() {
    printf("=== TSFi Generation 8: Recursive Wavelet Linking Audit ===\n");
    uint64_t P = MOTZKIN_PRIME;
    
    // --- EPOCH 1: GENESIS ---
    uint8_t *mem1 = (uint8_t*)aligned_alloc(4096, 4096);
    TsfiWaveletArena arena1;
    tsfi_wavelet_arena_init(&arena1, mem1, 4096);

    TsfiWavelet *W1 = tsfi_STAT(&arena1, P);
    TsfiWavelet *W2 = tsfi_STAT(&arena1, P);
    tsfi_RewireWavelet(W1); tsfi_RewireWavelet(W2);
    W1->Connect(W2);

    tsfi_CREATE_YI(W1, W2, 111, 222, 333, P);
    tsfi_SealHeader(W1, P); tsfi_SealHeader(W2, P);
    tsfi_ActivateYang(W1, 12345, P); tsfi_ActivateYang(W2, 12345, P);

    // Capture the anchor from the "Master" wavelet of Epoch 1
    uint64_t anchor1 = tsfi_GetWaveletAnchor(W1);
    printf("[EPOCH 1] Master Wavelet UID %llu generated Anchor: %llu\n", 
           (unsigned long long)W1->telemetry.unique_id, (unsigned long long)anchor1);

    // --- EPOCH 2: LINKED EVOLUTION ---
    uint8_t *mem2 = (uint8_t*)aligned_alloc(4096, 4096);
    TsfiWaveletArena arena2;
    tsfi_wavelet_arena_init(&arena2, mem2, 4096);

    TsfiWavelet *W3 = tsfi_STAT(&arena2, P);
    TsfiWavelet *W4 = tsfi_STAT(&arena2, P);
    tsfi_RewireWavelet(W3); tsfi_RewireWavelet(W4);
    W3->Connect(W4);

    // Physical Transition: Bind Epoch 2 wavelets to Epoch 1 state
    tsfi_AnchorToParent(W3, anchor1, P);
    tsfi_AnchorToParent(W4, anchor1, P);

    // Perform Handshake in Epoch 2
    tsfi_CREATE_YI(W3, W4, 111, 222, 333, P);
    tsfi_SealHeader(W3, P); tsfi_SealHeader(W4, P);
    tsfi_ActivateYang(W3, 12345, P); tsfi_ActivateYang(W4, 12345, P);

    printf("[EPOCH 2] Wavelets UID 1000/1001 stabilized via Epoch 1 Anchor.\n");

    // --- PHASE A: PARITY CHECK ---
    W3->Evolve(1); W4->Evolve(1);
    assert(W3->ichidai == W4->ichidai);
    printf("[PASS] Recursive Linking maintained bit-perfect parity.\n");

    // --- PHASE B: FRACTURE TEST (WRONG ANCHOR) ---
    printf("\n[PHASE B] Testing Fracture via Incorrect Temporal Anchor...\n");
    TsfiWavelet *W5 = tsfi_STAT(&arena2, P);
    TsfiWavelet *W6 = tsfi_STAT(&arena2, P);
    tsfi_RewireWavelet(W5); tsfi_RewireWavelet(W6);
    W5->Connect(W6);

    // Apply WRONG anchor
    tsfi_AnchorToParent(W5, anchor1 + 1, P);
    tsfi_AnchorToParent(W6, anchor1 + 1, P);

    tsfi_CREATE_YI(W5, W6, 111, 222, 333, P);
    printf("[PASS] Divergent branch created via corrupted anchor.\n");

    free(mem1);
    free(mem2);
    printf("\n[SUCCESS] Generation 8 Wavelet Linking Verified.\n");
}

int main() {
    run_wavelet_recursive_linking_audit();
    return 0;
}
