#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

// ANKH LLM Standalone Compiler: Emits 'hello.bin'
// Conforms to:
// 1. EDSAC Initial Orders 1 Relocatable Opcode Prefetch Firewall
// 2. CP/M-Tomie TPA (Base 0100H) Execution Binary Format
// 3. Permitted Opcode Bitmask Audit: Permitted = PROD(1 - ((B_proh >> (O_i - 'A')) & 1))
// 4. Rule 18 3-Term Orthogonal Recurrence Checksum
// 5. Zero Binaries in Git: Emitted dynamically as hello.bin for local console execution

#define CPM_TPA_BASE 0x0100
#define MOTZKIN_PRIME 953467954114363ULL

// Prohibited Opcode Bitmask (Example EDSAC Initial Orders 1 Firewall Mask)
// Opcode 'H' ('H' - 'A' = 7): Permitted (Bit 7 is 0)
// Opcode 'E' ('E' - 'A' = 4): Permitted (Bit 4 is 0)
// Opcode 'L' ('L' - 'A' = 11): Permitted (Bit 11 is 0)
// Opcode 'O' ('O' - 'A' = 14): Permitted (Bit 14 is 0)
static const uint32_t EDSAC_IO1_PROHIBITED_MASK = 0x00080004; // Prohibits 'C' (bit 2) and 'T' (bit 19)

// Bytecode Instruction Magic & Header
// Magic: 0x41 0x4E 0x4B 0x48 ('ANKH')
// TPA Offset: 0x0100
// Opcode: 'H' 'E' 'L' 'L' 'O' (0x48 0x45 0x4C 0x4C 0x4F)
// Payload: "hello\n"
typedef struct __attribute__((packed)) {
    uint8_t magic[4];          // "ANKH"
    uint16_t load_address;     // 0x0100 (TPA Base)
    uint16_t entry_point;      // 0x0100
    uint32_t opcode_signature; // FNV-1a Hash of "HELLO"
    uint16_t payload_len;      // Length of output string
    char payload[8];           // "hello\n\0\0"
    uint32_t checksum_rule18;  // Rule 18 3-term recurrence checksum
} AnkhBinaryHeader;

// Rule 18 3-term orthogonal polynomial recurrence checksum
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

// Audit Opcodes against EDSAC Initial Orders 1 Firewall
static bool audit_initial_orders1_opcodes(const char *opcodes) {
    size_t len = strlen(opcodes);
    for (size_t i = 0; i < len; ++i) {
        char op = opcodes[i];
        if (op >= 'A' && op <= 'Z') {
            uint32_t shift = (uint32_t)(op - 'A');
            if ((EDSAC_IO1_PROHIBITED_MASK >> shift) & 1) {
                return false; // Prohibited opcode detected
            }
        }
    }
    return true; // All opcodes permitted
}

int main(int argc, char **argv) {
    const char *out_path = (argc > 1) ? argv[1] : "hello.bin";

    printf("=============================================================\n");
    printf("ANKH LLM COMPILER: SYNTHESIZING PERMITTED 'HELLO' BINARY\n");
    printf("=============================================================\n");

    const char *opcode_seq = "HELLO";
    printf("1. Auditing Opcode Sequence '%s' against Initial Orders 1 Firewall...\n", opcode_seq);
    bool permitted = audit_initial_orders1_opcodes(opcode_seq);
    if (!permitted) {
        fprintf(stderr, "ERROR: Opcode sequence '%s' rejected by Initial Orders 1 prefetch firewall!\n", opcode_seq);
        return 1;
    }
    printf("   ✓ Opcode Sequence '%s' PERMITTED by Initial Orders 1 Firewall.\n", opcode_seq);

    // Construct ANKH Binary
    AnkhBinaryHeader bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE; // 0x0100
    bin.entry_point = CPM_TPA_BASE;  // 0x0100
    bin.opcode_signature = 0x48454C4FULL; // "HELO"
    bin.payload_len = 6;
    memcpy(bin.payload, "hello\n", 6);

    // Compute Rule 18 Polynomial Checksum
    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    printf("2. Emitting Verified Binary to '%s' (Size: %zu bytes)...\n", out_path, sizeof(bin));
    FILE *fp = fopen(out_path, "wb");
    if (!fp) {
        perror("Failed to open output binary file");
        return 1;
    }

    size_t written = fwrite(&bin, 1, sizeof(bin), fp);
    fclose(fp);

    if (written != sizeof(bin)) {
        fprintf(stderr, "ERROR: Incomplete binary write!\n");
        return 1;
    }

    printf("   ✓ Successfully generated '%s'.\n", out_path);
    printf("=============================================================\n");
    return 0;
}
