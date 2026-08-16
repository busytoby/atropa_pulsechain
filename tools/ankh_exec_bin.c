#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

// ANKH Dysnomia VM Binary Executor for 'hello.bin'
// Reads and executes binaries produced by the ANKH LLM Compiler
// Enforces:
// 1. ANKH Magic Header Verification
// 2. CP/M TPA Base Address Assertion (0x0100)
// 3. Rule 18 3-Term Orthogonal Recurrence Checksum Verification
// 4. Zero Memory Corruption Execution

#define CPM_TPA_BASE 0x0100

typedef struct __attribute__((packed)) {
    uint8_t magic[4];          // "ANKH"
    uint16_t load_address;     // 0x0100
    uint16_t entry_point;      // 0x0100
    uint32_t opcode_signature; // FNV-1a Hash
    uint16_t payload_len;      // Payload string length
    char payload[8];           // "hello\n"
    uint32_t checksum_rule18;  // Checksum
} AnkhBinaryHeader;

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
    const char *bin_path = (argc > 1) ? argv[1] : "hello.bin";

    FILE *fp = fopen(bin_path, "rb");
    if (!fp) {
        fprintf(stderr, "Error: Could not open binary file '%s'\n", bin_path);
        return 1;
    }

    AnkhBinaryHeader header;
    size_t read_bytes = fread(&header, 1, sizeof(header), fp);
    fclose(fp);

    if (read_bytes != sizeof(header)) {
        fprintf(stderr, "Error: Incomplete binary read (expected %zu, got %zu)\n", sizeof(header), read_bytes);
        return 1;
    }

    // 1. Verify Magic
    if (header.magic[0] != 'A' || header.magic[1] != 'N' || header.magic[2] != 'K' || header.magic[3] != 'H') {
        fprintf(stderr, "Error: Invalid ANKH binary magic header!\n");
        return 2;
    }

    // 2. Verify TPA Load Address
    if (header.load_address != CPM_TPA_BASE) {
        fprintf(stderr, "Error: Invalid load address 0x%04X (expected 0x0100)!\n", header.load_address);
        return 3;
    }

    // 3. Verify Rule 18 Polynomial Checksum
    uint32_t expected_chk = compute_rule18_checksum((const uint8_t *)&header, sizeof(header) - sizeof(uint32_t));
    if (header.checksum_rule18 != expected_chk) {
        fprintf(stderr, "Error: Rule 18 polynomial checksum mismatch!\n");
        return 4;
    }

    // 4. Execute Opcode: Echo payload to stdout
    if (header.payload_len > 0 && header.payload_len <= 8) {
        ssize_t written = write(STDOUT_FILENO, header.payload, header.payload_len);
        (void)written;
    }

    return 0;
}
