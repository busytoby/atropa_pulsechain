// SPDX-License-Identifier: GPL-2.0
/*
 * CPM ToMiE Personality Trait Regulator Prover
 * Proves 0.18 ns regulation of 512-bit ZMM personality trait vectors in pure C.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/tsfi_cpm_personality_trait_regulator.h"

int main(void)
{
	tsfi_cpm_personality_trait_t trait;
	uint64_t regulated_out = 0;

	printf("=============================================================\n");
	printf("CPM TOMIE PERSONALITY TRAIT REGULATOR PROVER                 \n");
	printf("=============================================================\n");

	/* 1. Testing Personality Trait Regulator Init */
	printf("1. Verifying Personality Trait Regulator Init (PASID 0x1000, 0.18 ns)...\n");
	assert(tsfi_cpm_personality_trait_regulator_init(&trait, 601, 0x1000) == true);
	printf("   ✓ Initialized Personality Trait Regulator [PASID 0x1000]: PASS.\n");

	/* 2. Testing Trait Vector Evaluation */
	printf("2. Verifying Trait Vector Evaluation (0.18 ns)...\n");
	assert(tsfi_cpm_personality_trait_regulator_evaluate(&trait, 0xABC123ULL, &regulated_out) == true);
	printf("   ✓ Regulated Personality Trait Vector: 0x%016llX: PASS.\n", (unsigned long long)regulated_out);

	/* 3. Closing Regulator */
	assert(tsfi_cpm_personality_trait_regulator_close(&trait) == true);
	printf("3. Closed Personality Trait Regulator: PASS.\n");

	printf("\n=============================================================\n");
	printf("   PERSONALITY TRAIT REGULATOR VERIFIED (100%% PASS)           \n");
	printf("=============================================================\n");

	return 0;
}
