#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_box_orchestrator.h"

TSFiBoxOrchestrator* tsfi_box_orchestrator_create(tsfi_dat *disk_dat, tsfi_trie_node *trie_root) {
    if (!trie_root) return NULL;
    TSFiBoxOrchestrator *orc = calloc(1, sizeof(TSFiBoxOrchestrator));
    if (!orc) return NULL;
    orc->trie_root = trie_root;
    orc->akb = tsfi_akb_create(disk_dat);
    tsfi_ot_accumulator_init(&orc->ot_accumulator);
    return orc;
}

int tsfi_box_orchestrator_step(TSFiBoxOrchestrator *orc, int x_start, int x_end, int y, int z,
                               const char *hyp_path, const char *ev_path) {
    if (!orc || !orc->akb) return -1;

    // 1. Blue Box: Load prior weights into AKB cache
    tsfi_akb_write(orc->akb, hyp_path, "60"); // prior P(H) = 60%
    tsfi_akb_write(orc->akb, ev_path, "50");  // prior P(E) = 50%
    
    char joint_path[256];
    snprintf(joint_path, sizeof(joint_path), "query/joint/%s/%s", hyp_path, ev_path);
    tsfi_akb_write(orc->akb, joint_path, "40"); // P(H AND E) = 40%

    // 2. Yellow Box: Scan coordinate range for strain boundaries
    TSFiUnifiedVoxelState classification = tsfi_unified_classify_region(orc->akb->disk_dat, x_start, x_end, y, z, 
                                                                        hyp_path, ev_path);

    if (classification.strain_class == CLASS_FRONTIER) {
        printf("  [Orchestrator] Yellow Box: CLASS_FRONTIER strain detected. Triggering logic check...\n");
        
        // 3. Green Box: Spawn Markov Logic VM constraint verification script
        TSFiMarkovVM vm;
        memset(&vm, 0, sizeof(TSFiMarkovVM));

        // Evaluate target limit based on Bayesian posterior (80% mapped to 80 weight value)
        int target_sat_limit = (int)(classification.posterior_probability * 100.0f);
        
        int bytecode[11] = {
            1, 50,              // weight 0.50
            0x31,
            1, 40,              // weight 0.40
            0x31,
            1, target_sat_limit, // sat limit
            0x32,
            6
        };

        tsfi_markov_vm_execute(&vm, bytecode, 11);

        if (vm.stack_len > 0 && vm.stack[0] == 1) {
            // 4. Red Box: Constraints satisfied -> Crystallize fact to cache and sync to disk
            printf("  [Orchestrator] Red Box: Logic satisfied. Crystallizing relation coordinates...\n");
            
            for (int x = x_start; x <= x_end; x++) {
                char coord_key[128];
                snprintf(coord_key, sizeof(coord_key), "svdag/%d/%d/%d", x, y, z);
                tsfi_akb_write(orc->akb, coord_key, "RELATION_TRUE");
            }

            tsfi_akb_sync(orc->akb, orc->trie_root);
            orc->resolved_count++;
            return 1; // Resolved
        } else {
            // Accumulate undecidable coordinates in OT Accumulator
            for (int x = x_start; x <= x_end; x++) {
                char coord_key[128];
                snprintf(coord_key, sizeof(coord_key), "svdag/%d/%d/%d", x, y, z);
                tsfi_ot_accumulator_add(&orc->ot_accumulator, coord_key, classification.posterior_probability);
            }
            orc->failed_count++;
            return 0; // Constraints failed
        }
    }

    return 2; // Stable, no step needed
}

void tsfi_box_orchestrator_destroy(TSFiBoxOrchestrator *orc) {
    if (!orc) return;
    if (orc->akb) tsfi_akb_destroy(orc->akb);
    free(orc);
}
