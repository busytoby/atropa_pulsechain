/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_LIGHTING_SPECULAR_BOUNCE_H
#define TSFI_CPM_SMPL_LIGHTING_SPECULAR_BOUNCE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_lighting_specular_bounce_init(uint32_t light_count);
bool tsfi_cpm_smpl_lighting_specular_bounce_eval(uint64_t zmm_rig_latch, uint32_t light_count, uint64_t *zmm_specular_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_LIGHTING_SPECULAR_BOUNCE_H */
