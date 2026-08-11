/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_TOMIE_DEEPSEEK_TELEMETRY_H
#define TSFI_CPM_TOMIE_DEEPSEEK_TELEMETRY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_tomie_deepseek_telemetry_log(uint32_t event_id, uint64_t latency_ns, uint64_t *zmm_telemetry_entry_out);
bool tsfi_cpm_tomie_deepseek_telemetry_read(uint32_t event_id, uint64_t *latency_ns_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_TOMIE_DEEPSEEK_TELEMETRY_H */
