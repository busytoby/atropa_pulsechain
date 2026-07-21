#ifndef TSFI_DECNET_EXEC_STACK_H
#define TSFI_DECNET_EXEC_STACK_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint8_t src_area;           // DECnet Source Area (1..63)
    uint16_t src_node;          // DECnet Source Node (1..1023)
    uint8_t dst_area;           // DECnet Dest Area (1..63)
    uint16_t dst_node;          // DECnet Dest Node (1..1023)
    uint8_t nsp_flags;          // NSP Control Flags (0x01 = DATA, 0x02 = ACK)
    uint16_t seq_num;           // Packet Sequence Number
} tsfi_decnet_hdr_t;

typedef struct {
    uint32_t decnet_proc_id;    // DECnet Stack Processor ID
    tsfi_decnet_hdr_t last_hdr; // Last Routed Header
    uint8_t payload[256];       // Packet Payload
    size_t payload_len;         // Payload Length
    uint32_t evm_gas_units;     // 200 Gas / Auncient Ether Units
    double fet_power_watts;     // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];     // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_decnet_exec_stack_t;

/* Initialize DECnet Direct Executable Stack Processor */
int tsfi_decnet_exec_stack_init(
    uint32_t decnet_proc_id,
    tsfi_decnet_exec_stack_t *proc
);

/* Push and execute DECnet packet frame directly inside executable stack */
int tsfi_decnet_exec_stack_push_frame(
    tsfi_decnet_exec_stack_t *proc,
    uint8_t src_area, uint16_t src_node,
    uint8_t dst_area, uint16_t dst_node,
    const uint8_t *payload, size_t payload_len
);

#endif // TSFI_DECNET_EXEC_STACK_H
