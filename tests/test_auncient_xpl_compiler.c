#include "auncient_sdk.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

// -------------------------------------------------------------
// .xpl to .dat.bin Compiler Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT .XPL SOURCE COMPILER SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    const char *src_path = "tests/test_source.xpl";
    const char *bin_path = "tests/test_compiled.dat.bin";

    // 1. Create a dummy .xpl source text file
    FILE *src = fopen(src_path, "w");
    assert(src != NULL);
    fprintf(src, "WRITE_ABD 999 1 1 1 0\n");
    fprintf(src, "READ_KERMIT\n");
    fclose(src);

    printf("[TEST] Compiling .xpl source to binary .dat.bin stream...\n");
    fflush(stdout);

    bool ok = auncient_sdk_compile_xpl_to_dat_bin(src_path, bin_path);
    assert(ok == true);
    printf("   ✓ Compiler executed successfully.\n");
    fflush(stdout);

    // 2. Read and verify output binary structure
    FILE *bin = fopen(bin_path, "rb");
    assert(bin != NULL);

    uint32_t signature = 0;
    size_t read_bytes = fread(&signature, sizeof(uint32_t), 1, bin);
    assert(read_bytes == 1);
    assert(signature == 0x58504C00); // "XPL\0"
    printf("   ✓ Verified compiled binary signature (0x58504C00).\n");
    fflush(stdout);

    uint32_t count = 0;
    read_bytes = fread(&count, sizeof(uint32_t), 1, bin);
    assert(read_bytes == 1);
    assert(count == 2);
    printf("   ✓ Verified instruction count (2).\n");
    fflush(stdout);

    // Verify first instruction: WRITE_ABD
    uint8_t op = 0;
    uint32_t val = 0;
    uint32_t mask = 0;
    
    read_bytes = fread(&op, sizeof(uint8_t), 1, bin);
    assert(read_bytes == 1);
    assert(op == ALU_OP_WRITE_ABD);

    read_bytes = fread(&val, sizeof(uint32_t), 1, bin);
    assert(read_bytes == 1);
    assert(val == 999);

    read_bytes = fread(&mask, sizeof(uint32_t), 1, bin);
    assert(read_bytes == 1);
    assert(mask == 0x7); // 1 + 2 + 4 = 7

    printf("   ✓ Verified instruction 0: Opcode WRITE_ABD, Value 999, Approvals mask 0x7.\n");
    fflush(stdout);

    fclose(bin);

    // Clean up temporary files
    remove(src_path);
    remove(bin_path);

    printf("=============================================================\n");
    printf("XPL COMPILER TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
