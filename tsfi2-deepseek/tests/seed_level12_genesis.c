#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "tsfi_merkle.h"
#include "tsfi_helmholtz.h"
#include "tsfi_io.h"

#define MANIFOLD_SIZE_2MB (2 * 1024 * 1024)
#define TARGET_MANIFOLD "audit_lore.pos"

int main() {
    printf("=== TSFi Level 12 Physical Expansion ===\n");
    tsfi_helmholtz_init();

    void *manifold = calloc(1, MANIFOLD_SIZE_2MB);
    if (!manifold) {
        perror("calloc manifold");
        return 1;
    }

    printf("[ACT] Restoring legacy manifold data...\n");
    if (tsfi_restore_manifold(TARGET_MANIFOLD, manifold, 1024 * 1024) == 0) {
        printf("  -> Legacy state root captured.\n");
    }

    uint8_t s[32], r[32], h[32];
    printf("[REDUCTION] Calculating first 12-level State Root (4096 leaves)...\n");
    tsfi_helmholtz_reduce_12(s, r, h, NULL, NULL, manifold, 1, 2026, NULL);

    printf("\n=== NEW TOPOGRAPHY SOLIDIFIED ===\n");
    printf("State Root: ");
    for(int i=0; i<32; i++) printf("%02x", s[i]);
    printf("\n");

    if (tsfi_persist_manifold(TARGET_MANIFOLD, manifold, MANIFOLD_SIZE_2MB) == 0) {
        printf("[SUCCESS] Manifold expanded to 2MB.\n");
    } else {
        printf("[FAIL] Failed to persist expanded manifold.\n");
        free(manifold);
        tsfi_helmholtz_cleanup();
        return 1;
    }

    free(manifold);
    tsfi_helmholtz_cleanup();
    return 0;
}
