#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_autodin_tape_proof.h"
#include "tsfi_parc_tape_label_yul.h"

int main() {
    printf("=== TSFi AUTODIN Tape Proof Sequential History Test ===\n");

    const char *test_file_path = "mock_tape_proof.dat.bin";
    uint8_t mock_header[720];
    memset(mock_header, 0, sizeof(mock_header));

    // 1. Initialize a valid 720-byte Yul DDL header using tsfi_tape_label_yul_format_full_header
    printf("[AUTODIN] Constructing mock Yul DDL label sequence...\n");
    int init_res = tsfi_tape_label_yul_format_full_header(
        mock_header,
        "VOL999",                 // Volume ID
        "test_file.dat.bin",      // File ID (Rule 13 compliant)
        2,                        // Security level
        0.0f, 0.0f, 100.0f, 100.0f, // Spatial bounds
        "VOL998", "VOL100",       // Prev / Next volumes
        1.57f,                    // Phase phi
        32, 30, 22,               // fx, fy, fz
        32, 30                    // Winchester keycode registers (32=d/D, 30=a/A)
    );
    assert(init_res == 0);

    // Write mock header to temporary file
    FILE *f = fopen(test_file_path, "wb");
    assert(f != NULL);
    size_t nwritten = fwrite(mock_header, 1, 720, f);
    assert(nwritten == 720);
    fclose(f);

    // 2. Execute verification loop via tsfi_autodin_verify_tape_file
    printf("[AUTODIN] Verifying mock tape file clearance and signatures...\n");
    tsfi_autodin_tape_proof_result_t result;
    int verify_res = tsfi_autodin_verify_tape_file(test_file_path, 2, &result);
    assert(verify_res == 0);
    
    // Validate returned telemetry values
    assert(result.sig_valid == 1);
    assert(result.proof_valid == 1);
    assert(strcmp(result.volume_id, "VOL999") == 0);
    assert(strcmp(result.file_id, "test_file.dat.bin") == 0);
    assert(result.security_level == 2);
    assert(strcmp(result.state_hash_hex, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855") == 0);
    printf("[AUTODIN] Success: Cryptographic signature and Merkle state hash confirmed.\n");

    // 3. Test verification failure: Insufficient security clearance
    printf("[AUTODIN] Verifying governance enforcement with insufficient clearance...\n");
    int clearance_res = tsfi_autodin_verify_tape_file(test_file_path, 1, &result);
    // Should return non-zero indicating a governance error
    assert(clearance_res != 0);
    printf("[AUTODIN] Success: Blocked unauthorized reader clearance levels.\n");

    // 4. Test validation constraint: Rule 13 invalid extension (.json)
    const char *invalid_ext_path = "mock_tape_proof.json";
    FILE *f_bad = fopen(invalid_ext_path, "wb");
    assert(f_bad != NULL);
    fwrite(mock_header, 1, 720, f_bad);
    fclose(f_bad);

    printf("[AUTODIN] Verifying extension checks for Rule 13 (banning .json)...\n");
    int rule13_res = tsfi_autodin_verify_tape_file(invalid_ext_path, 2, &result);
    assert(rule13_res == -2); // Indicates extension error
    printf("[AUTODIN] Success: Blocked non-conforming file extensions.\n");

    // Cleanup files
    remove(test_file_path);
    remove(invalid_ext_path);

    printf("=== ALL AUTODIN TAPE PROOF TESTS PASSED ===\n");
    return 0;
}
