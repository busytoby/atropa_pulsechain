/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_PERSONALITY_COAXIAL_AGENT_H
#define TSFI_CPM_PERSONALITY_COAXIAL_AGENT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_personality_coax_init(uint32_t agent_id, uint32_t channel_id);
bool tsfi_cpm_personality_coax_send_trait(uint32_t agent_id, uint64_t zmm_trait_latch, uint64_t *zmm_coax_latch_out);
bool tsfi_cpm_personality_coax_recv_trait(uint32_t agent_id, uint64_t *zmm_trait_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_PERSONALITY_COAXIAL_AGENT_H */
