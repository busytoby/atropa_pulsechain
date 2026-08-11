/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_TELEMETRY_RING_CRC32_PERSISTENCE_H
#define TSFI_CPM_TELEMETRY_RING_CRC32_PERSISTENCE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_telemetry_ring_crc32_persistence_init(uint32_t ring_id);
bool tsfi_cpm_telemetry_ring_crc32_persistence_eval(uint64_t zmm_rig_latch, uint32_t event_index, uint64_t *zmm_telemetry_latch_out);
bool tsfi_cpm_telemetry_ring_crc32_persistence_render_renderman(uint64_t zmm_telemetry_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_TELEMETRY_RING_CRC32_PERSISTENCE_H */
