#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "inc/tsfi2_compiler.h"
#include "inc/tsfi2_compiler_bin.h"
#include "inc/tsfi2_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <unistd.h>
#include "tsfi2-deepseek/inc/tsfi_displacementshader.h"
#include "tsfi2-deepseek/inc/tsfi_quadtree_ksds.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int tsfi_mf_es_evm_spool_guard(const char *jcl_content, int *is_valid);

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT TSV MOUNTED WINCHESTERMQ INTEGRATION VALIDATION RUNNER\n");
    printf("=============================================================\n");

    // Read the C source code of the integration test from disk
    FILE *sf = fopen("tests/test_auncient_tsv_wmq_integration.c", "r");
    assert(sf != NULL);

    char source_code[4096];
    size_t read_bytes = fread(source_code, 1, sizeof(source_code) - 1, sf);
    source_code[read_bytes] = '\0';
    fclose(sf);

    // Verify source payload against the communication word-count loop barrier
    int words = 0;
    bool in_word = false;
    for (size_t i = 0; i < read_bytes; i++) {
        if (source_code[i] == ' ' || source_code[i] == '\t' || source_code[i] == '\n' || source_code[i] == '\r' || source_code[i] == '_' || source_code[i] == '-') {
            in_word = false;
        } else if (!in_word) {
            in_word = true;
            words++;
        }
    }
    assert(words > 1);

    // Compile using our new tsfi2 compiler!
    uint8_t bytecode[256];
    size_t bytecode_len = 0;
    
    printf("[Runner] Compiling tests/test_auncient_tsv_wmq_integration.c with tsfi2 compiler...\n");
    bool ok = tsfi2_compile(source_code, bytecode, sizeof(bytecode), &bytecode_len);
    assert(ok == true);
    
    // Package to dat.bin
    const char *prog_file = "/tmp/test_auncient_tsv_wmq_integration_out.dat.bin";
    const char *mount_keys = NULL;
    const char *mount_vals = NULL;
    if (strstr(source_code, "// wmq_mount STANAG")) {
        mount_keys = "NetworkMount";
        mount_vals = "STANAG";
    } else if (strstr(source_code, "// wmq_mount DECNET")) {
        mount_keys = "NetworkMount";
        mount_vals = "DECNET";
    }
    
    if (mount_keys && mount_vals) {
        ok = tsfi2_compile_to_dat_bin_ext(prog_file, 0x1000, 1, mount_keys, mount_vals, bytecode, bytecode_len);
    } else {
        ok = tsfi2_compile_to_dat_bin(prog_file, 0x1000, 1, bytecode, bytecode_len);
    }
    assert(ok == true);

    printf("[Runner] Generating mock HathiTrust databases for wmq integration...\n");
    FILE *ht_pf = fopen("/tmp/ht_primary.dat.bin", "wb");
    assert(ht_pf != NULL);
    uint8_t spacer[128] = {0};
    fwrite(spacer, 1, 128, ht_pf);
    const char *mock_line = "HT_CATALOG_RECORD_MATCH_SUCCESS\n";
    fwrite(mock_line, 1, strlen(mock_line), ht_pf);
    fclose(ht_pf);

    uint32_t rec_offset = 128;
    InteropQuadNode aix_nodes[1] = {
        { 0, 0, 100, 100, rec_offset, { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF } }
    };
    bool aix_ok = tsfi_qt_ksds_aix_write(
        "/tmp/ht_aix_isbn.dat.bin",
        "AUNCIENT_AIX\nQuadtreeCount:\t1\nRecordCount:\t1",
        aix_nodes,
        1,
        &rec_offset,
        1
    );
    assert(aix_ok == true);

    printf("[Runner] Executing compiled TSV-mounted virtual hardware transaction...\n");
    Tsfi2CpuState cpu;
    ok = tsfi2_load_and_execute(prog_file, &cpu);
    assert(ok == true);
    assert(cpu.halted == true);
    assert(cpu.exit_code == 899025);

    remove("/tmp/ht_primary.dat.bin");
    remove("/tmp/ht_aix_isbn.dat.bin");

    // Verify DisplacementShader integration Pacings driven by WinchesterMQ boundary constraints
    TSFiDisplacementShader ds;
    tsfi_displacementshader_init(&ds, 2.5, 1.5);
    double disp_wrap = tsfi_displacementshader_eval(&ds, 256.0 + M_PI / 3.0, 0.0);
    assert(fabs(disp_wrap - 2.5) < 1e-5);
    printf("   ✓ WinchesterMQ vertex displacement math scaling verified successfully.\n");

    printf("   ✓ TSV-mounted validation finished successfully.\n");
    
    // Print Code Size Footprint Comparison
    printf("\n=============================================================\n");
    printf("CODE SIZE FOOTPRINT COMPARISON (BOILERPLATE vs TSV RELOCATION)\n");
    printf("=============================================================\n");
    
    int original_file_size = 10671; // Size of legacy test_auncient_alu_wmq_integration.c in bytes
    int tsv_file_size = (int)strlen(source_code);
    double reduction = (1.0 - ((double)tsv_file_size / original_file_size)) * 100.0;
    
    printf("Original ALU Test Source:   %d bytes\n", original_file_size);
    printf("New TSV-Relocated Source:   %d bytes\n", tsv_file_size);
    printf("Source Size Reduction:      %.2f%%\n", reduction);
    
    int original_bytecode_len = 17;     // Legacy emitted instructions (setting immediate registers)
    int tsv_bytecode_len = (int)bytecode_len;
    double bytecode_reduction = (1.0 - ((double)tsv_bytecode_len / original_bytecode_len)) * 100.0;

    printf("Legacy Emitted Bytecode:    %d bytes\n", original_bytecode_len);
    printf("TSV-Relocated Bytecode:     %d bytes\n", tsv_bytecode_len);
    printf("Bytecode Size Reduction:    %.2f%%\n", bytecode_reduction);
    printf("=============================================================\n");

    // Phase 5: Compile and Execute the transitioned gost_intrusion strategy closure
    printf("\n[Runner] Loading transitioned gost_intrusion strategy from disk...\n");
    FILE *gf = fopen("solidity/dysnomia/domain/strategies/gost_intrusion.strategy", "r");
    assert(gf != NULL);
    char gost_source[1024];
    size_t gost_bytes = fread(gost_source, 1, sizeof(gost_source) - 1, gf);
    gost_source[gost_bytes] = '\0';
    fclose(gf);

    uint8_t gost_bytecode[256];
    size_t gost_bytecode_len = 0;
    ok = tsfi2_compile(gost_source, gost_bytecode, sizeof(gost_bytecode), &gost_bytecode_len);
    assert(ok == true);

    const char *gost_bin = "/tmp/gost_strategy_out.dat.bin";
    ok = tsfi2_compile_to_dat_bin_ext(gost_bin, 0x1000, 1, "TIN", "950000000", gost_bytecode, gost_bytecode_len);
    assert(ok == true);

    printf("[Runner] Executing compiled gost_intrusion strategy closure...\n");
    
    // Programmatically redirect stdout to verify DISPLAY output
    fflush(stdout);
    int stdout_fd = dup(1);
    assert(stdout_fd >= 0);
    FILE *temp_out = freopen("/tmp/captured_stdout.txt", "w", stdout);
    assert(temp_out != NULL);

    ok = tsfi2_load_and_execute(gost_bin, &cpu);

    fflush(stdout);
    dup2(stdout_fd, 1);
    close(stdout_fd);
    
    // Clear error state and reopen stdout
    clearerr(stdout);
    stdout = fdopen(1, "w");
    assert(stdout != NULL);

    // Read the captured stdout from file
    FILE *rf = fopen("/tmp/captured_stdout.txt", "r");
    assert(rf != NULL);
    char out_buf[2048];
    size_t r_bytes = fread(out_buf, 1, sizeof(out_buf) - 1, rf);
    out_buf[r_bytes] = '\0';
    fclose(rf);
    remove("/tmp/captured_stdout.txt");

    // Print captured log back to actual stdout so it remains visible
    printf("%s", out_buf);

    assert(ok == true);
    assert(cpu.halted == true);
    assert(cpu.exit_code == 0);
    
    // Assert that the emulated JCL DISPLAY lines were actually printed during execution
    assert(strstr(out_buf, "[JCL] EXECUTE COBOL ADVERSARY GOST LOOP...") != NULL);
    assert(strstr(out_buf, "[JCL] PROCESSED IDENTITY: 050051122") != NULL);
    int is_valid = 1;
    int spool_res = tsfi_mf_es_evm_spool_guard(gost_source, &is_valid);
    int lockout = (spool_res == 1) ? 1 : 0;
    bool allowed = (is_valid == 1);
    assert(allowed == false);
    assert(lockout == 1);
    printf("   ✓ Spool guard security lockout verified via real tsfi_mf_es_evm_spool_guard successfully.\n");

    remove(gost_bin);

    // Phase 6: Compile and Execute the transitioned teddy_bear_endowment strategy closure
    printf("\n[Runner] Loading transitioned teddy_bear_endowment strategy from disk...\n");
    FILE *tf = fopen("solidity/dysnomia/domain/strategies/teddy_bear_endowment.strategy", "r");
    assert(tf != NULL);
    char teddy_bear_source[1024];
    size_t teddy_bear_bytes = fread(teddy_bear_source, 1, sizeof(teddy_bear_source) - 1, tf);
    teddy_bear_source[teddy_bear_bytes] = '\0';
    fclose(tf);

    uint8_t teddy_bear_bytecode[256];
    size_t teddy_bear_bytecode_len = 0;
    ok = tsfi2_compile(teddy_bear_source, teddy_bear_bytecode, sizeof(teddy_bear_bytecode), &teddy_bear_bytecode_len);
    assert(ok == true);

    const char *teddy_bear_bin = "/tmp/teddy_bear_strategy_out.dat.bin";
    ok = tsfi2_compile_to_dat_bin_ext(teddy_bear_bin, 0x1000, 1, "TIN", "950000000", teddy_bear_bytecode, teddy_bear_bytecode_len);
    assert(ok == true);

    printf("[Runner] Executing compiled teddy_bear_endowment strategy closure...\n");
    memset(&cpu, 0, sizeof(cpu));
    ok = tsfi2_load_and_execute(teddy_bear_bin, &cpu);
    assert(ok == true);
    assert(cpu.halted == true);
    assert(cpu.exit_code == 1000000);
    printf("   ✓ Hogan newborn teddy bear 1,000,000 Saat endowment strategy verified successfully.\n");
    remove(teddy_bear_bin);

    // Phase 7: Compile and Execute the transitioned xpl_test strategy
    printf("\n[Runner] Loading transitioned xpl_test strategy from disk...\n");
    FILE *xf = fopen("solidity/dysnomia/domain/strategies/xpl_test.strategy", "r");
    assert(xf != NULL);
    char xpl_source[1024];
    size_t xpl_bytes = fread(xpl_source, 1, sizeof(xpl_source) - 1, xf);
    xpl_source[xpl_bytes] = '\0';
    fclose(xf);

    uint8_t xpl_bytecode[256];
    size_t xpl_bytecode_len = 0;
    ok = tsfi2_compile(xpl_source, xpl_bytecode, sizeof(xpl_bytecode), &xpl_bytecode_len);
    assert(ok == true);

    const char *xpl_bin = "/tmp/xpl_strategy_out.dat.bin";
    ok = tsfi2_compile_to_dat_bin_ext(xpl_bin, 0x1000, 1, "TIN", "950000000", xpl_bytecode, xpl_bytecode_len);
    assert(ok == true);

    printf("[Runner] Executing compiled xpl_test strategy...\n");
    memset(&cpu, 0, sizeof(cpu));
    ok = tsfi2_load_and_execute(xpl_bin, &cpu);
    assert(ok == true);
    assert(cpu.halted == true);
    printf("   ✓ XPL strategy validation finished successfully.\n");
    remove(xpl_bin);

    remove(prog_file);
    return 0;
}
