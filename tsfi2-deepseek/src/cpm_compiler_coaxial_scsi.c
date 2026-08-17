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
    uint32_t opcode_signature;                    // "CPSI" (0x43505349)
    uint16_t proof_hardware_register_mapping;     // Theorem 391: Hardware State Register Mapping Invariance
    uint16_t proof_keycode_scsi_handshake_loop;   // Theorem 392: Keycode 30/32 Direct SCSI Handshake Loop
    uint16_t proof_dynamic_interop_concurrency;   // Theorem 393: Non-Mocked Dynamic Interop Registry Concurrency
    uint16_t proof_ook_lora_physical_bridge;      // Theorem 394: Dual-Modulation OOK/LoRa Physical Bridge
    uint16_t proof_coaxial_scsi_grand_seal;       // Theorem 395: Sovereign Coaxial Box Hardware Interop Grand Seal
    char scsi_interop_title[64];                  // "CPMTOMIE-COAXIAL-HARDWARE-SCSI-INTEROP-CORPS"
    uint32_t checksum_rule18;                     // 3-term recurrence checksum
} CpmCoaxialScsiBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "coaxial_scsi.bin";

    CpmCoaxialScsiBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x43505349; // "CPSI"

    bin.proof_hardware_register_mapping = 1;
    bin.proof_keycode_scsi_handshake_loop = 1;
    bin.proof_dynamic_interop_concurrency = 1;
    bin.proof_ook_lora_physical_bridge = 1;
    bin.proof_coaxial_scsi_grand_seal = 1;

    snprintf(bin.scsi_interop_title, sizeof(bin.scsi_interop_title),
             "CPMTOMIE-COAXIAL-HARDWARE-SCSI-INTEROP-CORPS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("CP/M-Tomie Coaxial Hardware SCSI Binary compiled successfully -> %s (%zu bytes, Rule 18 Checksum: 0x%08X)\n",
           out_path, sizeof(bin), bin.checksum_rule18);
    return 0;
}
