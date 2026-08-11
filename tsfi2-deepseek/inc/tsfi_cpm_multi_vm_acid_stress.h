/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_MULTI_VM_ACID_STRESS_H
#define TSFI_CPM_MULTI_VM_ACID_STRESS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_multi_vm_acid_stress_init(uint32_t vm_count);
bool tsfi_cpm_multi_vm_acid_stress_eval(uint64_t zmm_rig_latch, uint32_t vm_count, uint64_t *zmm_acid_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_MULTI_VM_ACID_STRESS_H */
