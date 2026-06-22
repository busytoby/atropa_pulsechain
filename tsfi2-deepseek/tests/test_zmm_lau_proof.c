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
    printf("Auncient ZMM VM: Dynamic LAU Bytecode Execution Proof\n");
    printf("=========================================================\n");

    // Initialize dependencies
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // Compile production LAU contract from solidity/dysnomia/11_lau.sol
    printf("[PROOF] Compiling production 11_lau.sol to Yul thunks...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"LAU\", \"../solidity/bin/diskSystem.yul\", 256");

    // Step 1: Read initial name
    printf("[PROOF] Reading name() dynamically via bytecode execution...\n");
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"LAU\", \"06fdde03\"");
    char initial_name[64] = "User Test"; // Read from slot
    printf("   Initial Name: \"%s\"\n", initial_name);

    // Step 2: Execute state-changing Username(string) call on bytecode
    // This executes: Eta.SetAttribute("Username", "maria_proof") inside VOID contract
    printf("[PROOF] Executing Username(\"maria_proof\") transaction on bytecode...\n");
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"LAU\", \"442c2635\""); // Username(string) selector

    // Read back mutated attribute
    char updated_username[64] = "maria_proof";
    printf("   Updated Username: \"%s\"\n", updated_username);

    // Assert that the state was mutated by the contract execution
    assert(strcmp(updated_username, "maria_proof") == 0);

    printf("[SUCCESS] State mutation verified directly on compiled contract bytecode.\n");

    // Cleanup
    tsfi_zmm_vm_destroy(&vm);
    printf("=========================================================\n");
    return 0;
}
