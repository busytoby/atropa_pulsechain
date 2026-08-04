#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "inc/tsfi2_compiler.h"
#include "inc/tsfi2_compiler_bin.h"
#include "inc/tsfi2_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static uint64_t calculate_fnv1a(const uint8_t *data, size_t len) {
    uint64_t hash = 14695981039346656037ULL;
    for (size_t i = 0; i < len; i++) {
        hash ^= data[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT DAT.BIN ACCUMULATOR UNIT TEST\n");
    printf("=============================================================\n");

    const char *source = "int main() { return 77; }";
    uint8_t bytecode[32];
    size_t bytecode_len = 0;
    
    bool ok = tsfi2_compile(source, bytecode, sizeof(bytecode), &bytecode_len);
    assert(ok == true);

    // Phase 1: Compile original dat.bin with STANAG network mount header
    const char *orig_file = "/tmp/accumulator_orig.dat.bin";
    ok = tsfi2_compile_to_dat_bin_ext(orig_file, 0x1000, 1, "NetworkMount", "STANAG", bytecode, bytecode_len);
    assert(ok == true);

    // Phase 2: Read the original dat.bin file, simulate state accumulation on header
    FILE *f = fopen(orig_file, "rb");
    assert(f != NULL);
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    uint8_t *buffer = malloc(size);
    assert(buffer != NULL);
    size_t read_bytes = fread(buffer, 1, size - 8, f); // Read everything except the trailing FNV-1a checksum
    assert(read_bytes == (size_t)(size - 8));
    fclose(f);

    // Find the header value "STANAG" and accumulate it to "DECNET"
    char *hdr_ptr = strstr((char *)buffer, "STANAG");
    assert(hdr_ptr != NULL);
    memcpy(hdr_ptr, "DECNET", 6); // Accumulate network target to DECNET

    // Re-align and calculate the new FNV-1a checksum for the accumulated file
    uint64_t new_checksum = calculate_fnv1a(buffer, size - 8);

    // Phase 3: Write the accumulated dat.bin back with the new checksum
    const char *accum_file = "/tmp/accumulator_final.dat.bin";
    FILE *out = fopen(accum_file, "wb");
    assert(out != NULL);
    fwrite(buffer, 1, size - 8, out);
    fwrite(&new_checksum, 1, sizeof(new_checksum), out);
    fclose(out);

    // Phase 4: Load and execute the accumulated dat.bin program
    Tsfi2CpuState cpu;
    ok = tsfi2_load_and_execute(accum_file, &cpu);
    assert(ok == true);
    assert(cpu.halted == true);
    assert(cpu.exit_code == 77);

    printf("   ✓ original dat.bin successfully compiled and read.\n");
    printf("   ✓ header successfully accumulated from STANAG to DECNET on disk.\n");
    printf("   ✓ FNV-1a checksum recalculated and verified successfully.\n");
    printf("   ✓ Accumulated dat.bin executed cleanly with exit code 77.\n");

    free(buffer);
    remove(orig_file);
    remove(accum_file);
    return 0;
}
