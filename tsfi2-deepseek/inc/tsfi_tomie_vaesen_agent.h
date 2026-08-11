/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_TOMIE_VAESEN_AGENT_H
#define TSFI_TOMIE_VAESEN_AGENT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_tomie_vaesen_agent_init(uint32_t vaesen_seed);
bool tsfi_tomie_vaesen_agent_eval(uint64_t zmm_rig_latch, uint32_t vaesen_seed, uint64_t *zmm_vaesen_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_TOMIE_VAESEN_AGENT_H */
