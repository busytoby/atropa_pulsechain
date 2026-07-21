#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_tree_net_vm.h"
#include "tsfi_lowpower_fet.h"

int tsfi_tree_net_vm_init_node(
    uint32_t node_key,
    uint16_t arpanet_imp, uint8_t arpanet_host,
    const uint8_t eth_mac[6],
    uint8_t decnet_area, uint16_t decnet_node,
    uint16_t stanag_id,
    tsfi_tree_net_vm_node_t *node
) {
    if (!node) return -1;
    memset(node, 0, sizeof(tsfi_tree_net_vm_node_t));

    node->node_key = node_key;
    node->addrs.arpanet_imp = arpanet_imp;
    node->addrs.arpanet_host = arpanet_host;
    if (eth_mac) memcpy(node->addrs.eth_mac, eth_mac, 6);
    node->addrs.decnet_area = decnet_area;
    node->addrs.decnet_node = decnet_node;
    node->addrs.stanag_id = stanag_id;

    node->evm_gas_units = 180; // 180 Gas / Auncient Ether Units per execution cycle

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    node->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(node->tape_dat_bin, sizeof(node->tape_dat_bin), "TREE_NET_VM_%08X.DAT.BIN", node_key);
    return 0;
}

int tsfi_tree_net_vm_route_packet(
    const tsfi_tree_net_vm_node_t *src_node,
    const tsfi_tree_net_vm_node_t *dst_node,
    const char *protocol_type,
    const uint8_t *payload, size_t payload_len
) {
    if (!src_node || !dst_node || !protocol_type || !payload) return -1;

    printf("[2-3 TREE NET VM] Route (%s): Node 0x%04X -> Node 0x%04X | "
           "ARPANET: IMP%u.H%u -> IMP%u.H%u | "
           "ETH: %02X:%02X:%02X:%02X:%02X:%02X -> %02X:%02X:%02X:%02X:%02X:%02X | "
           "DECNET: %u.%u -> %u.%u | STANAG: 0x%04X -> 0x%04X | Payload: %zu Bytes | Gas: %u | File: %s\n",
           protocol_type, src_node->node_key, dst_node->node_key,
           src_node->addrs.arpanet_imp, src_node->addrs.arpanet_host,
           dst_node->addrs.arpanet_imp, dst_node->addrs.arpanet_host,
           src_node->addrs.eth_mac[0], src_node->addrs.eth_mac[1], src_node->addrs.eth_mac[2],
           src_node->addrs.eth_mac[3], src_node->addrs.eth_mac[4], src_node->addrs.eth_mac[5],
           dst_node->addrs.eth_mac[0], dst_node->addrs.eth_mac[1], dst_node->addrs.eth_mac[2],
           dst_node->addrs.eth_mac[3], dst_node->addrs.eth_mac[4], dst_node->addrs.eth_mac[5],
           src_node->addrs.decnet_area, src_node->addrs.decnet_node,
           dst_node->addrs.decnet_area, dst_node->addrs.decnet_node,
           src_node->addrs.stanag_id, dst_node->addrs.stanag_id,
           payload_len, src_node->evm_gas_units, src_node->tape_dat_bin);

    return 0; // Routing successful
}
