#include "auncient_sdk.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

// -------------------------------------------------------------
// .xpl to .dat.bin Compiler & Execution Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT .XPL SOURCE COMPILER & EXECUTION SUITE\n");
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

    // 2. Setup Coaxial CICS execution environment
    sdk_coaxial_env_t env;
    ok = auncient_sdk_init_coaxial(&env);
    assert(ok == true);

    sdk_kermit_cache_t cache = { .cached_value = 0, .cached_ts = { 0, 0 }, .is_warm = false };
    sdk_cics_context_t ctx = {
        .env = &env,
        .cache = &cache,
        .quorum_type = SDK_QUORUM_MAJORITY,
        .writer_id = 77,
        .security_clearance = 3
    };

    printf("[TEST] Executing compiled .dat.bin binary stream via SDK...\n");
    fflush(stdout);

    uint32_t results[2] = { 0 };
    ok = auncient_sdk_execute_dat_bin(&ctx, bin_path, results, 2);
    assert(ok == true);
    assert(results[0] == 999);
    assert(results[1] == 999);
    printf("   ✓ Executed compiled .dat.bin binary stream. Result 0: %u, Result 1: %u.\n", results[0], results[1]);
    fflush(stdout);

    // Verify registers updated in coaxial environment
    assert(env.registers[0].value == 999);
    assert(cache.cached_value == 999);
    printf("   ✓ Verified register values and cache in CICS context are correct.\n");
    fflush(stdout);

    auncient_sdk_close_coaxial(&env);

    // Clean up temporary files
    remove(src_path);
    remove(bin_path);

    printf("=============================================================\n");
    printf("XPL COMPILER & EXECUTION TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
