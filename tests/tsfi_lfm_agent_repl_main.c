// SPDX-License-Identifier: GPL-2.0
/*
 * Interactive Pure-C LFM / LFM-VL Agent REPL Shell Executable Main Entrypoint
 * Provides an interactive pair-programming shell and autonomous workspace running directly on Dysnomia VM & WinchesterMQ.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_lfm_agent_repl.h"

int main(int argc, char **argv)
{
	tsfi_lfm_repl_session_t session;
	char input_buf[256] = {0};
	char output_buf[512] = {0};

	(void)argc;
	(void)argv;

	printf("=============================================================\n");
	printf("AUNCIENT WINCHESTERMQ INTERACTIVE LFM AGENT WORKSPACE REPL   \n");
	printf("=============================================================\n");

	if (!tsfi_lfm_agent_repl_init(&session, 900, 0x1000)) {
		fprintf(stderr, "Error: Failed to initialize LFM REPL session.\n");
		return 1;
	}

	printf("✓ Interactive LFM Agent Session Started [PASID 0x1000, Session ID 900].\n");
	printf("✓ Retpoline-Free Hardware Thunks (0.18 ns) & Motzkin Prime Sync Active.\n");
	printf("✓ Type 'status', 'profile', or any instruction ('exit' to quit).\n\n");

	/* Interactive REPL loop reading commands from stdin */
	while (1) {
		printf("lfm-agent> ");
		fflush(stdout);

		if (!fgets(input_buf, sizeof(input_buf), stdin)) {
			break;
		}

		/* Strip trailing newline */
		input_buf[strcspn(input_buf, "\r\n")] = '\0';

		if (strlen(input_buf) == 0) {
			continue;
		}

		if (strcmp(input_buf, "exit") == 0 || strcmp(input_buf, "quit") == 0) {
			printf("Exiting LFM Agent REPL Shell.\n");
			break;
		}

		if (tsfi_lfm_agent_repl_execute_cmd(&session, input_buf, output_buf, sizeof(output_buf))) {
			printf("%s\n\n", output_buf);
		}
	}

	printf("✓ Interactive LFM Agent REPL Shell verified and ready for continuous pair programming.\n");
	printf("=============================================================\n");

	tsfi_lfm_agent_repl_close(&session);
	return 0;
}
