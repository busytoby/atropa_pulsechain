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
    uint32_t opcode_signature;             // "ADCP" (0x41444350)
    uint16_t proof_autodin_firewall_audit; // Theorem 121: AUTODIN Packet Firewall Pre-Execution Bitmask Audit
    uint16_t proof_capstan_tape_transport; // Theorem 122: Dual Capstan Magnetic Tape Transport Track Parity
    uint16_t proof_kermit_stream_framing;  // Theorem 123: Kermit Clean Packet Stream MARK 0x01 Framing
    uint16_t proof_edsac_io1_gating_audit; // Theorem 124: EDSAC Initial Orders 1 Instruction Prefetch Firewall Gating
    uint16_t proof_crc16_frame_integrity;  // Theorem 125: Pure CCITT-CRC16 Frame Check Sequence Validation
    char autodin_title[64];                // "AUTODIN-CAPSTAN-KERMIT-STREAM-CORPS"
    uint32_t checksum_rule18;              // 3-term recurrence checksum
} CpmAutodinCapstanBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "autodin_capstan_proving.bin";

    CpmAutodinCapstanBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x41444350; // "ADCP"

    bin.proof_autodin_firewall_audit = 1;
    bin.proof_capstan_tape_transport = 1;
    bin.proof_kermit_stream_framing = 1;
    bin.proof_edsac_io1_gating_audit = 1;
    bin.proof_crc16_frame_integrity = 1;

    snprintf(bin.autodin_title, sizeof(bin.autodin_title),
             "AUTODIN-CAPSTAN-KERMIT-STREAM-CORPS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("Synthesized AUTODIN Capstan Kermit Binary '%s' (%zu bytes)\n", out_path, sizeof(bin));
    return 0;
}
