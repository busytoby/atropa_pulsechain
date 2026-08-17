#ifndef ANKH_LLM_CPM_REGISTRY_H
#define ANKH_LLM_CPM_REGISTRY_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define CPM_TPA_BASE 0x0100
#define ANKH_CPM_MAX_TITLE 64

typedef struct __attribute__((packed)) {
    uint8_t magic[4];                             // "ANKH" (0x41, 0x4E, 0x4B, 0x48)
    uint16_t load_address;                        // 0x0100
    uint16_t entry_point;                         // 0x0100
    uint32_t opcode_signature;                    // 4-byte ASCII FourCC
    uint16_t proof_theorems[5];                   // 5 Theorem Flags for suite
    char binary_title[ANKH_CPM_MAX_TITLE];        // System Identity
    uint32_t checksum_rule18;                     // 3-term recurrence checksum
} AnkhCpmBinaryDescriptor;

typedef struct {
    const char *fourcc_str;
    uint32_t opcode_signature;
    const char *title;
    const char *theorem_descriptions[5];
} AnkhCpmEntryDef;

static inline uint32_t ankh_cpm_compute_rule18_checksum(const uint8_t *data, size_t len) {
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

#endif /* ANKH_LLM_CPM_REGISTRY_H */
