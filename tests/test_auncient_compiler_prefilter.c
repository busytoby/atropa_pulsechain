#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "inc/tsfi2_compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT COMPILER PRE-FILTERING (ANALYZER) UNIT TEST\n");
    printf("=============================================================\n");

    uint8_t bc[256];
    size_t bclen = 0;

    // Test 1: Compile clean program (should pass)
    const char *clean_src = "int main() { __builtin_wmq_reg_write(1, 5); return 0; }";
    bool ok = tsfi2_compile(clean_src, bc, sizeof(bc), &bclen);
    assert(ok == true);
    printf("   ✓ Clean program compiled successfully.\n");

    // Test 2: Compile program with prohibited opcode 0xD9 without audit trigger (should pass)
    const char *no_audit_src = "int main() { __builtin_wmq_auth_idx(3); return 0; }";
    ok = tsfi2_compile(no_audit_src, bc, sizeof(bc), &bclen);
    assert(ok == true);
    printf("   ✓ Program with 0xD9 (without audit trigger) compiled successfully.\n");

    // Test 3: Compile program with prohibited opcode 0xD9 AND audit trigger comment (should fail)
    const char *prohibited_src = "// audit_prohibit\nint main() { __builtin_wmq_auth_idx(3); return 0; }";
    ok = tsfi2_compile(prohibited_src, bc, sizeof(bc), &bclen);
    assert(ok == false);
    printf("   ✓ Program with 0xD9 (with audit trigger) rejected by ANALYZER successfully.\n");

    printf("=============================================================\n");
    printf("COMPILER PRE-FILTERING TEST PASSED\n");
    printf("=============================================================\n");
    return 0;
}
