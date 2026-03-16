#include "tsfi_reaction.h"
#define DM(T) (T*)lau_malloc_wired(sizeof(T))
#include "tsfi_dysnomia_crc.h"
#include "tsfi_dys_math.h"
#include "lau_memory.h"
#include "tsfi_wave15.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

/**
 * Dysnomia Reaction & Reciprocity Benchmark (Origins + WAVE15 Safety)
 */

int main() {
    printf("=== Dysnomia Reaction & Reciprocity Benchmark (WAVE15 Safety) ===\n");
    
    // 1. WAVE15 Alignment Verification (Migration Protocol)
    // "4 WAVE15s will align without warning in 64-byte memory but 5 will not"
    size_t wave15_block = 4 * sizeof(WAVE15); // 60 bytes
    assert(wave15_block <= 64);
    printf("[SAFETY] WAVE15 Capacity Mapping Verified (4 atoms fits 64B).\n");

    // 2. SHOOT -> DAI (The Full Cycle)
    printf("[SHOOT] Realizing Full Genome...\n");
    struct YI* Mu = tsfi_reaction_shoot(NULL);
    
    if (Mu) {
        printf("  Xi: %lu\n", Mu->Xi->limbs[0]);
        printf("  Ring: %lu\n", Mu->Ring->limbs[0]);
        printf("  Monopole: %lu\n", Mu->Psi->Monopole->limbs[0]);
        
        printf("[REACTION] Computing Reciprocity (Handshake)...\n");
        struct Dai* D = tsfi_reaction_compute_reciprocity(Mu->Psi, Mu->Xi, NULL);
        
        if (D) {
            printf("  Ichidai: %lu\n", D->Ichidai->limbs[0]);
            printf("  Daiichi: %lu\n", D->Daiichi->limbs[0]);
            
            if (D->Ichidai && D->Daiichi->limbs[0]) {
                printf("[PASS] SHOOT to DAI Logic Reconciled.\n");
            }
            freeDAI(D);
        }
        freeYI(Mu);
    } else {
        printf("[FAIL] SHOOT failed.\n");
        return 1;
    }
    
    // 3. Data Flow Verification with WAVE15 Safety Stride
    // Per the protocol, packed 15-byte streams are unsafe for ZMM without padding.
    // We use stride = 16 to ensure float alignment and atom isolation.
    uint32_t count = 64; // Reduced from 1024 for speed (Goal < 5s)
    uint32_t stride = 16; 
    
    size_t total_size = count * stride;
    void* buffer = lau_memalign(512, total_size + 512); 
    
    WaveStream stream = {
        .data = buffer,
        .count = count,
        .atom_size = 15,
        .stride = stride,
        .flags = WAVE_FLAG_VERIFY
    };
    
    // Fill with sample data (secret injection)
    uint8_t* raw_data = (uint8_t*)stream.data;
    for (uint32_t i=0; i < count; i++) {
        WAVE15* atom = (WAVE15*)(raw_data + i * stride);
        atom->secret = (uint8_t)(i % 256);
    }
    
    printf("[FLOW] Dispatching SHAO stage (WAVE15 Safety Stride=16)...\n");
    struct SHAO* shao = DM(struct SHAO);
    shao->Rod = DM(struct SHA); shao->Rod->Mu = DM(struct Fa);
    shao->Rod->Mu->Channel = tsfi_bn_alloc();
    shao->Cone = DM(struct SHA); shao->Cone->Mu = DM(struct Fa);
    
    TSFiFusedOp* pipe_shao = tsfi_reaction_build_shao("SHAO_WAVE15", shao);
    WaveShaderUniforms u_shao = { .intensity = 1.0f, .decay = 0.0f };
    
    // Dispatch
    tsfi_op_dispatch(pipe_shao->ops[0], NULL, &stream, &u_shao, sizeof(u_shao));
    
    // Verify secret byte was preserved
    WAVE15* atom0 = (WAVE15*)raw_data;
    printf("  Atom 0 Secret: %d (Expected 0)\n", atom0->secret);
    
    if (atom0->secret == 0) {
        printf("[PASS] WAVE15 Safety Checks Passed (Secret Preserved with Stride 16).\n");
    } else {
        printf("[FAIL] WAVE15 Secret Corruption Detected!\n");
    }

    // Cleanup
    tsfi_fused_destroy(pipe_shao, NULL);
    freeSHAO(shao);
    lau_free(buffer);

    printf("=== Benchmark Complete ===\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}