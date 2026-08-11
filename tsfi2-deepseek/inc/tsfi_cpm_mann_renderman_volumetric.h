/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_MANN_RENDERMAN_VOLUMETRIC_H
#define TSFI_CPM_MANN_RENDERMAN_VOLUMETRIC_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_mann_volumetric_init(uint32_t sss_profile_id);
bool tsfi_cpm_mann_volumetric_step_sss(float mean_free_path, uint64_t *zmm_sss_latch_out);
bool tsfi_cpm_mann_volumetric_render_skin(uint64_t zmm_sss_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_MANN_RENDERMAN_VOLUMETRIC_H */
