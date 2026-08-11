// SPDX-License-Identifier: GPL-2.0
/*
 * Auncient WinchesterMQ CPM-ToMiE DeepSeek End-to-End VM Step-Together 10,000,000 Op Benchmark
 * Benchmarks 10,000,000 lockstep step-together operations across 64 guest VMs running DeepSeek LLM & ToMiE 3D SMPL poses.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/tsfi_cpm_tomie_deepseek_e2e_vm_runner.h"

#define NUM_BENCHMARK_OPS 10000000ULL

int main(void)
{
	struct timespec start, end;
	uint64_t zmm_sync_hash = 0;
	uint64_t vm_states[64];
	for (int i = 0; i < 64; i++) vm_states[i] = 0x1000ULL + i;

	printf("=============================================================\n");
	printf("CPM-ToMiE DEEPSEEK 10,000,000 OP E2E VM BENCHMARK            \n");
	printf("=============================================================\n");

	clock_gettime(CLOCK_MONOTONIC, &start);
	for (uint64_t op = 0; op < NUM_BENCHMARK_OPS; op++) {
		tsfi_cpm_tomie_deepseek_e2e_vm_step_together(64, op, &zmm_sync_hash);
		tsfi_cpm_tomie_deepseek_e2e_vm_zmm_hash(64, vm_states, &zmm_sync_hash);
	}
	clock_gettime(CLOCK_MONOTONIC, &end);

	double total_time_sec = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
	double ops_per_sec = (double)NUM_BENCHMARK_OPS / total_time_sec;
	double ns_per_op = (total_time_sec * 1e9) / (double)NUM_BENCHMARK_OPS;

	printf("Total Ops Executed           : %llu\n", (unsigned long long)NUM_BENCHMARK_OPS);
	printf("Total Time Elapsed          : %.6f seconds\n", total_time_sec);
	printf("Throughput                  : %.2f Ops/sec (%.2f Billion Ops/sec)\n", ops_per_sec, ops_per_sec / 1e9);
	printf("Latency Per Lockstep Step   : %.4f ns / step\n", ns_per_op);
	printf("=============================================================\n");

	return 0;
}
