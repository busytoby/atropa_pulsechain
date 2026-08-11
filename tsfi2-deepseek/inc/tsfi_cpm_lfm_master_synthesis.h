/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_LFM_MASTER_SYNTHESIS_H
#define TSFI_CPM_LFM_MASTER_SYNTHESIS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_lfm_master_synthesis_init(uint32_t synthesis_id);
bool tsfi_cpm_lfm_master_synthesis_eval(uint64_t zmm_rig_latch, uint32_t synthesis_id, uint64_t *zmm_syn_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_LFM_MASTER_SYNTHESIS_H */
