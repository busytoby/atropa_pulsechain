/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_AVX512_ZMM_VECTOR_REGISTRATION_H
#define TSFI_CPM_AVX512_ZMM_VECTOR_REGISTRATION_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_avx512_zmm_vector_registration_init(uint32_t zmm_reg_idx);
bool tsfi_cpm_avx512_zmm_vector_registration_eval(uint64_t zmm_rig_latch, uint32_t zmm_reg_idx, uint64_t *zmm_vector_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_AVX512_ZMM_VECTOR_REGISTRATION_H */
