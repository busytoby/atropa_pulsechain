#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

int main() {
    printf("=== TSFi ZMM VM Yul Integration Test ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Initialize Yul system for genesis.yul via VM text commands
    // YULINIT "genesis", "../solidity/bin/genesis.yul", 1
    printf("[TEST 1] Initializing Genesis Yul Contract via ZMM VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"genesis\", \"../solidity/bin/genesis.yul\", 1");
    printf("PASS\n");

    // 2. Execute getComplexity() on genesis contract
    // Selector for getComplexity() is 0x3df0a09e
    // Calldata is 3df0a09e
    printf("[TEST 2] Executing getComplexity() on Genesis Yul Contract via ZMM VM...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"genesis\", \"3df0a09e\"");
    
    printf("Output Hex: %s\n", vm.output_buffer);
    
    // The complexity returned should be 8, which in 32-byte big-endian hex is:
    // 0000000000000000000000000000000000000000000000000000000000000008
    const char *expected = "0000000000000000000000000000000000000000000000000000000000000008";
    assert(strcmp(vm.output_buffer, expected) == 0);
    printf("PASS: Correct complexity returned (8).\n");

    tsfi_zmm_vm_destroy(&vm);
    printf("=== ALL ZMM VM YUL TESTS PASSED ===\n");
    return 0;
}
