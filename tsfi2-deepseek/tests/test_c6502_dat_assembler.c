#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_dat.h"
#include "tsfi_trie.h"
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

// Parse assembly lines using DAT to generate 6502 machine code
static int assemble_line(tsfi_dat *assembler_dat, const char *line, uint8_t *output_bin) {
    char mnemonic[16];
    char arg[32] = {0};
    int matched = sscanf(line, "%s %s", mnemonic, arg);
    if (matched <= 0) return 0;

    // Search mnemonic in DAT
    const char *opcode_str = tsfi_dat_search(assembler_dat, mnemonic);
    if (!opcode_str) {
        printf("[ERROR] Unrecognized mnemonic: %s\n", mnemonic);
        return -1;
    }

    uint8_t base_op = (uint8_t)strtol(opcode_str, NULL, 16);

    if (strcmp(mnemonic, "LDA") == 0) {
        if (arg[0] == '#') { // LDA Immediate, e.g. LDA #$05
            output_bin[0] = 0xA9;
            output_bin[1] = (uint8_t)strtol(arg + 2, NULL, 16);
            return 2;
        } else if (arg[0] == '$') { // LDA Absolute/ZeroPage, e.g. LDA $0200
            uint16_t addr = (uint16_t)strtol(arg + 1, NULL, 16);
            if (addr <= 0xFF) {
                output_bin[0] = 0xA5; // Zero Page
                output_bin[1] = (uint8_t)addr;
                return 2;
            } else {
                output_bin[0] = 0xAD; // Absolute
                output_bin[1] = addr & 0xFF;
                output_bin[2] = (addr >> 8) & 0xFF;
                return 3;
            }
        }
    } else if (strcmp(mnemonic, "ADC") == 0) {
        if (arg[0] == '#') { // ADC Immediate, e.g. ADC #$03
            output_bin[0] = 0x69;
            output_bin[1] = (uint8_t)strtol(arg + 2, NULL, 16);
            return 2;
        }
    } else if (strcmp(mnemonic, "STA") == 0) {
        if (arg[0] == '$') { // STA Absolute, e.g. STA $0200
            uint16_t addr = (uint16_t)strtol(arg + 1, NULL, 16);
            output_bin[0] = 0x8D;
            output_bin[1] = addr & 0xFF;
            output_bin[2] = (addr >> 8) & 0xFF;
            return 3;
        }
    } else if (strcmp(mnemonic, "BRK") == 0) {
        output_bin[0] = 0x00;
        return 1;
    }

    // Default fallback
    output_bin[0] = base_op;
    return 1;
}

int main(void) {
    printf("=== TSFi c6502 Double-Array Trie (DAT) Assembler Verification ===\n");

    // 1. Initialize Trie and insert 6502 Opcode mnemonics
    tsfi_trie_node *trie = tsfi_trie_create_node('\0');
    tsfi_trie_insert(trie, "BRK", "00");
    tsfi_trie_insert(trie, "LDA", "A9");
    tsfi_trie_insert(trie, "ADC", "69");
    tsfi_trie_insert(trie, "STA", "8D");

    // 2. Compile to DAT
    printf("[DAT_ASM] Compiling 6502 Assembler mnemonics to DAT...\n");
    tsfi_dat *assembler_dat = tsfi_dat_compile(trie);
    assert(assembler_dat != NULL);

    // 3. Assemble program:
    // LDA #$05
    // ADC #$03
    // STA $0200
    // BRK
    const char *assembly_src[] = {
        "LDA #$05",
        "ADC #$03",
        "STA $0200",
        "BRK"
    };

    uint8_t bin_code[128];
    int bin_len = 0;

    printf("[DAT_ASM] Assembling source statements...\n");
    for (int i = 0; i < 4; i++) {
        int bytes = assemble_line(assembler_dat, assembly_src[i], bin_code + bin_len);
        assert(bytes > 0);
        bin_len += bytes;
    }

    printf("Generated Machine Code (%d bytes): ", bin_len);
    for (int i = 0; i < bin_len; i++) {
        printf("%02X ", bin_code[i]);
    }
    printf("\n");

    // Verify expected machine code sequence: A9 05 69 03 8D 00 02 00
    uint8_t expected_code[] = {0xA9, 0x05, 0x69, 0x03, 0x8D, 0x00, 0x02, 0x00};
    assert(bin_len == sizeof(expected_code));
    assert(memcmp(bin_code, expected_code, bin_len) == 0);
    printf("  [PASS] DAT Assembler generated matching byte sequence.\n");

    // 4. Save compiled program to file
    FILE *fp = fopen("c6502_assembled.bin", "wb");
    if (fp) {
        fwrite(bin_code, 1, bin_len, fp);
        fclose(fp);
        printf("[DAT_ASM] Program output saved to: c6502_assembled.bin\n");
    }

    // Clean up
    tsfi_trie_destroy(trie);
    tsfi_dat_destroy(assembler_dat);

    printf("=== ALL DAT ASSEMBLY TESTS PASSED ===\n");
    return 0;
}
