/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_LFM_AGENT_REPL_H
#define TSFI_LFM_AGENT_REPL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint32_t session_id;
	uint32_t active_pasid;
	bool is_active;
	uint64_t command_count;
} tsfi_lfm_repl_session_t;

bool tsfi_lfm_agent_repl_init(tsfi_lfm_repl_session_t *session, uint32_t session_id, uint32_t pasid);
bool tsfi_lfm_agent_repl_execute_cmd(tsfi_lfm_repl_session_t *session, const char *cmd, char *out_buf, size_t out_len);
bool tsfi_lfm_agent_repl_close(tsfi_lfm_repl_session_t *session);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_LFM_AGENT_REPL_H */
