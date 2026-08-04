#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "inc/tsfi2_compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void test_standards_code_size(void) {
    printf("[Test] Running C standards compiler built-in code size checks...\n");
    
    // Program with built-in vs program without built-in
    const char *source_with = "int main() { __builtin_wmq_send(); return 42; }";
    const char *source_without = "int main() { return 42; }";
    
    uint8_t bytecode_with[32];
    uint8_t bytecode_without[32];
    size_t len_with = 0;
    size_t len_without = 0;
    
    bool ok = tsfi2_compile(source_with, bytecode_with, sizeof(bytecode_with), &len_with);
    assert(ok == true);
    
    ok = tsfi2_compile(source_without, bytecode_without, sizeof(bytecode_without), &len_without);
    assert(ok == true);
    
    // Check that built-in compiles directly to exactly 8 bytes (no external boilerplate bloat)
    assert(len_with == 8);
    assert(len_without == 6);
    assert(len_with - len_without == 2); // Built-in adds exactly 2 bytes for the opcode
    
    printf("[Test] Standards compiler built-in code size checks passed.\n");
}

int main(void) {
    printf("[Test] Running standalone TSFi2 standards compiler tests...\n");
    test_standards_code_size();
    printf("[Test] All standards compiler tests completed successfully.\n");
    return 0;
}
