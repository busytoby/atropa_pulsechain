/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_COAXIAL_PIPELINE_INTEGRATION_H
#define TSFI_CPM_COAXIAL_PIPELINE_INTEGRATION_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_coaxial_pipeline_integration_init(uint32_t model_mask);
bool tsfi_cpm_coaxial_pipeline_integration_eval(uint64_t zmm_rig_latch, uint32_t model_mask, uint64_t *zmm_coaxial_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_COAXIAL_PIPELINE_INTEGRATION_H */
