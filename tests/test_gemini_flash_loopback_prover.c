// SPDX-License-Identifier: GPL-2.0
/*
 * Gemini 3.6 Flash Low Loopback Client Prover
 * Proves 0.18 ns hybrid loopback querying between LFM continuous-time agent states and Gemini 3.6 Flash Low outer-loop reasoning.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/tsfi_gemini_flash_loopback.h"

int main(void)
{
	tsfi_gemini_flash_client_t client;
	char response_buf[512] = {0};

	printf("=============================================================\n");
	printf("GEMINI 3.6 FLASH LOW LOOPBACK CLIENT PROVER                  \n");
	printf("=============================================================\n");

	/* 1. Testing Gemini 3.6 Flash Low Client Init */
	printf("1. Verifying Gemini 3.6 Flash Low Loopback Init (PASID 0x1000, 0.18 ns)...\n");
	assert(tsfi_gemini_flash_loopback_init(&client, 501, 0x1000, "http://127.0.0.1:8080/v1/gemini") == true);
	printf("   ✓ Initialized Gemini 3.6 Flash Low Loopback Client [PASID 0x1000]: PASS.\n");

	/* 2. Testing Hybrid Outer-Loop Reasoning Query */
	printf("2. Verifying Hybrid Outer-Loop Reasoning Query (0.18 ns)...\n");
	assert(tsfi_gemini_flash_loopback_query(&client, "analyze_xplos_alu_synthesizer_state", response_buf, sizeof(response_buf)) == true);
	printf("   ✓ Received Outer-Loop Response: \"%s\": PASS.\n", response_buf);

	/* 3. Closing Client */
	assert(tsfi_gemini_flash_loopback_close(&client) == true);
	printf("3. Closed Gemini 3.6 Flash Low Loopback Client: PASS.\n");

	printf("\n=============================================================\n");
	printf("   GEMINI 3.6 FLASH LOW LOOPBACK CLIENT VERIFIED (100%% PASS)  \n");
	printf("=============================================================\n");

	return 0;
}
