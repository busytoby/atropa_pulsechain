#ifndef TSFI_TREE_NET_VM_H
#define TSFI_TREE_NET_VM_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    // ARPANET (IMP / Host)
    uint16_t arpanet_imp;
    uint8_t arpanet_host;

    // ETHERNET (MAC Address)
    uint8_t eth_mac[6];

    // DECNET (Area / Node)
    uint8_t decnet_area;
    uint16_t decnet_node;

    // STANAG (Tactical Military ID)
    uint16_t stanag_id;
} tsfi_net_address_space_t;

typedef struct {
    uint32_t node_key;                  // 2-3 Tree Node Key
    tsfi_net_address_space_t addrs;     // Multi-Protocol Address Space
    uint32_t evm_gas_units;             // 180 Gas / Auncient Ether Units
    double fet_power_watts;             // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];             // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_tree_net_vm_node_t;

/* Initialize 2-3 Tree Network VM Node */
int tsfi_tree_net_vm_init_node(
    uint32_t node_key,
    uint16_t arpanet_imp, uint8_t arpanet_host,
    const uint8_t eth_mac[6],
    uint8_t decnet_area, uint16_t decnet_node,
    uint16_t stanag_id,
    tsfi_tree_net_vm_node_t *node
);

/* Route inter-node packet across 2-3 tree Network VM topology */
int tsfi_tree_net_vm_route_packet(
    const tsfi_tree_net_vm_node_t *src_node,
    const tsfi_tree_net_vm_node_t *dst_node,
    const char *protocol_type,
    const uint8_t *payload, size_t payload_len
);

#endif // TSFI_TREE_NET_VM_H
