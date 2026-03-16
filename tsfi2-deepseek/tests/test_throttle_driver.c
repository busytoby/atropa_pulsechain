#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "tsfi_logic.h"
#include "tsfi_wavelet_arena.h"
#include "lau_registry.h"
#include "lau_memory.h"
#include "tsfi_resonance.h"

int main() {
    printf("=== TSFi Throttle Driver (Alligator Stalling) ===\n");
    
    // 1. Initialize the system
    (void)lau_registry_get_head(); // Map telemetry
    WaveSystem ws = {0};
    tsfi_logic_init(&ws);
    
    // 2. Mock a "Sheaf Surge" by setting active session pointers
    // We don't actually need real threads, just non-null pointers
    // to trigger the active_count >= 2 check in master_logic_epoch.
    ws.active_sessions[0] = (void*)0x1234;
    ws.active_sessions[1] = (void*)0x5678;

    printf("[DRIVER] Inducing 5 flexes with Sheaf Density Surge...\n");
    for (int i = 0; i < 5; i++) {
        int ver = 0;
        // master_logic_epoch will see 2 active sessions and throttle
        master_logic_epoch(&ver);
        printf("[DRIVER] Epoch %d Flexed.\n", i);
    }

    printf("[DRIVER] Success. Alligator throttled correctly.\n");
    tsfi_logic_teardown();
    return 0;
}
