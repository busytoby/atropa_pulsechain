/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_ACCUMULATOR_SPACE_CHARGE_ISOLATOR_H
#define TSFI_CPM_ACCUMULATOR_SPACE_CHARGE_ISOLATOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_accumulator_space_charge_isolator_init(uint32_t charge_law_id);
bool tsfi_cpm_accumulator_space_charge_isolator_eval(uint64_t zmm_rig_latch, uint32_t charge_law_id, uint64_t *zmm_acc_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_ACCUMULATOR_SPACE_CHARGE_ISOLATOR_H */
