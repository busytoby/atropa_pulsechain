/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_LFM_ACID_ROLLBACK_HISTORY_H
#define TSFI_CPM_LFM_ACID_ROLLBACK_HISTORY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint32_t transaction_id;
	uint32_t pasid;
	char action_type[64];
	char target_file[128];
	uint64_t zmm_commit_latch;
	bool is_committed;
} tsfi_lfm_acid_transaction_t;

bool tsfi_cpm_lfm_acid_rollback_history_begin(tsfi_lfm_acid_transaction_t *tx, uint32_t pasid, const char *action_type, const char *target_file);
bool tsfi_cpm_lfm_acid_rollback_history_commit(tsfi_lfm_acid_transaction_t *tx);
bool tsfi_cpm_lfm_acid_rollback_history_rollback(tsfi_lfm_acid_transaction_t *tx);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_LFM_ACID_ROLLBACK_HISTORY_H */
