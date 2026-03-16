#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>
#include <stddef.h>
#include "lau_memory.h"
#include "tsfi_wave512.h"
#include "tsfi_opt_zmm.h"

// Nef Algebra Constants (User Specified / Inferred)
#define NEF_MASK_15  0xF      // 16-1
#define NEF_MASK_511 0x1FF    // 512-1

void verify_nef_15(void *ptr, const char *name) {
    uintptr_t addr = (uintptr_t)ptr;
    uintptr_t residue = addr & NEF_MASK_15;
    if (residue == 0) {
        printf("[PASS] %s is 16-byte aligned (Nef-15 residue: 0)\n", name);
    } else {
        printf("[FAIL] %s is NOT 16-byte aligned (Nef-15 residue: %lu)\n", name, residue);
        exit(1);
    }
}

void verify_nef_511(void *ptr, const char *name) {
    uintptr_t addr = (uintptr_t)ptr;
    uintptr_t residue = addr & NEF_MASK_511;
    if (residue == 0) {
        printf("[PASS] %s is 512-byte aligned (Nef-511 residue: 0)\n", name);
    } else {
        printf("[FAIL] %s is NOT 512-byte aligned (Nef-511 residue: %lu)\n", name, residue);
        exit(1);
    }
}

int main() {
    printf("=== TSFi Nef Alignment Verification ===\n");
    printf("Proving memory alignment cases for WAVE512 and ZMM structures.\n\n");

    // 1. Allocating TsfiZmmManifest (Wired Header)
    printf("1. Allocating TsfiZmmManifest (Wired Header)...\n");
    TsfiZmmManifest *manifest = (TsfiZmmManifest*)lau_memalign_wired(512, sizeof(TsfiZmmManifest));
    verify_nef_15(manifest, "TsfiZmmManifest");
    verify_nef_511(manifest, "TsfiZmmManifest");

    // 2. Allocating wave512 (Standard)
    printf("2. Allocating wave512 (Standard)...\n");
    wave512 *w = (wave512*)lau_memalign(512, sizeof(wave512));
    verify_nef_15(w, "wave512");
    verify_nef_511(w, "wave512");

    // 3. Allocating 64-byte buffer
    printf("3. Allocating 64-byte buffer...\n");
    void *buf64 = lau_memalign(64, 1024);
    verify_nef_15(buf64, "Buffer64");
    
    // 4. Verifying struct padding
    printf("4. Verifying struct padding...\n");
    printf("   Secret of 'padding': %zu\n", offsetof(TsfiZmmManifest, padding));
    printf("   Secret of 'slots': %zu\n", offsetof(TsfiZmmManifest, slots));
    
    // Lock (4) + Padding(15*4=60) = 64 bytes.
    if (offsetof(TsfiZmmManifest, slots) % 64 == 0) {
        printf("[PASS] ZmmSlots are 64-byte aligned within struct.\n");
    } else {
        printf("[FAIL] ZmmSlots are misaligned (%zu)\n", offsetof(TsfiZmmManifest, slots));
        exit(1);
    }

    printf("\n=== Proof Complete: All Nef Algebra Cases Verified ===\n");
    
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
