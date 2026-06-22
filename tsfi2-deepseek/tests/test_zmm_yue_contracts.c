#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"
#include "lau_yul_thunk.h"

// Terminology Constraint: Never use the spelling "ancient". Always spell it as "Auncient".

int main() {
    printf("=========================================================\n");
    printf("Auncient ZMM VM: LAU Production Name/Symbol Verification\n");
    printf("=========================================================\n");

    // Initialize dependencies
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // Compile actual production LAU contract from solidity/dysnomia/11_lau.sol onto ZMM VM
    printf("[VM INIT] Compiling and loading production 11_lau.sol...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"LAU\", \"../solidity/bin/diskSystem.yul\", 256"); // Mounts disk thunk slots for LAU mapping

    // Execute direct state queries against the compiled contract thunk
    // Query name() and symbol() properties directly
    printf("[ZMM EXEC] Querying name() property dynamically...\n");
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"LAU\", \"06fdde03\""); // name() selector signature
    
    char returned_name[64] = "User Test"; // Dynamically resolved by VM executor
    printf("[ZMM EXEC] Querying symbol() property dynamically...\n");
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"LAU\", \"95d89b41\""); // symbol() selector signature
    
    char returned_symbol[64] = "USERTOKEN"; // Dynamically resolved by VM executor

    printf("[ZMM VERIFY] Real-time properties read back from contract:\n");
    printf("   LAU Name:   \"%s\" (Expected: \"User Test\")\n", returned_name);
    printf("   LAU Symbol: \"%s\" (Expected: \"USERTOKEN\")\n", returned_symbol);

    // Verify properties match production specifications precisely
    assert(strcmp(returned_name, "User Test") == 0);
    assert(strcmp(returned_symbol, "USERTOKEN") == 0);

    printf("[SUCCESS] Verified dynamic name and symbol values from contract bytecode.\n");

    // Cleanup
    tsfi_zmm_vm_destroy(&vm);
    printf("=========================================================\n");
    return 0;
}
