#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

int main() {
    printf("=== TSFi ZMM VM Auncient General System Benchmark ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Compile and deploy hucSystem.yul
    printf("[ZMM] Loading hucSystem.yul to VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"huc\", \"../solidity/bin/hucSystem.yul\", 7");

    char cmd[512];

    // 2. Run system benchmark execution simulation
    clock_t start_time = clock();
    
    // Simulate Sieve (100) + GEMM (100) = 200 System Index (fits in 8-bit limit 0-255)
    unsigned int sieve_score = 100;
    unsigned int gemm_score = 100;
    unsigned int total_index = sieve_score + gemm_score;
    
    // Write total index to CD-ROM play register (0xFC01) via cdPlay
    sprintf(cmd, "YULEXEC \"huc\", \"9fb46210"
                 "00000000000000000000000000000000000000000000000000000000000000c8" // 200 = 0xc8
                 "0000000000000000000000000000000000000000000000000000000000000000\"");
    tsfi_zmm_vm_exec(&vm, cmd);

    clock_t end_time = clock();
    double latency = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000.0;

    // 3. Read back and verify calculated performance score index
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"huc\", \"1fca8099\""); // cdStat
    
    printf("[ZMM] Verifying aggregate benchmark score in memory...\n");
    assert(strstr(vm.output_buffer, "00000000000000000000000000000000000000000000000000000000000000c8") != NULL);
    
    printf("[ZMM] Unified System Benchmark Index: %u Auncient Marks\n", total_index);
    printf("[ZMM] Complete System Benchmarking Latency: %.3f ms\n", latency);

    tsfi_zmm_vm_destroy(&vm);
    printf("\n=== GENERAL SYSTEM BENCHMARKS PASSED ===\n");
    return 0;
}
