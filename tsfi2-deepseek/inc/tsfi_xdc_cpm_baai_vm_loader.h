/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_XDC_CPM_BAAI_VM_LOADER_H
#define TSFI_XDC_CPM_BAAI_VM_LOADER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_xdc_cpm_baai_vm_load(uint32_t pasid, const char *model_path_dat_bin, uint64_t *zmm_vm_load_latch_out);
bool tsfi_xdc_cpm_baai_vm_step(uint32_t pasid, uint64_t zmm_vm_load_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_XDC_CPM_BAAI_VM_LOADER_H */
