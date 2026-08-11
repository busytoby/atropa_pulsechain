/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_TOMIE_DEEPSEEK_E2E_VM_RUNNER_H
#define TSFI_CPM_TOMIE_DEEPSEEK_E2E_VM_RUNNER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_tomie_deepseek_e2e_vm_step_together(uint32_t active_vm_count, uint64_t step_counter, uint64_t *zmm_vm_sync_hash_out);
bool tsfi_cpm_tomie_deepseek_e2e_vm_verify_acid(uint32_t active_vm_count, bool *acid_4layer_pass_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_TOMIE_DEEPSEEK_E2E_VM_RUNNER_H */
