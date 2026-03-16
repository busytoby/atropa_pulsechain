#include "tsfi_wavelet_arena.h"
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

void run_generation7_persistence_audit() {
    printf("=== TSFi Generation 7 Persistence Audit ===\n");
    uint64_t P = MOTZKIN_PRIME;
    const char *manifold_path = "tests/gen7_manifold.pos";
    
    // --- STAGE 1: GENESIS ---
    uint8_t *mem = (uint8_t*)aligned_alloc(4096, 4096);
    TsfiWaveletArena arena;
    tsfi_wavelet_arena_init(&arena, mem, 4096);

    TsfiWavelet *W1 = tsfi_STAT(&arena, P);
    TsfiWavelet *W2 = tsfi_STAT(&arena, P);
    tsfi_RewireWavelet(W1); tsfi_RewireWavelet(W2);
    W1->Connect(W2);

    // Take to SEAL11 (YANG)
    tsfi_CREATE_YI(W1, W2, 111, 222, 333, P);
    tsfi_SealHeader(W1, P); tsfi_SealHeader(W2, P);
    tsfi_ActivateYang(W1, 12345, P); tsfi_ActivateYang(W2, 12345, P);

    TsfiWavelet *members[2] = {W1, W2};
    TsfiSheaf S;
    tsfi_CreateSheaf(&S, members, 2);

    // Evolve to specific state
    W1->Evolve(1); W2->Evolve(1);
    tsfi_ReduceSheaf(&S, P);
    assert(S.latch_q == 1);
    uint64_t genesis_receipt = S.Receipt;
    printf("[GENESIS] Sheaf Rigid (1,1). Receipt: %llu\n", (unsigned long long)genesis_receipt);

    // --- STAGE 2: PERSISTENCE ---
    TsfiSheafRigid R;
    tsfi_ProjectSheaf(&R, &S, &arena);

    int fd = open(manifold_path, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (write(fd, mem, 4096) != 4096) exit(1);
    if (write(fd, &R, sizeof(R)) != sizeof(R)) exit(1);
    close(fd);
    printf("[PERSIST] Manifold and Rigid Sheaf projected to disk.\n");

    // --- STAGE 3: RESTORATION ---
    memset(mem, 0, 4096); // Clear memory
    TsfiSheaf restored_S;
    TsfiSheafRigid restored_R;

    fd = open(manifold_path, O_RDONLY);
    if (read(fd, mem, 4096) != 4096) exit(1);
    if (read(fd, &restored_R, sizeof(restored_R)) != sizeof(restored_R)) exit(1);
    close(fd);

    tsfi_RestoreSheaf(&restored_S, &restored_R, &arena);
    printf("[RESTORE] Sheaf and Wavelets re-bound. JIT Methods rewired.\n");

    // --- STAGE 4: VERIFICATION ---
    printf("\n[VERIFY] Validating Resonant Evolution on Restored State...\n");
    // Pulse 0 on restored state
    restored_S.members[0]->Evolve(0); 
    restored_S.members[1]->Evolve(0);
    
    tsfi_ReduceSheaf(&restored_S, P);
    assert(restored_S.latch_q == 1);
    assert(restored_S.Receipt != genesis_receipt); // Must have evolved
    printf("[PASS] Restored Sheaf maintains Physical Stability (1,1).\n");

    printf("\n[VERIFY] Validating NAND Trap on Restored State...\n");
    restored_S.members[0]->Evolve(1); // Fracture
    tsfi_ReduceSheaf(&restored_S, P);
    assert(restored_S.latch_q == 0);
    printf("[PASS] Restored Sheaf correctly triggers NAND Trap (0,0) on fracture.\n");

    printf("\n[SUCCESS] Generation 7 Persistence Standard Verified.\n");
    free(mem);
    unlink(manifold_path);
}

int main() {
    run_generation7_persistence_audit();
    return 0;
}
