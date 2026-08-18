#ifndef AUNCIENT_FPGA_BEYOND_575_THEOREMS_576_580_H
#define AUNCIENT_FPGA_BEYOND_575_THEOREMS_576_580_H

#include "auncient_fpga_beyond_570_theorems_571_575.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum {
    LINE_BLACK = 0,
    LINE_RED   = 1
} BlackRedLine;

typedef union {
    uint8_t raw; /* 0..63 */
    struct {
        uint8_t line1 : 1; /* Bit 0 */
        uint8_t line2 : 1; /* Bit 1 */
        uint8_t line3 : 1; /* Bit 2 */
        uint8_t line4 : 1; /* Bit 3 */
        uint8_t line5 : 1; /* Bit 4 */
        uint8_t line6 : 1; /* Bit 5 */
        uint8_t unused: 2;
    } bits;
} BlackRedHexagram;

/* Double-Array Trie (DAT) Structure for Trigram Traversal */
#define DAT_TRIGRAM_NODES 16

typedef struct {
    int32_t base[DAT_TRIGRAM_NODES];
    int32_t check[DAT_TRIGRAM_NODES];
} TrigramDoubleArrayTrie;

typedef struct {
    float    in_silicon_dat_trigram_traversal_fidelity;
    float    in_silicon_64_black_red_hexagram_completeness_ratio;
    float    in_silicon_dat_lookup_latency_ns;
    uint64_t verified_black_red_saat_clearances;
    bool     dat_trigram_traversal_verified;       /* Theorem 576 */
    bool     black_red_hexagram_verified;          /* Theorem 577 */
    bool     dat_lookup_latency_verified;          /* Theorem 578 */
    bool     black_red_lossless_saat_verified;     /* Theorem 579 */
    bool     grand_580_parity_closure_verified;    /* Theorem 580 */
    uint32_t rule18_parity_checksum;
} FpgaBeyond575State;

void auncient_fpga_beyond_575_init(FpgaBeyond575State *state);
bool auncient_fpga_beyond_575_verify_theorems_576_580(FpgaBeyond575State *state);
uint32_t auncient_fpga_beyond_575_compute_rule18(const FpgaBeyond575State *state);

#endif /* AUNCIENT_FPGA_BEYOND_575_THEOREMS_576_580_H */
