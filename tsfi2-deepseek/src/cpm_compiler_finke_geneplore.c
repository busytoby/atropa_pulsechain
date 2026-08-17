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
    uint32_t opcode_signature;                    // "CPFG" (0x43504647)
    uint16_t proof_generative_preinventive_invar; // Theorem 471: Generative Preinventive Assembly Invariance
    uint16_t proof_exploratory_semantic_emergence;// Theorem 472: Exploratory Semantic Emergence
    uint16_t proof_nonteleological_serendipity;   // Theorem 473: Non-Teleological Creative Serendipity
    uint16_t proof_cyclic_geneplore_coupling;     // Theorem 474: Iterative Cyclic Geneplore Coupling
    uint16_t proof_geneplore_preinventive_seal;   // Theorem 475: Sovereign Geneplore Preinventive Grand Seal
    char geneplore_title[64];                     // "CPMTOMIE-FINKE-GENEPLORE-PREINVENTIVE-CORPS"
    uint32_t checksum_rule18;                     // 3-term recurrence checksum
} CpmFinkeGeneploreBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "finke_geneplore.bin";

    CpmFinkeGeneploreBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x43504647; // "CPFG"

    bin.proof_generative_preinventive_invar = 1;
    bin.proof_exploratory_semantic_emergence = 1;
    bin.proof_nonteleological_serendipity = 1;
    bin.proof_cyclic_geneplore_coupling = 1;
    bin.proof_geneplore_preinventive_seal = 1;

    snprintf(bin.geneplore_title, sizeof(bin.geneplore_title),
             "CPMTOMIE-FINKE-GENEPLORE-PREINVENTIVE-CORPS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("CP/M-Tomie Finke Geneplore Binary compiled successfully -> %s (%zu bytes, Rule 18 Checksum: 0x%08X)\n",
           out_path, sizeof(bin), bin.checksum_rule18);
    return 0;
}
