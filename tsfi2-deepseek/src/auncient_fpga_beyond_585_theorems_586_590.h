#ifndef AUNCIENT_FPGA_BEYOND_585_THEOREMS_586_590_H
#define AUNCIENT_FPGA_BEYOND_585_THEOREMS_586_590_H

#include "auncient_fpga_beyond_580_theorems_581_585.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* SCSI Command Block for 2-3 Tree Hexagram & DAT Trigram Traversal */
typedef struct {
    uint8_t  opcode;        /* 0x1A: TREE_NODE_QUERY, 0x2A: DAT_TRIGRAM_ROUTE */
    uint8_t  target_hex_key;/* 6-bit Black/Red Hexagram Key (0..63) */
    uint16_t transfer_length;
    uint32_t payload_offset;
    uint32_t session_id;
} ScsiTreeCommandBlock;

typedef struct {
    float    in_silicon_scsi_loopback_handshake_fidelity;
    float    in_silicon_displacement_shader_sync_ratio;
    float    in_silicon_scsi_tree_dispatch_latency_ns;
    uint64_t verified_scsi_tree_saat_clearances;
    bool     scsi_loopback_verified;               /* Theorem 586 */
    bool     displacement_shader_sync_verified;    /* Theorem 587 */
    bool     scsi_tree_dispatch_latency_verified;  /* Theorem 588 */
    bool     scsi_tree_saat_verified;              /* Theorem 589 */
    bool     grand_590_parity_closure_verified;    /* Theorem 590 */
    uint32_t rule18_parity_checksum;
} FpgaBeyond585State;

void auncient_fpga_beyond_585_init(FpgaBeyond585State *state);
bool auncient_fpga_beyond_585_verify_theorems_586_590(FpgaBeyond585State *state);
uint32_t auncient_fpga_beyond_585_compute_rule18(const FpgaBeyond585State *state);

#endif /* AUNCIENT_FPGA_BEYOND_585_THEOREMS_586_590_H */
