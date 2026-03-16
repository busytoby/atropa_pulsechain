#include "tsfi_wiring.h"
#include "tsfi_types.h"
#include "lau_memory.h"
#include "tsfi_wire_firmware.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    puts("--- REPRODUCING SCRAMBLE POINTER CORRUPTION ---");
    
    WaveSystem *ws = tsfi_create_system();
    if (!ws) {
        return 1;
    }
    
    // Set a pointer that must NOT be scrambled
    char *original_ptr = lau_strdup("PROTECT_ME");
    ws->current_directive = original_ptr;
    
    printf("[BEFORE] current_directive: %p (\"%s\")\n", (void*)ws->current_directive, ws->current_directive);
    
    // Execute scramble
    ws->scramble();
    
    printf("[AFTER]  current_directive: %p\n", (void*)ws->current_directive);
    
    if (ws->current_directive != original_ptr) {
        fprintf(stderr, "[FAIL] Pointer corrupted! Expected %p, got %p\n", (void*)original_ptr, (void*)ws->current_directive);
        return 1;
    }
    
    printf("[PASS] Pointer preserved. String: \"%s\"\n", ws->current_directive);
    
    lau_free(ws->current_directive);
    ws->current_directive = NULL;
    
    // Final cleanup - lau_free handles proxy and status
    LauSystemHeader *h = (LauSystemHeader *)((char *)ws - offsetof(LauSystemHeader, payload));
    if (h->resonance_as_status) {
        lau_free(h->resonance_as_status);
        h->resonance_as_status = NULL;
    }
    
    LauWireFirmware *fw = tsfi_wire_firmware_get();
    if (fw) {
        fw->rtl.cell_resonance_status_ptr = 0;
    }
    
    lau_unseal_object(ws);
    lau_free(ws);

    LauWireFirmware *fw2 = tsfi_wire_firmware_get_no_init();
    if (fw2) {
        lau_unseal_object(fw2);
    }

    extern void tsfi_wire_firmware_teardown(void);
    tsfi_wire_firmware_teardown();
    
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    
    puts("DONE");
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}