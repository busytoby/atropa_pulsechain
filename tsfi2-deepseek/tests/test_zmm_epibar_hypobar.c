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

// Hex helper for uint256 big-endian words
void wmq_fill_hex_uint256(char *dest, uint64_t val) {
    memset(dest, '0', 64);
    char tmp[32];
    int len = snprintf(tmp, sizeof(tmp), "%lx", val);
    memcpy(dest + 64 - len, tmp, len);
    dest[64] = '\0';
}

// ZMM WinchesterMQ sstore selector using Mount ("4d6f756e")
void wmq_sstore(TsfiZmmVmState *vm, uint64_t slot, uint64_t val) {
    char cmd[512];
    char hex_lun[65];
    char hex_val[65];
    
    if (slot >= 0x5000) {
        wmq_fill_hex_uint256(hex_lun, slot - 0x5000);
    } else {
        uint64_t diff = 0x5000 - slot;
        memset(hex_lun, 'f', 48);
        char tmp[32];
        int len = snprintf(tmp, sizeof(tmp), "%lx", (uint64_t)(0 - diff));
        memcpy(hex_lun + 64 - len, tmp, len);
        for (int i = 48; i < 64 - len; i++) {
            hex_lun[i] = 'f';
        }
        hex_lun[64] = '\0';
    }
    wmq_fill_hex_uint256(hex_val, val);
    
    snprintf(cmd, sizeof(cmd), "YULEXEC \"WinchesterMQ\", \"4d6f756e%s%s\"", hex_lun, hex_val);
    tsfi_zmm_vm_exec(vm, cmd);
}

// ZMM WinchesterMQ sload selector using Gount ("476f756e")
uint64_t wmq_sload(TsfiZmmVmState *vm, uint64_t slot) {
    char cmd[512];
    char hex_lun[65];
    
    if (slot >= 0x5000) {
        wmq_fill_hex_uint256(hex_lun, slot - 0x5000);
    } else {
        uint64_t diff = 0x5000 - slot;
        memset(hex_lun, 'f', 48);
        char tmp[32];
        int len = snprintf(tmp, sizeof(tmp), "%lx", (uint64_t)(0 - diff));
        memcpy(hex_lun + 64 - len, tmp, len);
        for (int i = 48; i < 64 - len; i++) {
            hex_lun[i] = 'f';
        }
        hex_lun[64] = '\0';
    }
    
    snprintf(cmd, sizeof(cmd), "YULEXEC \"WinchesterMQ\", \"476f756e%s\"", hex_lun);
    vm->output_pos = 0;
    memset(vm->output_buffer, 0, sizeof(vm->output_buffer));
    tsfi_zmm_vm_exec(vm, cmd);
    
    uint64_t val = 0;
    if (strlen(vm->output_buffer) >= 64) {
        sscanf(vm->output_buffer + 48, "%16lx", &val);
    }
    return val;
}

int main() {
    printf("=========================================================\n");
    printf("Auncient ZMM VM Epibar & Hypobar Storage Constraints Test\n");
    printf("=========================================================\n");

    // Initialize dependencies
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // Compile WinchesterMQ Yul contract on ZMM VM
    printf("[VM INIT] Compiling WinchesterMQ.yul onto ZMM VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"WinchesterMQ\", \"../solidity/bin/WinchesterMQ.yul\", 512");

    // Set Epibar (Storage Slot 103) and Hypobar (Storage Slot 104) inside VM
    uint64_t epibar_val = 1000;  // Epibar constraint coefficient (1.000)
    uint64_t hypobar_val = 2300; // Hypobar constraint coefficient (2.300)

    printf("[ZMM TEST] Writing Epibar to Slot 103: %lu...\n", epibar_val);
    wmq_sstore(&vm, 103, epibar_val);

    printf("[ZMM TEST] Writing Hypobar to Slot 104: %lu...\n", hypobar_val);
    wmq_sstore(&vm, 104, hypobar_val);

    // Read back values to verify ZMM VM storage preservation
    uint64_t loaded_epibar = wmq_sload(&vm, 103);
    uint64_t loaded_hypobar = wmq_sload(&vm, 104);

    printf("[ZMM TEST] Verification:\n");
    printf("   Loaded Epibar: %lu (Expected: %lu)\n", loaded_epibar, epibar_val);
    printf("   Loaded Hypobar: %lu (Expected: %lu)\n", loaded_hypobar, hypobar_val);

    assert(loaded_epibar == epibar_val);
    assert(loaded_hypobar == hypobar_val);

    printf("[SUCCESS] ZMM VM storage variables mapped and verified stably.\n");

    // Cleanup
    tsfi_zmm_vm_destroy(&vm);
    printf("=========================================================\n");
    return 0;
}
