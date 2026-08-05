#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "inc/tsfi2_compiler_bin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void test_compile_dat_bin(void) {
    printf("[Test] Running TSFi2 compiler .dat.bin packaging checks...\n");
    
    const char *out_file = "/tmp/test_program.dat.bin";
    uint8_t mock_bytecode[] = { 0x90, 0x90, 0x55, 0x48, 0x89, 0xE5, 0xC3 }; // NOP, NOP, PUSH RBP, MOV RBP RSP, RET
    
    bool ok = tsfi2_compile_to_dat_bin(out_file, 0x1000, 5, mock_bytecode, sizeof(mock_bytecode));
    assert(ok == true);
    
    FILE *f = fopen(out_file, "rb");
    assert(f != NULL);
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    assert(size > 512); // Must be padded to 512 bytes + payload + 8-byte checksum
    
    fclose(f);
    remove(out_file);
    
    printf("[Test] TSFi2 compiler .dat.bin packaging checks passed.\n");
}

static bool bin_strstr(const char *buf, size_t len, const char *sub) {
    size_t sub_len = strlen(sub);
    if (sub_len > len) return false;
    for (size_t i = 0; i <= len - sub_len; i++) {
        if (memcmp(buf + i, sub, sub_len) == 0) {
            return true;
        }
    }
    return false;
}

static void test_compile_dat_bin_ext(void) {
    printf("[Test] Running extended TSFi2 compiler packaging checks...\n");
    
    const char *out_file = "/tmp/test_program_ext.dat.bin";
    uint8_t mock_bytecode[] = { 0x90, 0x90, 0x55, 0x48, 0x89, 0xE5, 0xC3 };
    
    bool ok = tsfi2_compile_to_dat_bin_ext(
        out_file, 0x1000, 5,
        "StackSize\tRequiredPlugins",
        "4096\tphysics_soft_body",
        mock_bytecode, sizeof(mock_bytecode)
    );
    assert(ok == true);
    
    FILE *f = fopen(out_file, "rb");
    assert(f != NULL);
    
    char header_buf[32768];
    size_t read_bytes = fread(header_buf, 1, sizeof(header_buf) - 1, f);
    header_buf[read_bytes] = '\0';
    
    assert(bin_strstr(header_buf, read_bytes, "StackSize") == true);
    assert(bin_strstr(header_buf, read_bytes, "RequiredPlugins") == true);
    assert(bin_strstr(header_buf, read_bytes, "physics_soft_body") == true);
    
    fclose(f);
    remove(out_file);
    
    printf("[Test] Extended TSFi2 compiler packaging checks passed.\n");
}

int main(void) {
    printf("[Test] Running standalone TSFi2 compiler packaging tests...\n");
    test_compile_dat_bin();
    test_compile_dat_bin_ext();
    printf("[Test] All compiler packaging tests completed successfully.\n");
    return 0;
}
