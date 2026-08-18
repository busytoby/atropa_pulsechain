#ifndef AUNCIENT_FPGA_BEYOND_580_THEOREMS_581_585_H
#define AUNCIENT_FPGA_BEYOND_580_THEOREMS_581_585_H

#include "auncient_fpga_beyond_575_theorems_576_580.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Generic payload attached to a 2-3 Tree Node via Black/Red Hexagram Key */
typedef struct {
    uint8_t  payload_bytes[64]; /* Arbitrary sanitized payload storage */
    size_t   payload_size;
    uint32_t payload_crc;
} TreePayload;

/* 2-3 Tree Node augmented with Black/Red Hexagram Key & DAT Trigram Routing */
typedef struct TwoThreeTreeNode {
    BlackRedHexagram      hex_key_low;
    BlackRedHexagram      hex_key_high;
    TreePayload           payload_low;
    TreePayload           payload_high;
    bool                  is_3_node;
    bool                  is_leaf;
    struct TwoThreeTreeNode *child[3];
} TwoThreeTreeNode;

typedef struct {
    float    in_silicon_node_attachment_fidelity;
    float    in_silicon_arbitrary_payload_storage_ratio;
    float    in_silicon_tree_hex_dispatch_latency_ns;
    uint64_t verified_tree_payload_saat_clearances;
    bool     node_attachment_verified;           /* Theorem 581 */
    bool     payload_storage_verified;           /* Theorem 582 */
    bool     tree_hex_dispatch_latency_verified; /* Theorem 583 */
    bool     tree_payload_saat_verified;         /* Theorem 584 */
    bool     grand_585_parity_closure_verified;  /* Theorem 585 */
    uint32_t rule18_parity_checksum;
} FpgaBeyond580State;

void auncient_fpga_beyond_580_init(FpgaBeyond580State *state);
bool auncient_fpga_beyond_580_verify_theorems_581_585(FpgaBeyond580State *state);
uint32_t auncient_fpga_beyond_580_compute_rule18(const FpgaBeyond580State *state);

#endif /* AUNCIENT_FPGA_BEYOND_580_THEOREMS_581_585_H */
