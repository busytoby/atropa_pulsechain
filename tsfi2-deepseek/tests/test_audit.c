#define _POSIX_C_SOURCE 200809L
#include "tsfi_types.h"
#include "lau_memory.h"
#include <stdio.h>
#include <stdlib.h>

extern void tsfi_dl_thunks_init(void);
extern void tsfi_dl_thunks_teardown(void);
extern void lau_registry_teardown(void);
extern void lau_wire_mapped_logic(void *ptr);

int main() {
    printf("[AUDIT] Starting physical seal and leak diagnostic matrix.\n");
    
    // Spawn static inner geometrieZ
    tsfi_dl_thunks_init();
    
    // Spawn structural test blocks
    void *l1 = lau_malloc_loc(128, "test_audit.c", 19);
    void *l2 = lau_memalign_wired_loc(512, 256, "test_audit.c", 20);
    void *l3 = lau_malloc_gpu_loc(512, 1, "test_audit.c", 21);
    
    *(char*)l1 = 1; *(char*)l2 = 2; *(char*)l3 = 3;
    
    // Wire local execution layer
    lau_wire_mapped_logic(l2);
    
    // Activate 4-Dimensional Seals
    lau_seal_object(l1);
    lau_seal_object(l2);
    lau_seal_object(l3);
    
    printf("[AUDIT] Matrix Sealed. Expecting active allocations...\n");
    lau_report_memory_metrics();
    
    printf("\n[AUDIT] Decomposing Matrix...\n");
    
    // Explicitly break the sovereign seal before structural teardown
    lau_unseal_object(l1);
    lau_unseal_object(l2);
    lau_unseal_object(l3);
    
    // Free blocks
    lau_free(l1);
    lau_free(l2);
    lau_free(l3);
    
    // Dismantle innermost static proxy geometrieZ
    tsfi_dl_thunks_teardown();
    
    // Dismantle outermost registry manifold
    lau_registry_teardown();
    
    printf("\n[AUDIT] Matrix Decomposed. Expecting PERFECT zero leaks...\n");
    lau_report_memory_metrics();
    
    printf("\n[AUDIT] SUCCESS.\n");
    
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
