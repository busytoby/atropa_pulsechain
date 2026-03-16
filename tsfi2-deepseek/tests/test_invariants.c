#include "lau_memory.h"
#include "lau_thunk.h"
#include "tsfi_wiring.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

// Invariant 1: 512-byte Alignment
int verify_alignment(void *ptr, const char *name) {
    if (((uintptr_t)ptr % 512) != 0) {
        fprintf(stderr, "[FAIL] %s: Bad Alignment. Addr: %p (Mod 512 = %lu)\n", 
                name, ptr, (uintptr_t)ptr % 512);
        return 0;
    }
    return 1;
}

// Invariant 2: Header Integrity
int verify_header(void *ptr, const char *name) {
    // Assume Wired Header for architectural candidates
    LauWiredHeader *h = (LauWiredHeader *)((char *)ptr - offsetof(LauWiredHeader, payload));
    
    if (h->footer.magic != LAU_MAGIC) {
        fprintf(stderr, "[FAIL] %s: Bad Magic. Expected 0x%X, Got 0x%X\n", 
                name, LAU_MAGIC, h->footer.magic);
        return 0;
    }
    
    if (h->footer.type != LAU_TYPE_WIRED) {
        fprintf(stderr, "[FAIL] %s: Bad Type. Expected WIRED (0x20), Got 0x%X\n", 
                name, h->footer.type);
        return 0;
    }
    
    return 1;
}

// Invariant 3: Thunk Wiring
int verify_wiring(void *ptr, const char *name) {
    LauWiredHeader *h = (LauWiredHeader *)((char *)ptr - offsetof(LauWiredHeader, payload));
    
    if (!h->proxy) {
        fprintf(stderr, "[FAIL] %s: Missing ThunkProxy.\n", name);
        return 0;
    }
    
    if (!h->logic_epoch) {
        fprintf(stderr, "[FAIL] %s: Missing Logic Vectors (Epoch).\n", name);
        return 0;
    }
    
    return 1;
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    printf(">>> INVARIANT TEST SUITE <<<\n");
    
    // 1. Create a Reference System (The Control)
    WaveSystem *sys = tsfi_create_system();
    if (!sys) {
        fprintf(stderr, "[CRITICAL] Failed to allocate Reference System.\n");
        return 1;
    }
    
    int score = 0;
    score += verify_alignment(sys, "Reference_System");
    score += verify_header(sys, "Reference_System");
    score += verify_wiring(sys, "Reference_System");
    
    // Cleanup
    lau_final_cleanup(sys, -1);
    
    if (score == 3) {
        printf("[PASS] Reference System adheres to all invariants.\n");
            extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
    } else {
        printf("[FAIL] System Integrity Violation.\n");
        return 1;
    }
}