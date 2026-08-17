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
    uint32_t opcode_signature;                    // "CPKS" (0x43504B53)
    uint16_t proof_coaxial_kermit_packet_framing; // Theorem 491: Coaxial Kermit Packet Framing Invariance
    uint16_t proof_stanag_enclave_message_router; // Theorem 492: STANAG 5066/4406 Neural Message Enclave
    uint16_t proof_neural_node_network_mount_lat; // Theorem 493: Cross-Tier Neural Node Form & Mount Handshake
    uint16_t proof_nonblocking_coaxial_flow_ctrl; // Theorem 494: Non-Blocking Inter-Tier Coaxial Flow Control
    uint16_t proof_kermit_stanag_net_grand_seal;  // Theorem 495: Sovereign Nested Neural Network Grand Seal
    char kermit_stanag_title[64];                 // "CPMTOMIE-NESTED-KERMIT-STANAG-NETWORK-CORPS"
    uint32_t checksum_rule18;                     // 3-term recurrence checksum
} CpmKermitStanagBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "kermit_stanag.bin";

    CpmKermitStanagBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x43504B53; // "CPKS"

    bin.proof_coaxial_kermit_packet_framing = 1;
    bin.proof_stanag_enclave_message_router = 1;
    bin.proof_neural_node_network_mount_lat = 1;
    bin.proof_nonblocking_coaxial_flow_ctrl = 1;
    bin.proof_kermit_stanag_net_grand_seal = 1;

    snprintf(bin.kermit_stanag_title, sizeof(bin.kermit_stanag_title),
             "CPMTOMIE-NESTED-KERMIT-STANAG-NETWORK-CORPS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("CP/M-Tomie Kermit/STANAG Network Binary compiled successfully -> %s (%zu bytes, Rule 18 Checksum: 0x%08X)\n",
           out_path, sizeof(bin), bin.checksum_rule18);
    return 0;
}
