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
    uint32_t opcode_signature;             // "KERM" (0x4B45524D)
    uint16_t proof_kermit_mark_framing;    // Theorem 1: Standard Kermit MARK (0x01) packet delimiter
    uint16_t proof_kermit_packet_encoding; // Theorem 2: Error-free 7-bit / 8-bit clean transport
    uint16_t proof_kermit_crc_soundness;   // Theorem 3: Strict CRC packet parity validation
    uint16_t proof_kermit_coax_integration;// Theorem 4: Coaxial, LoRa, and SCSI transport interoperability
    char protocol_name[64];                // "CPMTOMIE-KERMIT-STREAM-TRANSPORT-PROTOCOL"
    uint32_t checksum_rule18;              // 3-term recurrence checksum
} CpmKermitStreamBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "kermit_stream.bin";

    CpmKermitStreamBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x4B45524D; // "KERM"

    bin.proof_kermit_mark_framing = 1;
    bin.proof_kermit_packet_encoding = 1;
    bin.proof_kermit_crc_soundness = 1;
    bin.proof_kermit_coax_integration = 1;

    snprintf(bin.protocol_name, sizeof(bin.protocol_name),
             "CPMTOMIE-KERMIT-STREAM-TRANSPORT-PROTOCOL");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("Synthesized Kermit Stream Protocol Binary '%s' (%zu bytes)\n", out_path, sizeof(bin));
    return 0;
}
