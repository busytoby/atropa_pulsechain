#include "tsfi_wiring.h"
#include "tsfi_types.h"
#include "lau_memory.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int main() {
    puts("--- RUNNING SCRAMBLE SAFETY TEST ---");
    
    // 1. Initialize System
    WaveSystem *ws = tsfi_create_system();
    if (!ws) {
        fprintf(stderr, "Failed to create system\n");
        return 1;
    }
    
    // 2. Set "User Data" (Post-MappedCommon fields)
    ws->system_id = 0x12345678;
    ws->current_intensity = 3.14159;
    
    // 3. Set "Interface Data" (MappedCommon fields - pointed to header)
    *ws->version = 100;
    
    printf("[TEST] Before Scramble: ID=0x%x, Intensity=%f, Ver=%d\n", 
           ws->system_id, ws->current_intensity, *ws->version);
    
    // 4. Execute Scramble
    ws->scramble();
    
    printf("[TEST] After Scramble:  ID=0x%x, Intensity=%f, Ver=%d\n", 
           ws->system_id, ws->current_intensity, *ws->version);
    
    // 5. Verification
    
    // A. Interface Preservation
    if (*ws->version != 100) {
        fprintf(stderr, "[FAIL] Scramble corrupted Interface Data (Version changed)!\n");
        return 1;
    }
    
    // B. Payload Randomization
    // Note: There is a tiny chance random bytes match original. Ignoring for simplicity.
    if (ws->system_id == 0x12345678) {
        fprintf(stderr, "[FAIL] Scramble did NOT randomize system_id!\n");
        return 1;
    }
    
    if (ws->current_intensity == 3.14159) {
        fprintf(stderr, "[FAIL] Scramble did NOT randomize intensity!\n");
        return 1;
    }
    
    puts("[PASS] Scramble randomized payload while protecting interface.");
    
    lau_free(ws);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
