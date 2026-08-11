/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_E2E_VM_RUNNER_PIPELINE_H
#define TSFI_CPM_E2E_VM_RUNNER_PIPELINE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_e2e_vm_runner_pipeline_init(uint32_t vm_id);
bool tsfi_cpm_e2e_vm_runner_pipeline_eval(uint64_t zmm_rig_latch, uint32_t step_count, uint64_t *zmm_vm_latch_out);
bool tsfi_cpm_e2e_vm_runner_pipeline_render_renderman(uint64_t zmm_vm_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_E2E_VM_RUNNER_PIPELINE_H */
