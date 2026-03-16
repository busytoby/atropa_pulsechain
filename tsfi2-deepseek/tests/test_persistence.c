#include "tsfi_wiring.h"
#include "tsfi_hotloader.h"
#include "lau_memory.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

int main() {
    puts("--- RUNNING PERSISTENCE TEST ---");
    
    // 1. Initialize System
    WaveSystem *ws = tsfi_create_system();
    if (!ws) {
        fprintf(stderr, "Failed to create system\n");
        return 1;
    }
    
    // Verify Initial State
    if (*ws->version != 0) {
        fprintf(stderr, "FAIL: Initial version not 0\n");
        return 1;
    }

    // 2. Compile Plugins
    if (tsfi_compile_plugin("plugins/plugin_a.c", "plugins/plugin_a.so") != 0) {
        fprintf(stderr, "FAIL: Compile A\n");
        return 1;
    }
    if (tsfi_compile_plugin("plugins/plugin_b.c", "plugins/plugin_b.so") != 0) {
        fprintf(stderr, "FAIL: Compile B\n");
        return 1;
    }

    // 3. Load Plugin A
    puts("[TEST] Loading Plugin A...");
    TSFiLogicTable logic_a;
    if (tsfi_load_plugin("plugins/plugin_a.so", &logic_a) != 0) return 1;
    lau_update_logic(ws, &logic_a);
    
    // 4. Execute Epoch (A sets to 100)
    ws->step_safety_epoch();
    printf("[TEST] Version after A: %d\n", *ws->version);
    ws->provenance(); // Verify symbol resolution
    
    if (*ws->version != 100) {
        fprintf(stderr, "FAIL: Plugin A did not set version to 100.\n");
        return 1;
    }

    // 5. Load Plugin B
    puts("[TEST] Loading Plugin B...");
    TSFiLogicTable logic_b;
    if (tsfi_load_plugin("plugins/plugin_b.so", &logic_b) != 0) return 1;
    lau_update_logic(ws, &logic_b);
    
    // 6. Execute Epoch (B adds 50)
    ws->step_safety_epoch();
    printf("[TEST] Version after B: %d\n", *ws->version);
    ws->provenance(); // Verify symbol resolution
    
    // 7. Verification
    if (*ws->version == 150) {
        puts("[PASS] State persisted across hot-swap (100 -> 150).");
    } else {
        printf("[FAIL] State lost or incorrect logic. Expected 150, got %d\n", *ws->version);
        return 1;
    }

        extern void lau_final_cleanup(WaveSystem*, int);
        lau_unseal_object(ws);
        lau_final_cleanup(ws, 3);
    lau_report_memory_metrics();
    return 0;
}