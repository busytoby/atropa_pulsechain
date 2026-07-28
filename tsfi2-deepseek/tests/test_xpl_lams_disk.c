#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>

// Library paths
#define BASE_DIR "lams_libraries"
#define MACLIB_DIR "lams_libraries/maclib"
#define SRCLIB_DIR "lams_libraries/srclib"
#define OBJLIB_DIR "lams_libraries/objlib"
#define LOADLIB_DIR "lams_libraries/loadlib"
#define CNTLLIB_DIR "lams_libraries/cntllib"

// Helper to create directory
static void make_dir(const char *path) {
#ifdef _WIN32
    mkdir(path);
#else
    mkdir(path, 0777);
#endif
}

// 1. RED ANALYZER Gating: Static file size validator
static bool red_analyzer_validate_file_size(const char *filepath) {
    FILE *f = fopen(filepath, "r");
    if (!f) return false;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);

    // Rule 8: 68KB Source file limit
    if (size >= 68000) {
        return false; // Rejected by RED rail
    }
    return true; // Accepted
}

// 2. BLACK XPLSM Gating: JCL assembler and linker simulation
static bool black_xplsm_run_compiler(const char *cntl_name) {
    char cntl_path[128];
    snprintf(cntl_path, sizeof(cntl_path), "%s/%s.xpl", CNTLLIB_DIR, cntl_name);

    FILE *f = fopen(cntl_path, "r");
    if (!f) return false;

    char line[128];
    bool code_written = false;

    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "COMPILE", 7) == 0) {
            char src_name[64] = {0};
            char mac_name[64] = {0};
            
            if (sscanf(line, "COMPILE %63s WITH %63s", src_name, mac_name) == 2) {
                // Simulate reading source and using macro to generate object
                char obj_path[128];
                snprintf(obj_path, sizeof(obj_path), "%s/%s.obj.dat.bin", OBJLIB_DIR, src_name);
                
                // Rule 13: Must output .dat.bin files
                FILE *obj_file = fopen(obj_path, "wb");
                if (obj_file) {
                    fprintf(obj_file, "OBJECT CODE FROM %s USING MACRO %s", src_name, mac_name);
                    fclose(obj_file);
                    code_written = true;
                }
            }
        }
    }
    fclose(f);
    return code_written;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT LAMS FILE-BASED CBT LIBRARIES VALIDATION SUITE\n");
    printf("=============================================================\n");

    // Initialize physical directories representing CBT libraries
    make_dir(BASE_DIR);
    make_dir(MACLIB_DIR);
    make_dir(SRCLIB_DIR);
    make_dir(OBJLIB_DIR);
    make_dir(LOADLIB_DIR);
    make_dir(CNTLLIB_DIR);

    // Setup mock members on disk
    const char *macro_path = MACLIB_DIR "/write_abd.mac";
    FILE *mac = fopen(macro_path, "w");
    if (mac) {
        fprintf(mac, "MACRO WRITE_ABD\\n");
        fclose(mac);
    }

    const char *source_path = SRCLIB_DIR "/mainjcl.xpl";
    FILE *src = fopen(source_path, "w");
    if (src) {
        fprintf(src, "WRITE_ABD 8585 1\\n");
        fclose(src);
    }

    const char *control_path = CNTLLIB_DIR "/compile_job.xpl";
    FILE *cntl = fopen(control_path, "w");
    if (cntl) {
        fprintf(cntl, "COMPILE mainjcl WITH write_abd\\n");
        fclose(cntl);
    }

    // Test 1: RED rail file size audits
    printf("[TEST] Checking source file sizes on disk against RED rail...\n");
    assert(red_analyzer_validate_file_size(source_path) == true);
    
    // Create oversized file to test rejection
    const char *over_path = SRCLIB_DIR "/oversize.xpl";
    FILE *over = fopen(over_path, "w");
    if (over) {
        for (int i = 0; i < 7000; i++) {
            fprintf(over, "1234567890\n"); // 70,000 bytes
        }
        fclose(over);
    }
    assert(red_analyzer_validate_file_size(over_path) == false);
    remove(over_path);
    printf("   ✓ RED rail successfully audited file size constraints on physical storage.\n");

    // Test 2: BLACK rail JCL compiler execution
    printf("[TEST] Running JCL compiler and resolving macro libraries...\n");
    assert(black_xplsm_run_compiler("compile_job") == true);
    
    // Verify compiled object file outputs
    char expected_obj_path[128];
    snprintf(expected_obj_path, sizeof(expected_obj_path), "%s/mainjcl.obj.dat.bin", OBJLIB_DIR);
    FILE *chk = fopen(expected_obj_path, "rb");
    assert(chk != NULL);
    char buf[128] = {0};
    assert(fread(buf, 1, sizeof(buf), chk) > 0);
    assert(strstr(buf, "mainjcl") != NULL);
    fclose(chk);
    remove(expected_obj_path);
    printf("   ✓ BLACK rail successfully compiled source modules to binary objects.\n");

    // Cleanup mock library structures
    remove(macro_path);
    remove(source_path);
    remove(control_path);
    remove(MACLIB_DIR);
    remove(SRCLIB_DIR);
    remove(OBJLIB_DIR);
    remove(LOADLIB_DIR);
    remove(CNTLLIB_DIR);
    remove(BASE_DIR);

    printf("=============================================================\n");
    printf("ALL FILE-BASED CBT LIBRARIES TESTS PASSED CLEANLY\n");
    printf("=============================================================\n");
    return 0;
}
