#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_helmholtz.h"
#include "tsfi_io.h"
#include "lau_memory.h"

int main() {
    printf("=== TSFi Level 10 NAND Impact Test ===\n");
    tsfi_helmholtz_init();
    uint64_t resonance_k = 2026;

    // Allocate 256KB Manifold (enough for Level 10 reduction: 1024 leaves * 256 bytes = 256KB)
    void *manifold = lau_malloc(256 * 1024);
    memset(manifold, 0, 256 * 1024);

    uint8_t root_initial[32];
    uint8_t receipt_initial[32];
    
    // 1. Initial Reduction
    printf("[ACT] Performing initial Level 10 reduction (All Zero)...\n");
    tsfi_helmholtz_reduce_10(root_initial, receipt_initial, NULL, NULL, manifold, 1, resonance_k, NULL);
    
    printf("Initial State Root: ");
    for(int i=0; i<32; i++) printf("%02x", root_initial[i]);
    printf("\n");

    // 2. Flip NAND Trap (Leaf 500)
    printf("[ACT] Flipping NAND Trap Set_N (Leaf 500)...\n");
    uint8_t *leaf500 = (uint8_t *)manifold + (500 * 256);
    leaf500[0] = 1; // Trigger Set_N

    uint8_t root_set[32];
    tsfi_helmholtz_reduce_10(root_set, receipt_initial, NULL, NULL, manifold, 1, resonance_k, NULL);
    
    printf("Set_N State Root:   ");
    for(int i=0; i<32; i++) printf("%02x", root_set[i]);
    printf("\n");

    // 3. Verify Change
    if (memcmp(root_initial, root_set, 32) != 0) {
        printf("[PASS] NAND Trap correctly influenced the Level 10 State Root.\n");
    } else {
        printf("[FAIL] NAND Trap had no influence on the State Root.\n");
        return 1;
    }

    // 4. Flip NAND Trap Reset_N (Leaf 501)
    printf("[ACT] Flipping NAND Trap Reset_N (Leaf 501)...\n");
    uint8_t *leaf501 = (uint8_t *)manifold + (501 * 256);
    leaf501[0] = 1; // Trigger Reset_N (Metastability 0,0 state simulation)

    uint8_t root_metastable[32];
    tsfi_helmholtz_reduce_10(root_metastable, receipt_initial, NULL, NULL, manifold, 1, resonance_k, NULL);
    
    printf("Metastable Root:    ");
    for(int i=0; i<32; i++) printf("%02x", root_metastable[i]);
    printf("\n");

    if (memcmp(root_set, root_metastable, 32) != 0) {
        printf("[PASS] Metastability transition reflected in the Root.\n");
    } else {
        printf("[FAIL] Metastability transition had no influence.\n");
        return 1;
    }

    lau_free(manifold);
    tsfi_helmholtz_cleanup();
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
