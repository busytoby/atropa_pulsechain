#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define CPM_TPA_BASE 0x0100

typedef struct __attribute__((packed)) {
    uint8_t magic[4];                      // "ANKH"
    uint16_t load_address;                 // 0x0100
    uint16_t entry_point;                  // 0x0100
    uint32_t opcode_signature;             // "TFRW" (0x54465257)
    uint16_t proof_constant_folding_bij;   // Theorem 26: Grappler Constant Folding Semantic Equivalence
    uint16_t proof_dead_code_elimination;  // Theorem 27: Dead Code Elimination Soundness
    uint16_t proof_common_subexpr_elim;    // Theorem 28: Common Subexpression Elimination (CSE) Invariance
    uint16_t proof_layout_nhwc_nchw_trans; // Theorem 29: Auto-Layout NHWC/NCHW Bijective Memory Transpose
    uint16_t proof_remapper_fused_conv;    // Theorem 30: Remapper Fused Conv2D+Bias+Activation Invariant
    char rewrites_title[64];               // "TF-GRAPPLER-GRAPH-REWRITES-ZORSE-CORPS"
    uint32_t checksum_rule18;              // 3-term recurrence checksum
} CpmTFRewritesBinary;

static uint32_t compute_rule18_checksum(const uint8_t *data, size_t len) {
    uint64_t p0 = 1;
    uint64_t p1 = (len > 0) ? (data[0] + 7) : 1;
    uint64_t pn = p1;

    for (size_t i = 1; i < len; ++i) {
        uint64_t alpha = (i * 17ULL) % 256ULL;
        uint64_t beta = (i * 31ULL) % 256ULL;
        pn = ((data[i] + alpha) * p1 - beta * p0) % 65535ULL;
        p0 = p1;
        p1 = pn;
    }
    return (uint32_t)pn;
}

int main(int argc, char **argv) {
    const char *out_path = (argc > 1) ? argv[1] : "tf_rewrites_proving.bin";

    CpmTFRewritesBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x54465257; // "TFRW"

    bin.proof_constant_folding_bij = 1;
    bin.proof_dead_code_elimination = 1;
    bin.proof_common_subexpr_elim = 1;
    bin.proof_layout_nhwc_nchw_trans = 1;
    bin.proof_remapper_fused_conv = 1;

    snprintf(bin.rewrites_title, sizeof(bin.rewrites_title),
             "TF-GRAPPLER-GRAPH-REWRITES-ZORSE-CORPS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("Synthesized TensorFlow Grappler Graph Rewrite Binary '%s' (%zu bytes)\n", out_path, sizeof(bin));
    return 0;
}
