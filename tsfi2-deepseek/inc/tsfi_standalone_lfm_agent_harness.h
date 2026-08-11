/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_STANDALONE_LFM_AGENT_HARNESS_H
#define TSFI_STANDALONE_LFM_AGENT_HARNESS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint32_t agent_id;
	uint32_t pasid;
	uint64_t zmm_state_latch;
	bool policy_filter_active;
} tsfi_lfm_agent_harness_t;

bool tsfi_standalone_lfm_agent_harness_init(tsfi_lfm_agent_harness_t *harness, uint32_t agent_id, uint32_t pasid);
bool tsfi_standalone_lfm_agent_harness_step(tsfi_lfm_agent_harness_t *harness, const char *input_buffer, char *output_buffer, size_t max_len);
bool tsfi_standalone_lfm_agent_harness_close(tsfi_lfm_agent_harness_t *harness);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_STANDALONE_LFM_AGENT_HARNESS_H */
