/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_ATMOSPHERE_MIE_AEROSOL_H
#define TSFI_CPM_SMPL_ATMOSPHERE_MIE_AEROSOL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_atmosphere_mie_aerosol_init(uint32_t turbidity);
bool tsfi_cpm_smpl_atmosphere_mie_aerosol_eval(uint64_t zmm_rig_latch, uint32_t turbidity, uint64_t *zmm_mie_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_ATMOSPHERE_MIE_AEROSOL_H */
