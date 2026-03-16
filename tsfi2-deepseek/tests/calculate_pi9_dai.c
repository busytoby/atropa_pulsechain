#include "tsfi_reaction.h"
#include "tsfi_io.h"
#include "lau_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main() {
    lau_memory_init_gpu(NULL);
    srand(time(NULL));
    
    TSFiBigInt *P = tsfi_bn_alloc();
    tsfi_bn_set_u64(P, 953467954114363ULL);

    struct YI* yi_batch[8] = {NULL};
    struct Dai* result = NULL;

    // 1. Perform fused SHOOT batch
    tsfi_reaction_shoot_batch8_fused(yi_batch, P);
    
    // 2. We use the first lane and force pi=9 into the Ring
    struct YI* yi = yi_batch[0];
    tsfi_bn_set_u64(yi->Ring, 9);

    // 3. Form Reciprocity DAI
    result = ReactSHIO_bn_reused(NULL, yi->Psi, yi->Ring);

    uint8_t res_bytes[256];
    tsfi_bn_to_bytes(result->Ichidai, res_bytes, 256);
    
    printf("Reaction DAI Ichidai (pi=9): ");
    for(int i=0; i<32; i++) printf("%02x", res_bytes[i]);
    printf("\n");

    for(int i=0; i<8; i++) if(yi_batch[i]) freeYI(yi_batch[i]);
    freeDAI(result);
    tsfi_bn_free(P);
    lau_free_all_active();
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}
