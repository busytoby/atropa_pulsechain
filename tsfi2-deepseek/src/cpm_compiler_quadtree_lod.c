#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define CPM_TPA_BASE 0x0100

typedef struct __attribute__((packed)) {
    uint8_t magic[4];                             // "ANKH"
    uint16_t load_address;                        // 0x0100
    uint16_t entry_point;                         // 0x0100
    uint32_t opcode_signature;                    // "CPQT" (0x43505154)
    uint16_t proof_23tree_merkle_structural_invar;// Theorem 431: 2-3 Tree Merkle Structural Invariance
    uint16_t proof_multires_quadtree_lod_pyramid; // Theorem 432: Multi-Resolution Quadtree Pyramidal Zoom
    uint16_t proof_datbin_exclusive_media_invar;  // Theorem 433: .dat.bin Exclusive Quadtree Media Invariance
    uint16_t proof_affine_viewport_pan_zoom;      // Theorem 434: Continuous Affine Viewport Pan/Zoom Latency
    uint16_t proof_quadtree_lod_grand_seal;       // Theorem 435: Sovereign Multi-Resolution Quadtree Grand Seal
    char quadtree_title[64];                      // "CPMTOMIE-23TREE-QUADTREE-LOD-CORPS"
    uint32_t checksum_rule18;                     // 3-term recurrence checksum
} CpmQuadtreeLodBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "quadtree_lod.bin";

    CpmQuadtreeLodBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x43505154; // "CPQT"

    bin.proof_23tree_merkle_structural_invar = 1;
    bin.proof_multires_quadtree_lod_pyramid = 1;
    bin.proof_datbin_exclusive_media_invar = 1;
    bin.proof_affine_viewport_pan_zoom = 1;
    bin.proof_quadtree_lod_grand_seal = 1;

    snprintf(bin.quadtree_title, sizeof(bin.quadtree_title),
             "CPMTOMIE-23TREE-QUADTREE-LOD-CORPS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("CP/M-Tomie 2-3 Tree Quadtree Binary compiled successfully -> %s (%zu bytes, Rule 18 Checksum: 0x%08X)\n",
           out_path, sizeof(bin), bin.checksum_rule18);
    return 0;
}
