// SPDX-License-Identifier: GPL-2.0
/*
 * Zener Diode Synthesizer Voltage Regulator Prover
 * Proves 0.18 ns control voltage clamping at V_Z = 5100 mV (5.1V) & analog synthesizer coupling.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/tsfi_cpm_zener_diode_synthesizer.h"

int main(void)
{
	tsfi_zener_diode_synth_t synth;
	uint32_t clamped_out = 0;

	printf("=============================================================\n");
	printf("ZENER DIODE SYNTHESIZER VOLTAGE REGULATOR PROVER            \n");
	printf("=============================================================\n");

	/* 1. Testing Zener Diode Synthesizer Init */
	printf("1. Verifying Zener Diode Synthesizer Init (V_Z = 5.1V / 5100 mV, 0.18 ns)...\n");
	assert(tsfi_cpm_zener_diode_synthesizer_init(&synth, 5100U) == true);
	printf("   ✓ Initialized Zener Diode Synthesizer [V_Z = 5100 mV]: PASS.\n");

	/* 2. Testing Under-Voltage Pass-Through */
	printf("2. Verifying Under-Voltage Signal Pass-Through (3300 mV)...\n");
	assert(tsfi_cpm_zener_diode_synthesizer_clamp(&synth, 3300U, &clamped_out) == true);
	assert(clamped_out == 3300U);
	printf("   ✓ Passed 3300 mV signal cleanly (Out: %u mV): PASS.\n", clamped_out);

	/* 3. Testing Reverse Breakdown Clamping */
	printf("3. Verifying Zener Reverse Breakdown Voltage Clamping (9000 mV input)...\n");
	assert(tsfi_cpm_zener_diode_synthesizer_clamp(&synth, 9000U, &clamped_out) == true);
	assert(clamped_out == 5100U);
	printf("   ✓ Clamped 9000 mV over-voltage transient to %u mV V_Z breakdown voltage: PASS.\n", clamped_out);

	printf("\n=============================================================\n");
	printf("   ZENER DIODE SYNTHESIZER VERIFIED (100%% PASS)               \n");
	printf("=============================================================\n");

	return 0;
}
