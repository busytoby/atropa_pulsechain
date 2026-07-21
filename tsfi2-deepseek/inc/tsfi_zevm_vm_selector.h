#ifndef TSFI_ZEVM_VM_SELECTOR_H
#define TSFI_ZEVM_VM_SELECTOR_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    ZEVM_VM_MODE_STANDARD  = 0,
    ZEVM_VM_MODE_LYNCH     = 1, // Preemptive Working-Set Executive
    ZEVM_VM_MODE_LOGAN     = 2, // 512-Sample Logarithmic Wavetable
    ZEVM_VM_MODE_HELMHOLTZ = 3  // Universal Isomorphic Wave Operator
} tsfi_zevm_vm_mode_t;

typedef struct {
    tsfi_zevm_vm_mode_t active_mode;
    char mode_name[32];
    char contract_addr[43];
    uint32_t active_pcb_or_channel;
} tsfi_zevm_vm_config_t;

/* Select and boot a specialized kernel VM choice on z/EVMn and System/370 Mainframe */
int tsfi_zevm_select_vm_kernel(tsfi_zevm_vm_mode_t mode, tsfi_zevm_vm_config_t *out_config);

#endif // TSFI_ZEVM_VM_SELECTOR_H
