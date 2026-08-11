/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_LFM_AGENT_DAEMON_H
#define TSFI_LFM_AGENT_DAEMON_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint32_t daemon_id;
	uint32_t active_pasid;
	bool is_running;
	uint64_t total_cycles;
} tsfi_lfm_daemon_ctx_t;

bool tsfi_lfm_agent_daemon_start(tsfi_lfm_daemon_ctx_t *ctx, uint32_t daemon_id, uint32_t pasid);
bool tsfi_lfm_agent_daemon_dispatch_opcode(tsfi_lfm_daemon_ctx_t *ctx, uint32_t opcode, uint64_t *latch_out);
bool tsfi_lfm_agent_daemon_stop(tsfi_lfm_daemon_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_LFM_AGENT_DAEMON_H */
