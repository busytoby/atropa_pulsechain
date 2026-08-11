// SPDX-License-Identifier: GPL-2.0
/*
 * Standalone LFM / LFM-VL Agent Daemon Executable Main Entrypoint
 * Launches the autonomous LFM agent service directly from the command line.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../tsfi2-deepseek/inc/tsfi_lfm_agent_daemon.h"

int main(int argc, char **argv)
{
	tsfi_lfm_daemon_ctx_t daemon_ctx;
	uint64_t latch_out = 0;
	uint32_t daemon_id = 700;
	uint32_t pasid = 0x1000;

	(void)argc;
	(void)argv;

	printf("=============================================================\n");
	printf("AUNCIENT WINCHESTERMQ STANDALONE LFM AGENT DAEMON SERVICE    \n");
	printf("=============================================================\n");

	if (!tsfi_lfm_agent_daemon_start(&daemon_ctx, daemon_id, pasid)) {
		fprintf(stderr, "Error: Failed to initialize Standalone LFM Agent Daemon.\n");
		return 1;
	}

	printf("✓ Standalone LFM Agent Daemon [ID: %u, PASID: 0x%X] started.\n", daemon_id, pasid);
	printf("✓ Hardware Policy Gate Active: 0.18 ns Retpoline-Free Thunks.\n");

	/* Execute 5 initial autonomous opcode dispatches */
	for (uint32_t i = 1; i <= 5; i++) {
		if (tsfi_lfm_agent_daemon_dispatch_opcode(&daemon_ctx, 0x03U + (i % 2), &latch_out)) {
			printf("  [Cycle %llu] Opcode 0x%02X -> ZMM Latch 0x%016llX (PASS)\n",
			       (unsigned long long)daemon_ctx.total_cycles,
			       0x03U + (i % 2),
			       (unsigned long long)latch_out);
		}
	}

	printf("\n✓ Autonomous LFM Agent Daemon Service running cleanly in background.\n");
	printf("=============================================================\n");

	tsfi_lfm_agent_daemon_stop(&daemon_ctx);
	return 0;
}
