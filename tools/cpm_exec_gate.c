#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

// CP/M-Tomie Executor for 'gate.bin'
#define CPM_TPA_BASE 0x0100

typedef struct __attribute__((packed)) {
    uint8_t magic[4];          // "ANKH"
    uint16_t load_address;     // 0x0100
    uint16_t entry_point;      // 0x0100
    uint32_t opcode_signature; // "GATE"
    uint16_t gates_arbitrated; // Gates
    uint16_t scsi_handshakes;  // Handshakes
    uint16_t payload_len;      // Text length
    char payload[128];         // Status string
    uint32_t checksum_rule18;  // Checksum
} CpmGateBinary;

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
    const char *bin_path = (argc > 1) ? argv[1] : "gate.bin";

    FILE *fp = fopen(bin_path, "rb");
    if (!fp) {
        fprintf(stderr, "Error: Could not open binary file '%s'\n", bin_path);
        return 1;
    }

    CpmGateBinary header;
    size_t read_bytes = fread(&header, 1, sizeof(header), fp);
    fclose(fp);

    if (read_bytes != sizeof(header)) {
        fprintf(stderr, "Error: Incomplete binary read\n");
        return 1;
    }

    if (header.magic[0] != 'A' || header.magic[1] != 'N' || header.magic[2] != 'K' || header.magic[3] != 'H') {
        fprintf(stderr, "Error: Invalid ANKH magic header!\n");
        return 2;
    }

    if (header.load_address != CPM_TPA_BASE) {
        fprintf(stderr, "Error: Invalid load address!\n");
        return 3;
    }

    uint32_t expected_chk = compute_rule18_checksum((const uint8_t *)&header, sizeof(header) - sizeof(uint32_t));
    if (header.checksum_rule18 != expected_chk) {
        fprintf(stderr, "Error: Rule 18 checksum mismatch!\n");
        return 4;
    }

    if (header.payload_len > 0 && header.payload_len <= 128) {
        ssize_t written = write(STDOUT_FILENO, header.payload, header.payload_len);
        (void)written;
    }

    return 0;
}
