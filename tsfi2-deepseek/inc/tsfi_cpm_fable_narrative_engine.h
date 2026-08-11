/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_FABLE_NARRATIVE_ENGINE_H
#define TSFI_CPM_FABLE_NARRATIVE_ENGINE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint32_t session_id;
	uint32_t pasid;
	uint64_t identity_reg;
	uint64_t signal_reg;
	uint64_t foundation_reg;
	uint64_t dynamo_reg;
	uint64_t personality_latch;
	bool fable_pattern_active;
} tsfi_fable_narrative_ctx_t;

bool tsfi_cpm_fable_narrative_engine_init(tsfi_fable_narrative_ctx_t *ctx, uint32_t session_id, uint32_t pasid);
bool tsfi_cpm_fable_narrative_engine_harmonize(tsfi_fable_narrative_ctx_t *ctx, const char *raw_prompt, char *narrative_out, size_t max_len);
bool tsfi_cpm_fable_narrative_engine_close(tsfi_fable_narrative_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_FABLE_NARRATIVE_ENGINE_H */
