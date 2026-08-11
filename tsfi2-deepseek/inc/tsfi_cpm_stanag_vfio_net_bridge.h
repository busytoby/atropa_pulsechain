/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_STANAG_VFIO_NET_BRIDGE_H
#define TSFI_CPM_STANAG_VFIO_NET_BRIDGE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_stanag_vfio_net_bridge_init(uint32_t packet_count);
bool tsfi_cpm_stanag_vfio_net_bridge_eval(uint64_t zmm_rig_latch, uint32_t packet_count, uint64_t *zmm_vfio_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_STANAG_VFIO_NET_BRIDGE_H */
