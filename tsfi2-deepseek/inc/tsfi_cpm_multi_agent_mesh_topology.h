/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_MULTI_AGENT_MESH_TOPOLOGY_H
#define TSFI_CPM_MULTI_AGENT_MESH_TOPOLOGY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_multi_agent_mesh_topology_init(uint32_t node_count);
bool tsfi_cpm_multi_agent_mesh_topology_eval(uint64_t zmm_rig_latch, uint32_t node_count, uint64_t *zmm_mesh_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_MULTI_AGENT_MESH_TOPOLOGY_H */
