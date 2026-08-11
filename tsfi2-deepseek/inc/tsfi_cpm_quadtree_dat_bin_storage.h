/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_QUADTREE_DAT_BIN_STORAGE_H
#define TSFI_CPM_QUADTREE_DAT_BIN_STORAGE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_quadtree_dat_bin_storage_init(uint32_t slice_id);
bool tsfi_cpm_quadtree_dat_bin_storage_eval(uint64_t zmm_rig_latch, uint32_t slice_id, uint64_t *zmm_dat_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_QUADTREE_DAT_BIN_STORAGE_H */
