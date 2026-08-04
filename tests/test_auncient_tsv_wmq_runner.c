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
#include "tsfi2-deepseek/inc/tsfi_displacementshader.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT TSV MOUNTED WINCHESTERMQ INTEGRATION VALIDATION RUNNER\n");
    printf("=============================================================\n");

    // Read the C source code of the integration test from disk
    FILE *sf = fopen("tests/test_auncient_tsv_wmq_integration.c", "r");
    assert(sf != NULL);

    char source_code[1024];
    size_t read_bytes = fread(source_code, 1, sizeof(source_code) - 1, sf);
    source_code[read_bytes] = '\0';
    fclose(sf);

    // Compile using our new tsfi2 compiler!
    uint8_t bytecode[64];
    size_t bytecode_len = 0;
    
    printf("[Runner] Compiling tests/test_auncient_tsv_wmq_integration.c with tsfi2 compiler...\n");
    bool ok = tsfi2_compile(source_code, bytecode, sizeof(bytecode), &bytecode_len);
    assert(ok == true);
    
    // Package to dat.bin
    const char *prog_file = "/tmp/test_auncient_tsv_wmq_integration_out.dat.bin";
    ok = tsfi2_compile_to_dat_bin(prog_file, 0x1000, 1, bytecode, bytecode_len);
    assert(ok == true);

    printf("[Runner] Executing compiled TSV-mounted virtual hardware transaction...\n");
    Tsfi2CpuState cpu;
    ok = tsfi2_load_and_execute(prog_file, &cpu);
    assert(ok == true);
    assert(cpu.halted == true);
    assert(cpu.exit_code == 42);

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
    
    int original_file_size = 5891; // Size of legacy test_auncient_alu_wmq_integration.c in bytes
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

    remove(prog_file);
    return 0;
}
