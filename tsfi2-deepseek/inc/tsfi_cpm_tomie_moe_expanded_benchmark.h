/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_TOMIE_MOE_EXPANDED_BENCHMARK_H
#define TSFI_CPM_TOMIE_MOE_EXPANDED_BENCHMARK_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_tomie_moe_expanded_benchmark_init(uint64_t target_ops);
bool tsfi_cpm_tomie_moe_expanded_benchmark_run(uint64_t num_ops, double *ops_per_sec_out, double *ns_per_op_out, uint64_t *zmm_bench_latch_out);
bool tsfi_cpm_tomie_moe_expanded_benchmark_render_renderman(uint64_t zmm_bench_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_TOMIE_MOE_EXPANDED_BENCHMARK_H */
