/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_RETPOLINE_FAULT_INTERCEPTOR_H
#define TSFI_RETPOLINE_FAULT_INTERCEPTOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_retpoline_fault_interceptor_init(uint64_t security_mask);
bool tsfi_retpoline_fault_interceptor_eval(uint64_t target_addr, uint64_t *fault_mask_out);
uint64_t tsfi_retpoline_fault_interceptor_stress_benchmark(uint64_t iteration_count);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_RETPOLINE_FAULT_INTERCEPTOR_H */
