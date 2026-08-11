/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_MANN_RENDERMAN_ATMOSPHERE_H
#define TSFI_CPM_MANN_RENDERMAN_ATMOSPHERE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_mann_atmosphere_init(float turbidity);
bool tsfi_cpm_mann_atmosphere_compute_scattering(float sun_elevation, uint64_t *zmm_sky_latch_out);
bool tsfi_cpm_mann_atmosphere_apply_fog(uint64_t zmm_sky_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_MANN_RENDERMAN_ATMOSPHERE_H */
