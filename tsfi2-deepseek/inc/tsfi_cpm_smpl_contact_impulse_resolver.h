/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_CONTACT_IMPULSE_RESOLVER_H
#define TSFI_CPM_SMPL_CONTACT_IMPULSE_RESOLVER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_contact_impulse_resolver_init(uint32_t impulse_pair_count);
bool tsfi_cpm_smpl_contact_impulse_resolver_eval(uint64_t zmm_rig_latch, uint32_t impulse_pair_count, uint64_t *zmm_impulse_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_CONTACT_IMPULSE_RESOLVER_H */
