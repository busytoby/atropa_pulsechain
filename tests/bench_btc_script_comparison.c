#include "inc/btc_rails_vm.h"
#include "firmware/tsfi_lib/tsfi_btc_thunk.c"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define ITERATIONS 2000000

void bench_jit_thunk() {
    printf("[Bench] Starting legacy JIT thunk loop...\n");
    fflush(stdout);
    
    BtcStack stack;
    uint8_t script[8];
    script[0] = 0x7c; // OP_SWAP
    script[1] = 0x7c; // OP_SWAP
    script[2] = 0x7c; // OP_SWAP
    script[3] = 0x7c; // OP_SWAP
    
    uint8_t val1[4] = {1, 2, 3, 4};
    uint8_t val2[4] = {5, 6, 7, 8};
    
    clock_t start = clock();
    for (int i = 0; i < ITERATIONS; i++) {
        stack_init(&stack);
        stack_push(&stack, val1, 4);
        stack_push(&stack, val2, 4);
        
        btc_thunk_execute(script, 4, &stack);
    }
    clock_t end = clock();
    double cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("[Bench] Legacy JIT Thunk: %.4f seconds (%d iterations)\n", cpu_time, ITERATIONS);
    fflush(stdout);
}

void bench_cia_rails_vm() {
    printf("[Bench] Starting dual-CIA Rails VM loop...\n");
    fflush(stdout);
    
    BtcRailsVm *vm = btc_rails_vm_init(65536);
    uint8_t val1[4] = {1, 2, 3, 4};
    uint8_t val2[4] = {5, 6, 7, 8};
    
    /* Simulate basic stack operations directly */
    clock_t start = clock();
    for (int i = 0; i < ITERATIONS; i++) {
        vm->cia1.timer_a_count = 0x8000;
        vm->cia1.timer_b_count = 0x0001;
        
        btc_rails_vm_push_ds(vm, val1, 4);
        btc_rails_vm_push_ds(vm, val2, 4);
        
        /* Emulate operations (pop, push) */
        uint8_t out1[4];
        uint8_t out2[4];
        btc_rails_vm_pop_ds(vm, out1, 4);
        btc_rails_vm_pop_ds(vm, out2, 4);
        
        btc_rails_vm_push_ds(vm, out1, 4);
        btc_rails_vm_push_ds(vm, out2, 4);
    }
    clock_t end = clock();
    double cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("[Bench] Dual-CIA Rails VM: %.4f seconds (%d iterations)\n", cpu_time, ITERATIONS);
    fflush(stdout);
    
    btc_rails_vm_free(vm);
}

int main() {
    printf("=== AUNCIENT BTC SCRIPT PERFORMANCE BENCHMARK ===\n");
    fflush(stdout);
    
    bench_jit_thunk();
    bench_cia_rails_vm();
    
    printf("=================================================\n");
    fflush(stdout);
    return 0;
}
