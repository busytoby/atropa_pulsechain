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
    const char *sub_src_path = "tests/sub_source.xpl";
    const char *sub_bin_path = "tests/sub_99.dat.bin";

    // 1. Create a dummy sub .xpl source text file (sets value to 1234)
    FILE *sub_src = fopen(sub_src_path, "w");
    assert(sub_src != NULL);
    fprintf(sub_src, "WRITE_ABD 1234 1 1 1 0\n");
    fclose(sub_src);

    // 2. Compile sub .xpl
    bool ok = auncient_sdk_compile_xpl_to_dat_bin(sub_src_path, sub_bin_path);
    assert(ok == true);

    // 3. Create a parent .xpl source text file (loads sub 99, then reads)
    FILE *src = fopen(src_path, "w");
    assert(src != NULL);
    fprintf(src, "LOAD_SUB_XPL 99\n");
    fprintf(src, "READ_KERMIT\n");
    fclose(src);

    printf("[TEST] Compiling parent .xpl source to binary .dat.bin stream...\n");
    fflush(stdout);

    ok = auncient_sdk_compile_xpl_to_dat_bin(src_path, bin_path);
    assert(ok == true);
    printf("   ✓ Parent compiler executed successfully.\n");
    fflush(stdout);

    // 4. Setup Coaxial CICS execution environment
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

    printf("[TEST] Executing parent compiled .dat.bin binary stream via SDK...\n");
    fflush(stdout);

    uint32_t results[2] = { 0 };
    ok = auncient_sdk_execute_dat_bin(&ctx, bin_path, results, 2);
    assert(ok == true);
    assert(results[0] == 99);   // Executed Trace for sub-load instruction
    assert(results[1] == 1234); // Result of READ_KERMIT reflecting sub-write
    printf("   ✓ Executed parent compiled .dat.bin binary stream. Result 0 (loaded ID): %u, Result 1 (read value): %u.\n", results[0], results[1]);
    fflush(stdout);

    // Verify registers updated in coaxial environment
    assert(env.registers[0].value == 1234);
    assert(cache.cached_value == 1234);
    printf("   ✓ Verified register values and cache in CICS context match sub-routine write.\n");
    fflush(stdout);

    auncient_sdk_close_coaxial(&env);

    // Clean up temporary files
    remove(src_path);
    remove(bin_path);
    remove(sub_src_path);
    remove(sub_bin_path);

    printf("=============================================================\n");
    printf("XPL COMPILER & EXECUTION TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
