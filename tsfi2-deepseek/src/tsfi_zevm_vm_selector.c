#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_zevm_vm_selector.h"

int tsfi_zevm_select_vm_kernel(tsfi_zevm_vm_mode_t mode, tsfi_zevm_vm_config_t *out_config) {
    if (!out_config) return -1;
    memset(out_config, 0, sizeof(tsfi_zevm_vm_config_t));

    out_config->active_mode = mode;

    switch (mode) {
        case ZEVM_VM_MODE_LYNCH:
            snprintf(out_config->mode_name, sizeof(out_config->mode_name), "LYNCH_EXECUTIVE_VM");
            snprintf(out_config->contract_addr, sizeof(out_config->contract_addr), "dynamic_0x370_lynch_vm");
            out_config->active_pcb_or_channel = 1; // PID 1 Root Kernel
            break;
        case ZEVM_VM_MODE_LOGAN:
            snprintf(out_config->mode_name, sizeof(out_config->mode_name), "LOGAN_RESONATOR_VM");
            snprintf(out_config->contract_addr, sizeof(out_config->contract_addr), "dynamic_0x370_logan_vm");
            out_config->active_pcb_or_channel = 512; // 512-Sample Wavetable
            break;
        case ZEVM_VM_MODE_HELMHOLTZ:
            snprintf(out_config->mode_name, sizeof(out_config->mode_name), "HELMHOLTZ_WAVE_VM");
            snprintf(out_config->contract_addr, sizeof(out_config->contract_addr), "dynamic_0x370_helm_vm");
            out_config->active_pcb_or_channel = 0; // Universal Channel 0
            break;
        case ZEVM_VM_MODE_STANDARD:
        default:
            snprintf(out_config->mode_name, sizeof(out_config->mode_name), "STANDARD_EVM_VM");
            snprintf(out_config->contract_addr, sizeof(out_config->contract_addr), "dynamic_0x370_evm_vm");
            out_config->active_pcb_or_channel = 0;
            break;
    }

    printf("[z/EVMn HYPERVISOR] Booted Mainframe VM Kernel Mode: %s -> Target Address: %s\n",
           out_config->mode_name, out_config->contract_addr);

    return 0; // VM booted successfully
}
