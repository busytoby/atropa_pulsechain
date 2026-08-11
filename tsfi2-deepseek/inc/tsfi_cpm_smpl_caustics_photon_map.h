/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_CAUSTICS_PHOTON_MAP_H
#define TSFI_CPM_SMPL_CAUSTICS_PHOTON_MAP_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_caustics_photon_map_init(uint32_t photon_count);
bool tsfi_cpm_smpl_caustics_photon_map_eval(uint64_t zmm_rig_latch, uint32_t photon_count, uint64_t *zmm_photon_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_CAUSTICS_PHOTON_MAP_H */
