#include "tsfi_micro_focus.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    printf("[TEST] Running Micro Focus COBOL standard compatibility checks...\n");

    // 1. Verify COMP-5 Native Binary Handling
    uint8_t buffer[8] = {0};
    int res = tsfi_mf_comp5_encode(0x12345678, buffer, 4);
    assert(res == 0);
    // On x86/little-endian, it is stored in little-endian order
    assert(buffer[0] == 0x78);
    assert(buffer[1] == 0x56);
    assert(buffer[2] == 0x34);
    assert(buffer[3] == 0x12);

    int64_t val = tsfi_mf_comp5_decode(buffer, 4, 1);
    assert(val == 0x12345678);

    // Negative sign extension test
    memset(buffer, 0xFF, sizeof(buffer));
    int64_t neg_val = tsfi_mf_comp5_decode(buffer, 2, 1);
    assert(neg_val == -1);

    // 2. Verify ISAM File Handling (.dat.bin per Rule 13)
    const char *filepath = "test_isam.dat.bin";
    const char *invalid_filepath = "test_isam.json";

    unlink(filepath); // Remove if exists

    // Test Rule 13 failure
    uint8_t dummy_data[4] = {1, 2, 3, 4};
    int res_invalid = tsfi_mf_isam_write_record(invalid_filepath, 100, dummy_data, 4);
    assert(res_invalid == -2); // Blocked extension

    // Write valid records
    int res_write1 = tsfi_mf_isam_write_record(filepath, 1001, (const uint8_t *)"ISAMRecord1", 11);
    int res_write2 = tsfi_mf_isam_write_record(filepath, 1002, (const uint8_t *)"ISAMRecordTwo", 13);
    assert(res_write1 == 0);
    assert(res_write2 == 0);

    // Read valid records
    uint8_t read_buf[32] = {0};
    int res_read1 = tsfi_mf_isam_read_record(filepath, 1001, read_buf, sizeof(read_buf));
    assert(res_read1 == 0);
    assert(strncmp((const char *)read_buf, "ISAMRecord1", 11) == 0);

    memset(read_buf, 0, sizeof(read_buf));
    int res_read2 = tsfi_mf_isam_read_record(filepath, 1002, read_buf, sizeof(read_buf));
    assert(res_read2 == 0);
    assert(strncmp((const char *)read_buf, "ISAMRecordTwo", 13) == 0);

    // Read nonexistent record
    int res_read_nonexistent = tsfi_mf_isam_read_record(filepath, 9999, read_buf, sizeof(read_buf));
    assert(res_read_nonexistent == -7);

    unlink(filepath); // Cleanup

    // 3. Verify SIF CAD Parser Integration
    printf("[TEST] Validating SIF CAD parsing...\n");
    tsfi_cgm_scene sif_scene;
    tsfi_cgm_scene_init(&sif_scene);
    const char *sif_line = "SIF_SPHERE X:2.5 Y:-1.0 Z:8.0 R:3.5 COLOR:B";
    int sif_res = tsfi_mf_sif_parse(sif_line, &sif_scene);
    assert(sif_res == 0);
    assert(sif_scene.primitive_count == 1);
    assert(sif_scene.primitives[0].type == CGM_PRIM_SPHERE);
    assert(sif_scene.primitives[0].position.x == 2.5f);
    assert(sif_scene.primitives[0].param1 == 3.5f);
    assert(sif_scene.primitives[0].color.z == 1.0f); // Blue
    printf("  [PASS] SIF CAD coordinate translation verified.\n");

    // 4. Verify Screen Section Terminal Layout
    printf("[TEST] Validating Screen Section rendering...\n");
    char term_buf[80 * 24];
    const char *screen_def = "LINE:2 COL:15 VALUE:SYSTEM OK";
    int term_res = tsfi_mf_screen_section_render(screen_def, term_buf, 80, 24);
    assert(term_res == 0);
    // At line 2 (offset 1), col 15 (offset 14): check content
    assert(strncmp(&term_buf[1 * 80 + 14], "SYSTEM OK", 9) == 0);
    printf("  [PASS] Screen Section layout rendering verified.\n");

    printf("[SUCCESS] Micro Focus COBOL standard compatibility checks completed successfully!\n");
    return 0;
}
