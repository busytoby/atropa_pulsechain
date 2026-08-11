/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_MATERIAL_SKIN_EPIDERMIS_LAYER_H
#define TSFI_CPM_SMPL_MATERIAL_SKIN_EPIDERMIS_LAYER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_material_skin_epidermis_layer_init(uint32_t melanin_concentration);
bool tsfi_cpm_smpl_material_skin_epidermis_layer_eval(uint64_t zmm_rig_latch, uint32_t melanin_concentration, uint64_t *zmm_skin_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_MATERIAL_SKIN_EPIDERMIS_LAYER_H */
