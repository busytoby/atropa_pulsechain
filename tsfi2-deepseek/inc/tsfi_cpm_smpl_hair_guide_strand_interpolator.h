/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_HAIR_GUIDE_STRAND_INTERPOLATOR_H
#define TSFI_CPM_SMPL_HAIR_GUIDE_STRAND_INTERPOLATOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_hair_guide_strand_interpolator_init(uint32_t dense_strand_count);
bool tsfi_cpm_smpl_hair_guide_strand_interpolator_eval(uint64_t zmm_rig_latch, uint32_t dense_strand_count, uint64_t *zmm_interp_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_HAIR_GUIDE_STRAND_INTERPOLATOR_H */
