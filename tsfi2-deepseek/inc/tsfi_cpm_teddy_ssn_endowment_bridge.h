/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_TEDDY_BEAR_SSN_ENDOWMENT_BRIDGE_H
#define TSFI_CPM_TEDDY_BEAR_SSN_ENDOWMENT_BRIDGE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_teddy_bear_ssn_endowment_bridge_init(uint64_t teddy_bear_dna_seed);
bool tsfi_cpm_teddy_bear_ssn_endowment_bridge_eval(uint64_t zmm_rig_latch, uint64_t teddy_bear_dna_seed, uint64_t *zmm_teddy_bear_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_TEDDY_BEAR_SSN_ENDOWMENT_BRIDGE_H */
