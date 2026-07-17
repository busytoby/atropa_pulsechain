#include "tsfi_micro_focus.h"
#include "tsfi_ramac_layout.h"
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

    // Load multi-primitive Teddy Bear model
    const char *teddy_sif[] = {
        "SIF_SPHERE X:0.0 Y:1.5 Z:8.0 R:1.5 COLOR:G",   // Head
        "SIF_SPHERE X:0.0 Y:-0.5 Z:8.0 R:2.0 COLOR:G",  // Body
        "SIF_SPHERE X:-1.2 Y:2.7 Z:8.0 R:0.6 COLOR:B",  // Left Ear
        "SIF_SPHERE X:1.2 Y:2.7 Z:8.0 R:0.6 COLOR:B",   // Right Ear
        "SIF_SPHERE X:-0.5 Y:1.8 Z:6.6 R:0.15 COLOR:B", // Left Eye
        "SIF_SPHERE X:0.5 Y:1.8 Z:6.6 R:0.15 COLOR:B",  // Right Eye
        "SIF_SPHERE X:0.0 Y:1.3 Z:6.5 R:0.4 COLOR:G",   // Nose
        "SIF_SPHERE X:-1.8 Y:-0.2 Z:8.0 R:0.8 COLOR:G", // Left Arm
        "SIF_SPHERE X:1.8 Y:-0.2 Z:8.0 R:0.8 COLOR:G",  // Right Arm
        "SIF_SPHERE X:-1.2 Y:-2.2 Z:8.0 R:0.9 COLOR:G", // Left Leg
        "SIF_SPHERE X:1.2 Y:-2.2 Z:8.0 R:0.9 COLOR:G"   // Right Leg
    };
    for (int i = 0; i < 11; i++) {
        int res = tsfi_mf_sif_parse(teddy_sif[i], &sif_scene);
        assert(res == 0);
    }
    assert(sif_scene.primitive_count == 12);
    printf("  [PASS] SIF CAD Teddy Bear model parser verified (12 primitives).\n");

    // Verify SIF_PLANE parsing
    const char *plane_line = "SIF_PLANE NX:0.0 NY:1.0 NZ:0.0 D:-5.0 COLOR:G";
    int plane_res = tsfi_mf_sif_parse(plane_line, &sif_scene);
    assert(plane_res == 0);
    assert(sif_scene.primitive_count == 13);
    assert(sif_scene.primitives[12].type == CGM_PRIM_PLANE);
    assert(sif_scene.primitives[12].position.y == 1.0f); // Normal vector Y
    assert(sif_scene.primitives[12].param1 == -5.0f); // Distance
    assert(sif_scene.primitives[12].color.y == 1.0f); // Green
    printf("  [PASS] SIF CAD plane primitive parser verified.\n");

    // Verify SIF serialization
    char serialize_buf[1024];
    int ser_res = tsfi_mf_sif_serialize(&sif_scene, serialize_buf, sizeof(serialize_buf));
    assert(ser_res == 0);
    assert(strstr(serialize_buf, "SIF_SPHERE X:2.5 Y:-1.0 Z:8.0 R:3.5 COLOR:B") != NULL);
    assert(strstr(serialize_buf, "SIF_PLANE NX:0.0 NY:1.0 NZ:0.0 D:-5.0 COLOR:G") != NULL);
    printf("  [PASS] SIF CAD serialization verified.\n");

    // 4. Verify Screen Section Terminal Layout
    printf("[TEST] Validating Screen Section rendering...\n");
    char term_buf[80 * 24];
    const char *screen_def = "LINE:2 COL:15 VALUE:SYSTEM OK";
    int term_res = tsfi_mf_screen_section_render(screen_def, term_buf, 80, 24);
    assert(term_res == 0);
    // At line 2 (offset 1), col 15 (offset 14): check content
    assert(strncmp(&term_buf[1 * 80 + 14], "SYSTEM OK", 9) == 0);
    printf("  [PASS] Screen Section layout rendering verified.\n");

    // 5. Verify Michael Coughlan Stock Inventory Processing
    printf("[TEST] Validating Coughlan Stock Inventory record processing...\n");
    char raw_record[24];
    memcpy(raw_record, "A001", 4);
    memcpy(raw_record + 4, "TeddyBear   ", 12);
    tsfi_mf_comp5_encode(5, (uint8_t*)(raw_record + 16), 4);
    tsfi_mf_comp5_encode(100, (uint8_t*)(raw_record + 20), 4);

    char report_line[128];
    int stock_res = tsfi_mf_stock_inventory_process(raw_record, report_line);
    assert(stock_res == 0);
    assert(strstr(report_line, "ID:A001") != NULL);
    assert(strstr(report_line, "QTY:0005") != NULL);
    assert(strstr(report_line, "VALUE:00000500") != NULL);
    assert(strstr(report_line, "STATUS:LOW STOCK") != NULL);
    printf("  [PASS] Coughlan Stock record layout and explicit status verified.\n");

    // Verify Michael Coughlan Sales Commission Processing
    printf("[TEST] Validating Coughlan Sales Commission record processing...\n");
    char sales_record[24];
    memcpy(sales_record, "R999", 4);
    memcpy(sales_record + 4, "SalesAgent  ", 12);
    tsfi_mf_comp5_encode(15000, (uint8_t*)(sales_record + 16), 4);

    char comm_line[128];
    int comm_res = tsfi_mf_sales_commission_process(sales_record, comm_line);
    assert(comm_res == 0);
    assert(strstr(comm_line, "REP:R999") != NULL);
    assert(strstr(comm_line, "SALES:015000") != NULL);
    assert(strstr(comm_line, "COMM:000900") != NULL); // (10000*0.05) + (5000*0.08) = 500 + 400 = 900
    printf("  [PASS] Coughlan Sales Commission calculation and record layout verified.\n");

    // Verify Diyat Tax calculations
    printf("[TEST] Validating Diyat Tax calculations...\n");
    uint32_t tax_val = 0;
    int diyat_res = tsfi_mf_calculate_diyat_tax("SEMI", 100000, &tax_val);
    assert(diyat_res == 0);
    assert(tax_val == 120000);
    diyat_res = tsfi_mf_calculate_diyat_tax("HALF", 100000, &tax_val);
    assert(diyat_res == 0);
    assert(tax_val == 50000);
    printf("  [PASS] Diyat Tax rates verified without level-88 indicators.\n");

    // Verify Diyat Yul Gas Tax Excision
    printf("[TEST] Validating Diyat Yul Gas Tax Excision...\n");
    extern _Thread_local uint64_t g_transaction_diyat_tax_total;
    uint64_t initial_tax = g_transaction_diyat_tax_total;
    int excise_res = tsfi_diyat_yul_excise_gas_taxes(50000, "0xD32c39fEE49391c7952d1b30b15921b0D3b42E69");
    assert(excise_res == 0);
    extern uint64_t lau_yul_thunk_sload(uint64_t key);
    assert(lau_yul_thunk_sload(0xF199) == 5000); // 10% of 50000 gas
    assert(g_transaction_diyat_tax_total == initial_tax + 5000);
    printf("  [PASS] Diyat Yul Gas Tax Excision on 2-3 tree blockchain verified.\n");

    // Verify S370 Yul execution bridge
    printf("[TEST] Validating S370 Yul execution bridge...\n");
    tsfi_uncol_instruction test_prog[2];
    strcpy(test_prog[0].op, "LOAD");
    test_prog[0].reg_dest = 1;
    test_prog[0].address = 100;
    strcpy(test_prog[1].op, "STORE");
    test_prog[1].reg_dest = 1;
    test_prog[1].address = 200;
    int bridge_res = tsfi_s370_yul_exec_bridge(test_prog, 2);
    assert(bridge_res == 0);
    printf("  [PASS] S370 Yul execution bridge verified.\n");

    printf("[SUCCESS] Micro Focus COBOL standard compatibility checks completed successfully!\n");
    return 0;
}
