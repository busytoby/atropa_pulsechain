/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SOFT_BODY_FET_DISCHARGE_ISOLATOR_H
#define TSFI_CPM_SOFT_BODY_FET_DISCHARGE_ISOLATOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_soft_body_fet_discharge_isolator_init(uint32_t fet_id);
bool tsfi_cpm_soft_body_fet_discharge_isolator_eval(uint64_t zmm_rig_latch, uint32_t fet_id, uint64_t *zmm_fet_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SOFT_BODY_FET_DISCHARGE_ISOLATOR_H */
