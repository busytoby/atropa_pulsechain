#ifndef AUNCIENT_FPGA_BEYOND_595_THEOREMS_596_600_H
#define AUNCIENT_FPGA_BEYOND_595_THEOREMS_596_600_H

#include "auncient_fpga_beyond_590_theorems_591_595.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Hexagram-to-DAT Equivalence Engine */
typedef struct {
    BlackRedHexagram hex_coordinate;
    uint32_t         dat_base_offset;
    uint32_t         dat_check_offset;
    uint32_t         attached_tree_node_id;
    VmChainDomain    target_vm_domain;
    uint8_t          quadtree_dat_bin_slice[64]; /* Pure binary media .dat.bin layout (Rule 13) */
} HexagramDatMappingRecord;

typedef struct {
    float    in_silicon_hexagram_dat_equivalence_fidelity;
    float    in_silicon_quadtree_blockchain_universal_ratio;
    float    in_silicon_hex_dat_lookup_latency_ns;
    uint64_t verified_hex_dat_saat_clearances;
    bool     hexagram_dat_equivalence_verified;  /* Theorem 596 */
    bool     quadtree_blockchain_verified;       /* Theorem 597 */
    bool     hex_dat_lookup_latency_verified;    /* Theorem 598 */
    bool     hex_dat_lossless_saat_verified;     /* Theorem 599 */
    bool     grand_600_parity_closure_verified;  /* Theorem 600 */
    uint32_t rule18_parity_checksum;
} FpgaBeyond595State;

void auncient_fpga_beyond_595_init(FpgaBeyond595State *state);
bool auncient_fpga_beyond_595_verify_theorems_596_600(FpgaBeyond595State *state);
uint32_t auncient_fpga_beyond_595_compute_rule18(const FpgaBeyond595State *state);

#endif /* AUNCIENT_FPGA_BEYOND_595_THEOREMS_596_600_H */
