/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_TOMIE_DEEPSEEK_ISOLATION_STRESS_H
#define TSFI_CPM_TOMIE_DEEPSEEK_ISOLATION_STRESS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_tomie_deepseek_verify_pasid_isolation(uint32_t pasid_src, uint32_t pasid_dst, bool *blocked_out);
bool tsfi_cpm_tomie_deepseek_stress_64vm_collisions(uint32_t active_vm_count, uint32_t *collision_count_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_TOMIE_DEEPSEEK_ISOLATION_STRESS_H */
