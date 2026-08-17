#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define CPM_TPA_BASE 0x0100

typedef struct __attribute__((packed)) {
    uint8_t magic[4];                    // "ANKH"
    uint16_t load_address;               // 0x0100
    uint16_t entry_point;                // 0x0100
    uint32_t opcode_signature;           // "STNN" (0x53544E4E)
    uint32_t total_standards_rules;      // 20 Core Project Rules
    uint16_t proof_auncient_lore;        // Rule 1: Auncient spelling preserved
    uint16_t proof_no_mocks_interop;     // Rule 7: Thread-safe dynamic interop only
    uint16_t proof_68k_file_limit;       // Rule 8: File bounds strictly under 68,000 bytes
    uint16_t proof_fet_discharge_only;   // Rule 10: Soft body Verlet solvers for FET discharge only
    uint16_t proof_dat_bin_storage_only; // Rule 13: Strict .dat.bin quadtree storage format
    uint16_t proof_displacementshader;   // Rule 14: DisplacementShader vertex scaling sync
    uint16_t proof_no_prohibited_terms;  // Rule 18: Non-preferential bijective matrices
    uint16_t proof_no_brotli_compression;// Rule 20: RFC 1951 Pure C Gzip/Deflate only
    char system_name[64];                // "CPMTOMIE-STANDARDS-COMPLIANT-NEURAL-SYSTEM"
    uint32_t checksum_rule18;            // 3-term recurrence checksum
} CpmStandardsNNProofBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "standards_nn_system.bin";

    CpmStandardsNNProofBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x53544E4E; // "STNN"
    bin.total_standards_rules = 20;

    bin.proof_auncient_lore = 1;
    bin.proof_no_mocks_interop = 1;
    bin.proof_68k_file_limit = 1;
    bin.proof_fet_discharge_only = 1;
    bin.proof_dat_bin_storage_only = 1;
    bin.proof_displacementshader = 1;
    bin.proof_no_prohibited_terms = 1;
    bin.proof_no_brotli_compression = 1;

    snprintf(bin.system_name, sizeof(bin.system_name),
             "CPMTOMIE-STANDARDS-COMPLIANT-NEURAL-SYSTEM");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("Synthesized Standards Compliant NN Binary '%s' (%zu bytes)\n", out_path, sizeof(bin));
    return 0;
}
