/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_TOMIE_DEEPSEEK_H
#define TSFI_CPM_TOMIE_DEEPSEEK_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_tomie_deepseek_init(uint32_t model_pasid);
bool tsfi_cpm_tomie_deepseek_infer(uint32_t token_id, uint64_t *zmm_embedding_512bit_out);
bool tsfi_cpm_tomie_deepseek_render_smpl(uint64_t zmm_embedding_512bit, uint64_t smpl_vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_TOMIE_DEEPSEEK_H */
