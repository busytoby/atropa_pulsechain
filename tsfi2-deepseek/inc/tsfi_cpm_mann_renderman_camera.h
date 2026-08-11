/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_MANN_RENDERMAN_CAMERA_H
#define TSFI_CPM_MANN_RENDERMAN_CAMERA_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_mann_camera_init(float fov_degrees);
bool tsfi_cpm_mann_camera_orbit(float yaw, float pitch, float distance, uint64_t *zmm_camera_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_MANN_RENDERMAN_CAMERA_H */
