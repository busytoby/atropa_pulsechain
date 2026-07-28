#include <stdio.h>
#include <string.h>
#include "tsfi_rau_thunk.h"

// Simulated hardware register array inside the running VM context
static uint64_t simulated_rau_virtual_registers[32] = {0};

int tsfi_rau_thunk_dump_registers(uint32_t core_id, uint64_t *out_reg_bank_32) {
    if (!out_reg_bank_32) return -1;

    printf("[RAU THUNK] Initiating bulk register dump for Core %u over WinchesterMQ...\n", core_id);
    printf("  -> Sending Selector 0x077a4189 (dump_registers) to WMQ\n");

    // Copy the entire simulated 32-word virtual register bank to the destination buffer
    memcpy(out_reg_bank_32, simulated_rau_virtual_registers, sizeof(uint64_t) * 32);
    return 0;
}

int tsfi_rau_thunk_load_registers(uint32_t core_id, const uint64_t *in_reg_bank_32) {
    if (!in_reg_bank_32) return -1;

    printf("[RAU THUNK] Initiating bulk register load for Core %u over WinchesterMQ...\n", core_id);
    printf("  -> Sending Selector 0x088b529a (load_registers) to WMQ\n");

    // Copy the source buffer into the simulated virtual register bank
    memcpy(simulated_rau_virtual_registers, in_reg_bank_32, sizeof(uint64_t) * 32);
    return 0;
}
