/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_RETPOLINE_FAULT_STRESS_EXT_H
#define TSFI_RETPOLINE_FAULT_STRESS_EXT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_retpoline_fault_stress_ext_init(uint64_t security_mask);
bool tsfi_retpoline_fault_stress_ext_eval(uint64_t target_addr, uint64_t *stress_latch_out);
uint64_t tsfi_retpoline_fault_stress_ext_run(uint64_t total_iterations);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_RETPOLINE_FAULT_STRESS_EXT_H */
