/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_DYNAMIC_ZMM_CONTRACT_RESOLVER_H
#define TSFI_CPM_DYNAMIC_ZMM_CONTRACT_RESOLVER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_dynamic_zmm_contract_resolver_init(uint64_t contract_addr);
bool tsfi_cpm_dynamic_zmm_contract_resolver_eval(uint64_t zmm_rig_latch, uint64_t contract_addr, uint64_t *zmm_contract_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_DYNAMIC_ZMM_CONTRACT_RESOLVER_H */
