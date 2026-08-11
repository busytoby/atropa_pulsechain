/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_VOLUMETRIC_LIGHT_SHAFT_MARCHER_H
#define TSFI_CPM_SMPL_VOLUMETRIC_LIGHT_SHAFT_MARCHER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_volumetric_light_shaft_marcher_init(uint32_t march_step_count);
bool tsfi_cpm_smpl_volumetric_light_shaft_marcher_eval(uint64_t zmm_rig_latch, uint32_t march_step_count, uint64_t *zmm_shaft_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_VOLUMETRIC_LIGHT_SHAFT_MARCHER_H */
