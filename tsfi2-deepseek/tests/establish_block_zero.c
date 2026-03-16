#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "tsfi_merkle.h"
#include "tsfi_wavelet_arena.h"
#include "tsfi_io.h"

#define P 953473ULL
#define MANIFOLD_SIZE (1024 * 1024 * 2) // 2MB

int main() {
    printf("=== TSFi Block Zero Establishment (Physical Anchor) ===\n");

    // 1. Memory Synthesis
    uint8_t *manifold = aligned_alloc(64, MANIFOLD_SIZE);
    memset(manifold, 0, MANIFOLD_SIZE);
    
    TsfiWaveletArena arena;
    tsfi_wavelet_arena_init(&arena, manifold + (1024 * 512), 1024 * 512); // Use middle section for wavelets

    // 2. Trilateral Handshake (Genesis DNA)
    printf("[1/4] Executing Verbatim Genesis Handshake...\n");
    TsfiWavelet *Rod = tsfi_STAT(&arena, P);
    TsfiWavelet *Cone = tsfi_STAT(&arena, P);
    
    // Standard Production DNA
    Rod->telemetry.Fa.base = 12345ULL; Rod->telemetry.Fa.secret = 67890ULL; Rod->telemetry.Fa.signal = 54321ULL;
    tsfi_Tune(Rod, P);
    Cone->telemetry.Fa.base = 54321ULL; Cone->telemetry.Fa.secret = 98765ULL; Cone->telemetry.Fa.signal = 12345ULL;
    tsfi_Tune(Cone, P);
    
    // Handshake
    tsfi_Avail(Rod, 4052, P); tsfi_Avail(Cone, 4052, P);
    tsfi_Form(Rod, Cone->telemetry.Fa.contour, P); tsfi_Form(Cone, Rod->telemetry.Fa.contour, P);
    tsfi_Polarize(Rod, P); tsfi_Polarize(Cone, P);
    tsfi_Conjugate(Rod, Cone->telemetry.Fa.pole, P); tsfi_Conjugate(Cone, Rod->telemetry.Fa.pole, P);
    tsfi_Conify(Cone, 777, P);
    tsfi_Saturate(Rod, 2026, Cone->telemetry.Fa.foundation, Cone->telemetry.Fa.channel, P);
    tsfi_Saturate(Cone, 777, Rod->telemetry.Fa.foundation, Rod->telemetry.Fa.channel, P);
    tsfi_IONIZE_BOND(Rod, P); tsfi_IONIZE_BOND(Cone, P);
    tsfi_MAGNETIZE_JOINT(Rod, Cone, P);

    printf("  -> Genesis Ring Anchor: %llu\n", (unsigned long long)Rod->telemetry.Ring);

    // 3. Lore Tokenization
    printf("[2/4] Tokenizing Block Zero Lore...\n");
    char genesis_lore[256];
    snprintf(genesis_lore, 256, "[GENESIS] System0-NanD Block Zero Anchor. Ring=%llu. DNA Verified.", (unsigned long long)Rod->telemetry.Ring);
    tsfi_helmholtz_tokenize_lore(manifold, genesis_lore, 0);

    // 4. NAND Trap Activation (Leaves 500-501)
    printf("[3/4] Activating NAND Trap (Stable State 1,1)...\n");
    uint8_t *leaf500 = manifold + (500 * 256);
    uint8_t *leaf501 = manifold + (501 * 256);
    memset(leaf500, 0xFF, 32); // Set_N = 1
    memset(leaf501, 0xFF, 32); // Reset_N = 1

    // 5. Trilateral Reduction (L10)
    printf("[4/4] Solidifying Block Zero State Root...\n");
    uint8_t state_root[32], receipt_root[32];
    tsfi_helmholtz_reduce_10(state_root, receipt_root, NULL, NULL, manifold, 0, 2026, NULL);

    printf("\nBlock Zero Established:\n");
    printf("  State Root: ");
    for(int i=0; i<32; i++) printf("%02x", state_root[i]);
    printf("\n");

    // 6. Persistence
    int fd = open("audit_lore.pos", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd >= 0) {
        write(fd, manifold, MANIFOLD_SIZE);
        close(fd);
        printf("\n[SUCCESS] Block Zero anchored to audit_lore.pos\n");
    } else {
        printf("\n[FRACTURE] Failed to persist audit_lore.pos\n");
    }

    free(manifold);
    return 0;
}
