/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_PERSONALITY_TRAIT_REGULATOR_H
#define TSFI_CPM_PERSONALITY_TRAIT_REGULATOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint32_t session_id;
	uint32_t active_pasid;
	uint64_t trait_vector;
	bool is_regulated;
} tsfi_cpm_personality_trait_t;

bool tsfi_cpm_personality_trait_regulator_init(tsfi_cpm_personality_trait_t *trait, uint32_t session_id, uint32_t pasid);
bool tsfi_cpm_personality_trait_regulator_evaluate(tsfi_cpm_personality_trait_t *trait, uint64_t input_signal, uint64_t *regulated_vector_out);
bool tsfi_cpm_personality_trait_regulator_close(tsfi_cpm_personality_trait_t *trait);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_PERSONALITY_TRAIT_REGULATOR_H */
