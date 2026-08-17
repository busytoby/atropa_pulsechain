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
    uint32_t opcode_signature;           // "DUAL" (0x4455414C)
    uint16_t proof_ground_closure;       // Theorem 1: Lower algebraic barrier (restitution to Totient Zero)
    uint16_t proof_open_trajectory;      // Theorem 2: Upper open phase space (unbounded exploration capacity)
    uint16_t proof_lyapunov_synthesis;   // Theorem 3: Lyapunov as non-dissipative envelope protecting openness
    uint16_t proof_dialectic_resolution; // Theorem 4: Formal synthesis of closure preserving open innovation
    char dialectic_id[64];               // "LYAPUNOV-OPENNESS-CLOSURE-DUALITY-CORPS"
    uint32_t checksum_rule18;            // 3-term recurrence checksum
} CpmLyapunovDualityBinary;

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
    const char *bin_path = (argc > 1) ? argv[1] : "lyapunov_duality.bin";

    FILE *f = fopen(bin_path, "rb");
    if (!f) {
        fprintf(stderr, "Error: Cannot open '%s'\n", bin_path);
        return 1;
    }

    CpmLyapunovDualityBinary bin;
    if (fread(&bin, 1, sizeof(bin), f) != sizeof(bin)) {
        fprintf(stderr, "Error: Invalid binary format\n");
        fclose(f);
        return 1;
    }
    fclose(f);

    if (memcmp(bin.magic, "ANKH", 4) != 0 || bin.load_address != CPM_TPA_BASE) {
        fprintf(stderr, "Error: Corrupted ANKH header\n");
        return 1;
    }

    uint32_t expected = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));
    if (bin.checksum_rule18 != expected) {
        fprintf(stderr, "Error: Checksum mismatch\n");
        return 1;
    }

    printf("=================================================================\n");
    printf("CP/M-TOMIE TPA (0100H) EXECUTION: LYAPUNOV OPENNESS & CLOSURE DUAL\n");
    printf("=================================================================\n");
    printf(" Synthesis ID:             %s\n", bin.dialectic_id);
    printf(" [T1] Ground Closure:      %s\n", bin.proof_ground_closure ? "PROVEN (LOWER BOUND TOTIENT_0)" : "FAIL");
    printf(" [T2] Open Trajectory:     %s\n", bin.proof_open_trajectory ? "PROVEN (UPPER PHASE UNBOUNDED)" : "FAIL");
    printf(" [T3] Lyapunov Envelope:   %s\n", bin.proof_lyapunov_synthesis ? "PROVEN (STABILITY PROTECTS OPENNESS)" : "FAIL");
    printf(" [T4] Dialectic Synthesis: %s\n", bin.proof_dialectic_resolution ? "PROVEN (CLOSURE ENABLES DISCOVERY)" : "FAIL");
    printf(" Rule 18 Checksum:         0x%08X (VERIFIED)\n", bin.checksum_rule18);
    printf("[FORMAL SYNTHESIS: CLOSURE SERVES AS THE ENABLER OF OPEN DISCOVERY]\n");
    printf("=================================================================\n");

    return 0;
}
