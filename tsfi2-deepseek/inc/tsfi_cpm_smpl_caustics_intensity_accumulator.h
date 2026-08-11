/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_CAUSTICS_INTENSITY_ACCUMULATOR_H
#define TSFI_CPM_SMPL_CAUSTICS_INTENSITY_ACCUMULATOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_caustics_intensity_accumulator_init(uint32_t grid_res);
bool tsfi_cpm_smpl_caustics_intensity_accumulator_eval(uint64_t zmm_rig_latch, uint32_t grid_res, uint64_t *zmm_caustic_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_CAUSTICS_INTENSITY_ACCUMULATOR_H */
