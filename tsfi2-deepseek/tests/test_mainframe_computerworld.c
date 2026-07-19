#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "tsfi_ramac_layout.h"
#include "tsfi_wire_firmware.h"
#include "tsfi_mainframe_computerworld.h"

static void test_y2k_windowing(void) {
    printf("[Computerworld Test] Verifying Y2K Date Windowing (Pivot 50)...\n");
    
    // Resolve year checks
    assert(tsfi_cw_y2k_resolve_year(49) == 2049);
    assert(tsfi_cw_y2k_resolve_year(50) == 1950);
    assert(tsfi_cw_y2k_resolve_year(99) == 1999);
    assert(tsfi_cw_y2k_resolve_year(0) == 2000);

    // Validate dates
    int is_valid = 0;
    // Leap years
    assert(tsfi_cw_y2k_check_date(0, 2, 29, &is_valid) == 0 && is_valid == 1); // 2000 is leap
    assert(tsfi_cw_y2k_check_date(96, 2, 29, &is_valid) == 0 && is_valid == 1); // 1996 is leap
    assert(tsfi_cw_y2k_check_date(97, 2, 29, &is_valid) == 0 && is_valid == 0); // 1997 not leap
    
    // Normal bounds
    assert(tsfi_cw_y2k_check_date(26, 12, 31, &is_valid) == 0 && is_valid == 1);
    assert(tsfi_cw_y2k_check_date(26, 13, 1, &is_valid) == 0 && is_valid == 0);
    assert(tsfi_cw_y2k_check_date(26, 6, 31, &is_valid) == 0 && is_valid == 0); // June has 30 days
}

static void test_ebcdic_translation(void) {
    printf("[Computerworld Test] Verifying EBCDIC <-> ASCII translations...\n");
    
    const char *ascii_str = "MAINFRAME1979";
    uint8_t ebcdic_buf[32];
    char ascii_back[32];
    
    int rc = tsfi_cw_ascii_to_ebcdic_buf(ascii_str, ebcdic_buf, 13);
    assert(rc == 0);
    
    // EBCDIC characters for 'M', 'A', 'I', 'N'
    assert(ebcdic_buf[0] == 0xD4); // M
    assert(ebcdic_buf[1] == 0xC1); // A
    assert(ebcdic_buf[2] == 0xC9); // I
    
    rc = tsfi_cw_ebcdic_to_ascii_buf(ebcdic_buf, ascii_back, 13);
    assert(rc == 0);
    assert(strcmp(ascii_str, ascii_back) == 0);
}

static void test_punch_card_parsing(void) {
    printf("[Computerworld Test] Verifying IBM 80-Column Punch Card Record Parser...\n");
    // Construct an exact 80-column punch card deck line
    char card[81];
    memset(card, ' ', 80);
    memcpy(card, "001000", 6);
    card[6] = '*';
    memcpy(card + 7, "  MO", 4);
    memcpy(card + 11, "VEAUNCIDENT TO VALUE-OUT", 24);
    memcpy(card + 72, "PROG0001", 8);
    card[80] = '\0';

    tsfi_cw_punch_card out;
    int rc = tsfi_cw_parse_punch_card(card, &out);
    assert(rc == 0);
    
    assert(strcmp(out.sequence, "001000") == 0);
    assert(out.indicator == '*');
    assert(strcmp(out.area_a, "  MO") == 0);
    assert(strncmp(out.area_b, "VEAUNCIDENT TO VALUE-OUT", 24) == 0);
    assert(strcmp(out.identification, "PROG0001") == 0);
}

static void test_cobol_comp3(void) {
    printf("[Computerworld Test] Verifying COBOL COMP-3 Packed Decimal Standard...\n");
    
    uint8_t comp3[16];
    int comp3_len = 0;
    char ascii[32];
    
    // Positive odd digits: "12345" -> 0x12 0x34 0x5C
    int rc = tsfi_cw_pack_comp3("12345", comp3, sizeof(comp3), &comp3_len);
    assert(rc == 0);
    assert(comp3_len == 3);
    assert(comp3[0] == 0x12);
    assert(comp3[1] == 0x34);
    assert(comp3[2] == 0x5C);
    
    rc = tsfi_cw_unpack_comp3(comp3, comp3_len, ascii, sizeof(ascii));
    assert(rc == 0);
    assert(strcmp(ascii, "12345") == 0);

    // Negative even digits: "-4567" -> 0x04 0x56 0x7D
    rc = tsfi_cw_pack_comp3("-4567", comp3, sizeof(comp3), &comp3_len);
    assert(rc == 0);
    assert(comp3_len == 3);
    assert(comp3[0] == 0x04);
    assert(comp3[1] == 0x56);
    assert(comp3[2] == 0x7D);
    
    rc = tsfi_cw_unpack_comp3(comp3, comp3_len, ascii, sizeof(ascii));
    assert(rc == 0);
    assert(strcmp(ascii, "-4567") == 0);
    
    // Zero: "0" -> 0x0C
    rc = tsfi_cw_pack_comp3("0", comp3, sizeof(comp3), &comp3_len);
    assert(rc == 0);
    assert(comp3_len == 1);
    assert(comp3[0] == 0x0C);
    
    rc = tsfi_cw_unpack_comp3(comp3, comp3_len, ascii, sizeof(ascii));
    assert(rc == 0);
    assert(strcmp(ascii, "0") == 0);
}

static int omp_galasa_mock_eval(void *ctx) {
    int *attempts = (int *)ctx;
    (*attempts)++;
    return (*attempts >= 2);
}

static void test_new_mainframe_features(void) {
    remove("evm_storage.dat.bin");
    printf("[Computerworld Test] Verifying IBM HFP Conversions...\n");
    float f1 = 1.0f;
    uint32_t ibm1 = tsfi_cw_ieee_to_ibm_float(f1);
    float f1_back = tsfi_cw_ibm_to_ieee_float(ibm1);
    assert(f1_back == f1);

    float f2 = -123.45f;
    uint32_t ibm2 = tsfi_cw_ieee_to_ibm_float(f2);
    float f2_back = tsfi_cw_ibm_to_ieee_float(ibm2);
    assert(abs((int)(f2_back * 100) - (int)(f2 * 100)) <= 2);

    printf("[Computerworld Test] Verifying COBOL Copybook Parsing...\n");
    tsfi_cw_copybook cb;
    memset(&cb, 0, sizeof(cb));
    int rc = tsfi_cw_parse_copybook_line("05 CUSTOMER-NAME PIC X(25).", &cb);
    assert(rc == 0);
    assert(cb.field_count == 1);
    assert(strcmp(cb.fields[0].name, "CUSTOMER-NAME") == 0);
    assert(cb.fields[0].type == COBOL_TYPE_ALPHA);
    assert(cb.fields[0].length == 25);
    assert(cb.fields[0].offset == 0);

    rc = tsfi_cw_parse_copybook_line("10 ACCOUNT-BAL PIC S9(7)V99 COMP-3.", &cb);
    assert(rc == 0);
    assert(cb.field_count == 2);
    assert(strcmp(cb.fields[1].name, "ACCOUNT-BAL") == 0);
    assert(cb.fields[1].type == COBOL_TYPE_COMP3);
    assert(cb.fields[1].length == 5);
    assert(cb.fields[1].offset == 25);

    rc = tsfi_cw_parse_copybook_line("15 TX-COUNT PIC 9(4) COMP-5.", &cb);
    assert(rc == 0);
    assert(cb.field_count == 3);
    assert(strcmp(cb.fields[2].name, "TX-COUNT") == 0);
    assert(cb.fields[2].type == COBOL_TYPE_COMP5);
    assert(cb.fields[2].length == 2);
    assert(cb.fields[2].offset == 30);

    rc = tsfi_cw_parse_copybook_line("01 CUST-DATA PIC X(10) VALUE 'AUNCIENT'.", &cb);
    assert(rc == 0);
    assert(cb.field_count == 4);
    assert(cb.fields[3].level == 1);
    assert(strcmp(cb.fields[3].value, "AUNCIENT") == 0);

    // Punch card helper validations
    tsfi_cw_punch_card card_comm;
    card_comm.indicator = '*';
    assert(tsfi_cw_card_is_comment(&card_comm) == 1);
    assert(tsfi_cw_card_is_continuation(&card_comm) == 0);

    tsfi_cw_punch_card card_cont;
    card_cont.indicator = '-';
    assert(tsfi_cw_card_is_comment(&card_cont) == 0);
    assert(tsfi_cw_card_is_continuation(&card_cont) == 1);

    // COMP-3 negative sign B validation (0x12 0x3B -> "-123")
    uint8_t comp3_sign_b[2] = {0x12, 0x3B};
    char ascii_sign_b[16];
    assert(tsfi_cw_unpack_comp3(comp3_sign_b, 2, ascii_sign_b, sizeof(ascii_sign_b)) == 0);
    assert(strcmp(ascii_sign_b, "-123") == 0);

    printf("[Computerworld Test] Verifying VSAM KSDS Emulator...\n");
    tsfi_cw_vsam_ksds ksds;
    rc = tsfi_cw_vsam_open(&ksds, "test_vsam.dat");
    assert(rc == -5); // Rule 13 check

    rc = tsfi_cw_vsam_open(&ksds, "test_vsam.dat.bin");
    assert(rc == 0);

    uint8_t data_in[10] = {0xAA, 0xBB, 0xCC, 0xDD};
    rc = tsfi_cw_vsam_write(&ksds, "KEY3", data_in, 4);
    assert(rc == 0);
    rc = tsfi_cw_vsam_write(&ksds, "KEY1", data_in, 4);
    assert(rc == 0);
    rc = tsfi_cw_vsam_write(&ksds, "KEY2", data_in, 4);
    assert(rc == 0);

    // Check KSDS sorting order
    assert(strcmp(ksds.index[0].key, "KEY1") == 0);
    assert(strcmp(ksds.index[1].key, "KEY2") == 0);
    assert(strcmp(ksds.index[2].key, "KEY3") == 0);

    uint8_t data_out[10];
    int out_len = 0;
    rc = tsfi_cw_vsam_read(&ksds, "KEY2", data_out, sizeof(data_out), &out_len);
    assert(rc == 0);
    assert(out_len == 4);
    assert(data_out[0] == 0xAA);
    assert(data_out[3] == 0xDD);

    remove("test_vsam.dat.bin");

    printf("[Computerworld Test] Verifying Job Control Language (JCL) Simulator...\n");
    const char *jcl_deck[] = {
        "//TESTJOB JOB (ACCT),CLASS=A,MSGCLASS=X",
        "//STEP1 EXEC PGM=RECON",
        "//STEP2 EXEC PGM=COMP3",
        "//SYSIN DD *"
    };
    int steps = tsfi_cw_run_jcl(jcl_deck, 4);
    assert(steps == 2);

    const char *jcl_cond_deck[] = {
        "//STEP1 EXEC PGM=ERR",
        "//STEP2 EXEC PGM=COMP3,COND=(12,EQ)"
    };
    int steps_ex = tsfi_cw_run_jcl_ex(jcl_cond_deck, 2, 0);
    assert(steps_ex == 1); // STEP2 bypassed due to return code 12 from ERR step

    // REDEFINES parsing test
    rc = tsfi_cw_parse_copybook_line("05 ACCOUNT-STR REDEFINES ACCOUNT-BAL PIC X(5).", &cb);
    assert(rc == 0);
    assert(cb.field_count == 5);
    assert(strcmp(cb.fields[4].redefines, "ACCOUNT-BAL") == 0);
    assert(cb.fields[4].offset == 25); // overlaps with ACCOUNT-BAL

    // Julian Date Conversions
    char jul_buf[16];
    assert(tsfi_cw_gregorian_to_julian(26, 7, 18, jul_buf, sizeof(jul_buf)) == 0);
    assert(strcmp(jul_buf, "26.199") == 0);
    
    uint32_t j_yy, j_mm, j_dd;
    assert(tsfi_cw_julian_to_gregorian("26.199", &j_yy, &j_mm, &j_dd) == 0);
    assert(j_yy == 26);
    assert(j_mm == 7);
    assert(j_dd == 18);

    // EBCDIC Padding & Stripping
    uint8_t ebcdic_pad[10];
    assert(tsfi_cw_ebcdic_pad_record("AUNCIENT", ebcdic_pad, 10) == 0);
    assert(ebcdic_pad[8] == 0x40); // space pad
    
    char ascii_stripped[12];
    assert(tsfi_cw_ebcdic_strip_record(ebcdic_pad, 10, ascii_stripped, sizeof(ascii_stripped)) == 0);
    assert(strcmp(ascii_stripped, "AUNCIENT") == 0);

    // VSAM CI Split validation
    tsfi_cw_vsam_ci_set ci_set;
    tsfi_cw_vsam_ci_init(&ci_set);
    assert(tsfi_cw_vsam_ci_insert(&ci_set, 0, "KEY1") == 0);
    assert(tsfi_cw_vsam_ci_insert(&ci_set, 0, "KEY3") == 0);
    assert(tsfi_cw_vsam_ci_insert(&ci_set, 0, "KEY4") == 0);
    assert(tsfi_cw_vsam_ci_insert(&ci_set, 0, "KEY5") == 0);
    assert(tsfi_cw_vsam_ci_insert(&ci_set, 0, "KEY2") == 1); // Split occurs
    assert(ci_set.ci_count == 2);
    assert(ci_set.cis[0].record_count == 2);
    assert(strcmp(ci_set.cis[0].keys[0], "KEY1") == 0);
    assert(strcmp(ci_set.cis[0].keys[1], "KEY2") == 0);
    assert(ci_set.cis[1].record_count == 3);
    assert(strcmp(ci_set.cis[1].keys[0], "KEY3") == 0);

    // JCL SYSIN Inline card parsing
    const char *jcl_sysin_deck[] = {
        "//STEP1 EXEC PGM=RECON",
        "//SYSIN DD *",
        "RECORD1 PAYLOAD",
        "RECORD2 PAYLOAD",
        "/*",
        "//STEP2 EXEC PGM=COMP3"
    };
    char sysin_buf[128];
    int sysin_bytes = tsfi_cw_run_jcl_sysin(jcl_sysin_deck, 6, sysin_buf, sizeof(sysin_buf));
    assert(sysin_bytes > 0);
    assert(strstr(sysin_buf, "RECORD1 PAYLOAD") != NULL);
    assert(strstr(sysin_buf, "RECORD2 PAYLOAD") != NULL);
    assert(strstr(sysin_buf, "STEP2") == NULL);

    // COBOL OCCURS Clause parsing
    rc = tsfi_cw_parse_copybook_line("05 HISTORY-LIST PIC S9(7)V99 COMP-3 OCCURS 10 TIMES.", &cb);
    assert(rc == 0);
    assert(cb.field_count == 6);
    assert(cb.fields[5].occurs == 10);
    assert(cb.fields[5].length == 50); // 5 bytes base * 10 occurrences

    // Dynamic Y2K pivots
    assert(tsfi_cw_y2k_resolve_year_ex(30, 40) == 2030);
    assert(tsfi_cw_y2k_resolve_year_ex(30, 20) == 1930);
    int pivot_valid = 0;
    assert(tsfi_cw_y2k_check_date_ex(97, 2, 29, 90, &pivot_valid) == 0 && pivot_valid == 0);

    // EBCDIC Control Character mappings
    uint8_t ascii_ctrl = 0;
    assert(tsfi_cw_ebcdic_to_ascii_control(0x15, &ascii_ctrl) == 0);
    assert(ascii_ctrl == 0x0A);

    // VSAM Alternate Index
    tsfi_cw_vsam_aix alt_idx;
    tsfi_cw_vsam_aix_init(&alt_idx);
    assert(tsfi_cw_vsam_aix_add(&alt_idx, "ALTKEY1", "KEY1") == 0);
    char resolved_p_key[16];
    assert(tsfi_cw_vsam_aix_resolve(&alt_idx, "ALTKEY1", resolved_p_key) == 0);
    assert(strcmp(resolved_p_key, "KEY1") == 0);

    // JCL Dataset Dispositions
    tsfi_cw_jcl_disp jcl_disp;
    assert(tsfi_cw_jcl_parse_disp("DISP=(NEW,CATLG,DELETE)", &jcl_disp) == 0);
    assert(strcmp(jcl_disp.status, "NEW") == 0);
    assert(strcmp(jcl_disp.normal, "CATLG") == 0);
    assert(strcmp(jcl_disp.abnormal, "DELETE") == 0);

    // Zoned decimal sign encoding
    uint8_t zoned_pad[10];
    int zoned_len_out = 0;
    assert(tsfi_cw_pack_zoned("-12345", zoned_pad, 10, &zoned_len_out) == 0);
    assert(zoned_len_out == 5);
    assert(zoned_pad[4] == 0xD5); // '5' is 5, negative zone is D
    char ascii_zoned[16];
    assert(tsfi_cw_unpack_zoned(zoned_pad, 5, ascii_zoned, sizeof(ascii_zoned)) == 0);
    assert(strcmp(ascii_zoned, "-12345") == 0);

    // COBOL USAGE parsing
    rc = tsfi_cw_parse_copybook_line("05 STR-VAR PIC X(10) USAGE IS DISPLAY.", &cb);
    assert(rc == 0);
    assert(cb.fields[6].usage == COBOL_USAGE_DISPLAY);
    rc = tsfi_cw_parse_copybook_line("05 COMP-VAR PIC 9(4) USAGE COMP.", &cb);
    assert(rc == 0);
    assert(cb.fields[7].usage == COBOL_USAGE_COMP);

    // JCL PROC execution
    const char *jcl_proc_deck[] = {
        "//TESTJOB JOB (ACCT)",
        "//MYSTEP EXEC MYPROC"
    };
    const char *proc_deck[] = {
        "//STEP1 EXEC PGM=RECON",
        "//STEP2 EXEC PGM=COMP3"
    };
    int proc_steps_run = tsfi_cw_run_jcl_proc(jcl_proc_deck, 2, proc_deck, 2, 0);
    assert(proc_steps_run == 2);

    // VSAM CA Split validation
    tsfi_cw_vsam_ca_set ca_set;
    tsfi_cw_vsam_ca_init(&ca_set);
    assert(tsfi_cw_vsam_ca_insert(&ca_set, 0, "KEY1") == 0);
    assert(tsfi_cw_vsam_ca_insert(&ca_set, 0, "KEY2") == 0);
    assert(tsfi_cw_vsam_ca_insert(&ca_set, 0, "KEY3") == 0);
    assert(tsfi_cw_vsam_ca_insert(&ca_set, 0, "KEY4") == 0);
    assert(tsfi_cw_vsam_ca_insert(&ca_set, 0, "KEY5") == 1); // CI split occurs inside CA 0
    assert(ca_set.ca_count == 1);
    assert(ca_set.cis_sets[0].ci_count == 2);

    // Multi-Century Y2K resolution
    assert(tsfi_cw_y2k_resolve_year_multi(26, 18) == 1826);
    assert(tsfi_cw_y2k_resolve_year_multi(84, 19) == 1984);

    // COBOL OCCURS DEPENDING ON parsing
    rc = tsfi_cw_parse_copybook_line("05 ITEM-ARRAY PIC X(5) OCCURS 5 DEPENDING ON ITEM-COUNT.", &cb);
    assert(rc == 0);
    assert(cb.field_count == 9);
    assert(strcmp(cb.fields[8].depending_on, "ITEM-COUNT") == 0);

    // JCL Step Checkpoint Restarts
    const char *jcl_restart_deck[] = {
        "//STEP1 EXEC PGM=RECON",
        "//STEP2 EXEC PGM=COMP3",
        "//STEP3 EXEC PGM=HOGAN"
    };
    int restart_steps = tsfi_cw_run_jcl_restart(jcl_restart_deck, 3, "STEP2");
    assert(restart_steps == 2);

    // VSAM Key-Range Partitioning (KRDS)
    tsfi_cw_vsam_krds krds;
    tsfi_cw_vsam_krds_init(&krds);
    assert(tsfi_cw_vsam_krds_add_partition(&krds, "A", "M", "part1.dat.bin") == 0);
    assert(tsfi_cw_vsam_krds_add_partition(&krds, "N", "Z", "part2.dat.bin") == 0);
    assert(strcmp(tsfi_cw_vsam_krds_resolve(&krds, "G"), "part1.dat.bin") == 0);
    assert(strcmp(tsfi_cw_vsam_krds_resolve(&krds, "R"), "part2.dat.bin") == 0);

    // FB 80 Record Blocking
    uint8_t fb80_block[200];
    int recs_blocked = 0;
    int bytes_blocked = tsfi_cw_block_fb80("LINE1\nLINE2\n", fb80_block, sizeof(fb80_block), &recs_blocked);
    assert(bytes_blocked == 160);
    assert(recs_blocked == 2);
    assert(fb80_block[5] == 0x40);

    // Leap Second Correction Logs
    int leap_sec_offset = 0;
    assert(tsfi_cw_y2k_adjust_leap_seconds(1985, &leap_sec_offset) == 0);
    assert(leap_sec_offset == 19);

    // VSAM Relative Record Data Sets (RRDS)
    tsfi_cw_vsam_rrds rrds;
    tsfi_cw_vsam_rrds_init(&rrds, "test_rrds.dat.bin");
    uint8_t rrds_data[10] = {0x11, 0x22};
    assert(tsfi_cw_vsam_rrds_write(&rrds, 5, rrds_data, 2) == 0);
    uint8_t rrds_out[10];
    int rrds_out_len = 0;
    assert(tsfi_cw_vsam_rrds_read(&rrds, 5, rrds_out, sizeof(rrds_out), &rrds_out_len) == 0);
    assert(rrds_out_len == 2);
    assert(rrds_out[0] == 0xEE);

    // JCL Procedure overrides (SET)
    const char *jcl_set_deck[] = {
        "// SET VAR=PGM=COMP3",
        "//STEP1 EXEC VAR"
    };
    char jcl_set_out[256];
    int set_bytes = tsfi_cw_run_jcl_set(jcl_set_deck, 2, jcl_set_out, sizeof(jcl_set_out));
    assert(set_bytes > 0);
    assert(strstr(jcl_set_out, "EXEC PGM=COMP3") != NULL);

    // EBCDIC CP 285 translation
    assert(tsfi_cw_ascii_to_ebcdic_cp285(0xA3) == 0x5B);
    assert(tsfi_cw_ebcdic_to_ascii_cp285(0x5B) == 0xA3);

    // Nested COBOL Structures
    rc = tsfi_cw_parse_copybook_line("01 GROUP-RECORD.", &cb);
    assert(rc == 0);
    assert(cb.field_count == 10);
    assert(cb.fields[9].length == 0);

    // Century Leap bounds
    assert(tsfi_cw_y2k_is_century_leap(2000) == 1);
    assert(tsfi_cw_y2k_is_century_leap(1900) == 0);
    assert(tsfi_cw_y2k_is_century_leap(2024) == 1);

    // VSAM Entry-Sequenced Data Sets (ESDS)
    tsfi_cw_vsam_esds esds;
    tsfi_cw_vsam_esds_init(&esds, "test_esds.dat.bin");
    uint32_t rba1 = 0, rba2 = 0;
    uint8_t esds_d1[4] = {0xAA, 0xBB};
    uint8_t esds_d2[6] = {0xCC, 0xDD, 0xEE};
    assert(tsfi_cw_vsam_esds_write(&esds, esds_d1, 2, &rba1) == 0);
    assert(tsfi_cw_vsam_esds_write(&esds, esds_d2, 3, &rba2) == 0);
    assert(rba1 == 0);
    assert(rba2 == 2);
    uint8_t esds_out[10];
    int esds_out_len = 0;
    assert(tsfi_cw_vsam_esds_read(&esds, 2, esds_out, sizeof(esds_out), &esds_out_len) == 0);
    assert(esds_out_len == 3);

    // COBOL INDEXED BY Clause
    rc = tsfi_cw_parse_copybook_line("05 TABLE-A PIC X(1) OCCURS 5 INDEXED BY TABLE-IDX.", &cb);
    assert(rc == 0);
    assert(cb.field_count == 11);
    assert(strcmp(cb.fields[10].indexed_by, "TABLE-IDX") == 0);

    // Dynamic EBCDIC Translation Maps
    uint8_t custom_a2e[256];
    uint8_t custom_e2a[256];
    memset(custom_a2e, 0x40, 256);
    memset(custom_e2a, 0x20, 256);
    custom_a2e['A'] = 0xC1;
    custom_e2a[0xC1] = 'A';
    tsfi_cw_set_custom_translation_tables(custom_a2e, custom_e2a);
    assert(tsfi_cw_ascii_to_ebcdic('A') == 0xC1);
    assert(tsfi_cw_ebcdic_to_ascii(0xC1) == 'A');
    tsfi_cw_set_custom_translation_tables(NULL, NULL);

    // Multi-format Gregorian parsing
    uint32_t myy = 0, mmm = 0, mdd = 0;
    assert(tsfi_cw_parse_multi_format_date("071826", "MMDDYY", &myy, &mmm, &mdd) == 0);
    assert(myy == 26);
    assert(mmm == 7);
    assert(mdd == 18);

    // COBOL Level 66 RENAMES Clause
    rc = tsfi_cw_parse_copybook_line("66 RENAMED-VAR RENAMES COMP-VAR THRU TABLE-A.", &cb);
    assert(rc == 0);
    assert(cb.field_count == 12);
    assert(strcmp(cb.fields[11].renames_start, "COMP-VAR") == 0);
    assert(strcmp(cb.fields[11].renames_end, "TABLE-A") == 0);

    // VSAM Key Compression
    char compressed_k[16];
    assert(tsfi_cw_vsam_compress_key("KEY12345", "KEY12000", compressed_k, sizeof(compressed_k)) == 0);
    assert(strcmp(compressed_k, "5345") == 0);
    char decompressed_k[16];
    assert(tsfi_cw_vsam_decompress_key(compressed_k, "KEY12000", decompressed_k, sizeof(decompressed_k)) == 0);
    assert(strcmp(decompressed_k, "KEY12345") == 0);

    // JCL Temporary Datasets (SYSUT1)
    tsfi_cw_jcl_temp_pool t_pool;
    tsfi_cw_jcl_temp_pool_init(&t_pool);
    assert(tsfi_cw_jcl_temp_pool_add(&t_pool, "SYSUT1", "temp1.dat.bin") == 0);
    assert(strcmp(tsfi_cw_jcl_temp_pool_get(&t_pool, "SYSUT1"), "temp1.dat.bin") == 0);

    // Packed decimal decimal-alignment
    char aligned_val[32];
    assert(tsfi_cw_align_comp3_fractional("123.4", 3, aligned_val, sizeof(aligned_val)) == 0);
    assert(strcmp(aligned_val, "123.400") == 0);

    // Julian Year Century Standardizer
    char std_julian[16];
    assert(tsfi_cw_julian_standardize("26.199", 50, std_julian, sizeof(std_julian)) == 0);
    assert(strcmp(std_julian, "2026.199") == 0);

    // VSAM Linear Data Sets (LDS)
    tsfi_cw_vsam_lds lds;
    tsfi_cw_vsam_lds_init(&lds, "test_lds.dat.bin");
    uint8_t lds_w_data[4096];
    memset(lds_w_data, 0xBB, 4096);
    assert(tsfi_cw_vsam_lds_write_page(&lds, 2, lds_w_data) == 0);
    uint8_t lds_r_data[4096];
    assert(tsfi_cw_vsam_lds_read_page(&lds, 2, lds_r_data) == 0);
    assert(lds_r_data[0] == 0xBB);

    // COBOL BLANK WHEN ZERO Clause
    rc = tsfi_cw_parse_copybook_line("05 ZERO-VAR PIC 9(4) BLANK WHEN ZERO.", &cb);
    assert(rc == 0);
    assert(cb.field_count == 13);
    assert(cb.fields[12].blank_when_zero == 1);

    // EBCDIC DBCS CP930
    uint8_t dbcs_str[6] = {0x0E, 0xC1, 0xC2, 0x0F, 0x40};
    int dbcs_count = 0;
    assert(tsfi_cw_ebcdic_is_dbcs(dbcs_str, 5, &dbcs_count) == 0);
    assert(dbcs_count == 2);

    // JCL Nested Procedure loops
    const char *jcl_nested_deck[] = {
        "//TESTJOB JOB (ACCT)",
        "//MYSTEP EXEC MYPROC"
    };
    const char *proc_nested_deck[] = {
        "//SUBSTEP EXEC MYPROC"
    };
    int nested_rc = tsfi_cw_run_jcl_proc_nested(jcl_nested_deck, 2, proc_nested_deck, 1, 0, 1);
    assert(nested_rc == -9);

    // Century boundary (2100) Leap year
    int leap_2100 = 0;
    assert(tsfi_cw_y2k_check_century_leap_2100(2100, &leap_2100) == 0);
    assert(leap_2100 == 0);
    assert(tsfi_cw_y2k_check_century_leap_2100(2000, &leap_2100) == 0);
    assert(leap_2100 == 1);

    // COBOL JUSTIFIED RIGHT Clause
    rc = tsfi_cw_parse_copybook_line("05 STR-VAR PIC X(10) JUSTIFIED RIGHT.", &cb);
    assert(rc == 0);
    assert(cb.field_count == 14);
    assert(cb.fields[13].justified_right == 1);

    // VSAM Key-Length checks
    tsfi_cw_vsam_ksds ksds_lim;
    tsfi_cw_vsam_open(&ksds_lim, "test_ksds_lim.dat.bin");
    uint8_t dummy_d[4] = {0xAA};
    assert(tsfi_cw_vsam_write(&ksds_lim, "VERYLONGKEYEXCEEDINGFIFTEENCHARS", dummy_d, 1) == -6);
    int dummy_len = 0;
    assert(tsfi_cw_vsam_read(&ksds_lim, "VERYLONGKEYEXCEEDINGFIFTEENCHARS", dummy_d, sizeof(dummy_d), &dummy_len) == -6);

    // COBOL SIGN SEPARATE (Display Sign) formats
    char sign_sep_out[16];
    assert(tsfi_cw_pack_sign_separate("-123", sign_sep_out, sizeof(sign_sep_out), 1) == 0);
    assert(strcmp(sign_sep_out, "-123") == 0);
    assert(tsfi_cw_pack_sign_separate("-123", sign_sep_out, sizeof(sign_sep_out), 0) == 0);
    assert(strcmp(sign_sep_out, "123-") == 0);
    char sign_sep_dec[16];
    assert(tsfi_cw_unpack_sign_separate("123-", sign_sep_dec, sizeof(sign_sep_dec), 0) == 0);
    assert(strcmp(sign_sep_dec, "-123") == 0);

    // JCL DD Statement Concatenations
    const char *jcl_concat_deck[] = {
        "//STEP1 EXEC PGM=IEBGENER",
        "//SYSUT1 DD DSN=FILE1.DAT",
        "//       DD DSN=FILE2.DAT",
        "//       DD DSN=FILE3.DAT"
    };
    char concat_res[128];
    assert(tsfi_cw_run_jcl_concat(jcl_concat_deck, 4, concat_res, sizeof(concat_res)) > 0);
    assert(strcmp(concat_res, "FILE1.DAT,FILE2.DAT,FILE3.DAT") == 0);

    // Century Epoch Offset Mapping
    assert(tsfi_cw_y2k_resolve_epoch_base(26, 1950) == 2026);
    assert(tsfi_cw_y2k_resolve_epoch_base(99, 1950) == 1999);

    // VSAM Partitioned Data Sets (PDS)
    tsfi_cw_vsam_pds pds;
    tsfi_cw_vsam_pds_init(&pds, "test_pds.dat.bin");
    assert(tsfi_cw_vsam_pds_add_member(&pds, "MEM1", 100, 50) == 0);
    uint32_t p_offset = 0, p_len = 0;
    assert(tsfi_cw_vsam_pds_find_member(&pds, "MEM1", &p_offset, &p_len) == 0);
    assert(p_offset == 100);
    assert(p_len == 50);

    // COBOL SYNCHRONIZED (SYNC) Clause
    rc = tsfi_cw_parse_copybook_line("05 HALF-WORD PIC S9(4) COMP-5 SYNC.", &cb);
    assert(rc == 0);
    assert(cb.field_count == 15);
    assert(cb.fields[14].sync_align == 4);

    // EBCDIC Custom Padding Character Converter
    uint8_t pad_out[8];
    assert(tsfi_cw_ascii_to_ebcdic_pad("ABC", pad_out, 5, 0x40) == 0);
    assert(pad_out[0] == tsfi_cw_ascii_to_ebcdic('A'));
    assert(pad_out[4] == 0x40);

    // JCL COND Step Evaluation
    assert(tsfi_cw_jcl_eval_cond(12, 8, "LT") == 1);
    assert(tsfi_cw_jcl_eval_cond(4, 8, "LT") == 0);

    // Julian to Gregorian Y2K date conversion
    char greg_out_d[16];
    assert(tsfi_cw_julian_to_gregorian_y2k("26.200", 50, greg_out_d, sizeof(greg_out_d)) == 0);
    assert(strcmp(greg_out_d, "2026-07-19") == 0);

    // COBOL SIGN Embedded Zone Nibbles
    uint8_t zoned_out[16];
    assert(tsfi_cw_pack_zoned_sign("-123", zoned_out, sizeof(zoned_out), 0) == 3);
    char zoned_dec[16];
    assert(tsfi_cw_unpack_zoned_sign(zoned_out, 3, zoned_dec, sizeof(zoned_dec), 0) == 0);
    assert(strcmp(zoned_dec, "-123") == 0);

    // VSAM RRDS Slot Occupancy
    tsfi_cw_vsam_rrds rrds_occ;
    tsfi_cw_vsam_rrds_init(&rrds_occ, "test_occ.dat.bin");
    assert(tsfi_cw_vsam_rrds_is_occupied(&rrds_occ, 10) == 0);
    uint8_t occ_data[4] = {0xAA};
    assert(tsfi_cw_vsam_rrds_write(&rrds_occ, 10, occ_data, 1) == 0);
    assert(tsfi_cw_vsam_rrds_is_occupied(&rrds_occ, 10) == 1);

    // EBCDIC CP273 Translation
    assert(tsfi_cw_ascii_to_ebcdic_cp273(0xA7) == 0x7C);
    assert(tsfi_cw_ebcdic_to_ascii_cp273(0x7C) == 0xA7);

    // JCL EXPORT Variable Cards
    const char *jcl_exp_deck[] = {
        "// EXPORT EXP1=MYVALUE",
        "//STEP1 EXEC PGM=PROG"
    };
    char exp_n[32], exp_v[32];
    assert(tsfi_cw_run_jcl_export(jcl_exp_deck, 2, exp_n, exp_v) == 0);
    assert(strcmp(exp_n, "EXP1") == 0);
    assert(strcmp(exp_v, "MYVALUE") == 0);

    // Dynamic Gregorian Day-of-Month bounds
    assert(tsfi_cw_y2k_check_date_bounds(24, 2, 29, 50) == 0);
    assert(tsfi_cw_y2k_check_date_bounds(25, 2, 29, 50) == -2);

    // VSAM Alternate Index Paths
    tsfi_cw_vsam_ksds base_ksds;
    tsfi_cw_vsam_open(&base_ksds, "test_base.dat.bin");
    tsfi_cw_vsam_aix aix_path;
    tsfi_cw_vsam_aix_init(&aix_path);
    uint8_t base_d[4] = {0x11, 0x22};
    assert(tsfi_cw_vsam_write(&base_ksds, "BKEY1", base_d, 2) == 0);
    assert(tsfi_cw_vsam_aix_add(&aix_path, "AKEY1", "BKEY1") == 0);
    uint8_t path_out_d[4];
    int path_out_len = 0;
    assert(tsfi_cw_vsam_path_read(&base_ksds, &aix_path, "AKEY1", path_out_d, sizeof(path_out_d), &path_out_len) == 0);
    assert(path_out_len == 2);

    // COBOL SIGN LEADING/TRAILING SEPARATE Clause
    rc = tsfi_cw_parse_copybook_line("05 SIGN-VAR PIC S9(5) SIGN LEADING SEPARATE.", &cb);
    assert(rc == 0);
    assert(cb.field_count == 16);
    assert(cb.fields[15].sign_leading == 1);
    assert(cb.fields[15].sign_separate == 1);

    // EBCDIC CP500 Translation
    assert(tsfi_cw_ascii_to_ebcdic_cp500('[') == 0x4A);
    assert(tsfi_cw_ebcdic_to_ascii_cp500(0x4A) == '[');

    // JCL SYSOUT print capture
    tsfi_cw_jcl_sysout sout;
    tsfi_cw_jcl_sysout_init(&sout);
    assert(tsfi_cw_jcl_sysout_write(&sout, "LOG LINE\n") == 0);
    assert(strcmp(sout.buffer, "LOG LINE\n") == 0);

    // Y2K Date difference
    int diff_days = 0;
    assert(tsfi_cw_y2k_date_diff(26, 7, 19, 26, 7, 21, 50, &diff_days) == 0);
    assert(diff_days == 2);

    // VSAM Record Level Sharing (RLS) locks
    assert(tsfi_cw_vsam_lock_record(&base_ksds, "BKEY1") == 0);
    assert(tsfi_cw_vsam_write(&base_ksds, "BKEY1", base_d, 2) == -7);
    assert(tsfi_cw_vsam_unlock_record(&base_ksds, "BKEY1") == 0);
    assert(tsfi_cw_vsam_write(&base_ksds, "BKEY1", base_d, 2) == 0);

    // EBCDIC CP850 Translation
    assert(tsfi_cw_ascii_to_ebcdic_cp850('{') == 0xC0);
    assert(tsfi_cw_ebcdic_to_ascii_cp850(0xC0) == '{');

    // JCL PROC step overrides
    const char *jcl_override_deck[] = {
        "//STEP1.SYSIN DD DSN=SYS1.LINKLIB",
        "//STEP2 EXEC PGM=PROG"
    };
    char override_card[128];
    assert(tsfi_cw_run_jcl_override(jcl_override_deck, 2, "STEP1", override_card, sizeof(override_card)) == 0);
    assert(strstr(override_card, "DSN=SYS1.LINKLIB") != NULL);

    // Y2K Day-of-Week Calculator
    int dow = -1;
    assert(tsfi_cw_y2k_day_of_week(26, 7, 19, 50, &dow) == 0);
    assert(dow == 0);

    // VSAM Control Interval Split
    tsfi_cw_vsam_ksds split_ksds;
    memset(&split_ksds, 0, sizeof(split_ksds));
    strcpy(split_ksds.filepath, "split_test.dat.bin");
    assert(tsfi_cw_vsam_write(&split_ksds, "K1", base_d, 2) == 0);
    assert(tsfi_cw_vsam_write(&split_ksds, "K2", base_d, 2) == 0);
    assert(tsfi_cw_vsam_write(&split_ksds, "K3", base_d, 2) == 0);
    assert(tsfi_cw_vsam_get_ci_splits(&split_ksds) == 1);

    // COBOL OCCURS DEPENDING ON clause
    tsfi_cw_copybook cb_dep;
    memset(&cb_dep, 0, sizeof(cb_dep));
    rc = tsfi_cw_parse_copybook_line("05 DEPEND-VAR PIC 9(2) OCCURS 5 DEPENDING ON DEP-FLD.", &cb_dep);
    assert(rc == 0);
    assert(cb_dep.field_count == 1);
    assert(strcmp(cb_dep.fields[0].depending_on, "DEP-FLD") == 0);
    int dynamic_len = tsfi_cw_cobol_get_dynamic_record_length(&cb_dep, "DEP-FLD", 3);
    assert(dynamic_len == 6);

    // EBCDIC DBCS Shift-In/Shift-Out checks
    uint8_t dbcs_valid[] = {0x0E, 0x41, 0x42, 0x0F};
    uint8_t dbcs_invalid[] = {0x0E, 0x41, 0x42};
    assert(tsfi_cw_ebcdic_validate_dbcs_boundaries(dbcs_valid, sizeof(dbcs_valid)) == 0);
    assert(tsfi_cw_ebcdic_validate_dbcs_boundaries(dbcs_invalid, sizeof(dbcs_invalid)) == -4);

    // JCL GDG resolver
    char gdg_res[256];
    assert(tsfi_cw_jcl_resolve_gdg("MY.DATA.GDG(+1)", 4, gdg_res, sizeof(gdg_res)) == 0);
    assert(strcmp(gdg_res, "MY.DATA.GDG.G0005V00") == 0);

    // Y2K Leap century overrides
    assert(tsfi_cw_y2k_is_leap_year(2000) == 1);
    assert(tsfi_cw_y2k_is_leap_year(2100) == 0);

    // VSAM Record Compression Stats
    float comp_ratio = 0.0f;
    assert(tsfi_cw_vsam_get_compression_ratio(&split_ksds, &comp_ratio) == 0);
    assert(comp_ratio > 0.0f);

    // COBOL REDEFINES Bounds Validator
    tsfi_cw_copybook cb_red;
    memset(&cb_red, 0, sizeof(cb_red));
    assert(tsfi_cw_parse_copybook_line("05 F1 PIC X(4).", &cb_red) == 0);
    assert(tsfi_cw_parse_copybook_line("05 F2 REDEFINES F1 PIC X(5).", &cb_red) == -8);

    // EBCDIC CP285 Character Map
    assert(tsfi_cw_ascii_to_ebcdic_cp285(0x9C) == 0x5B);
    assert(tsfi_cw_ebcdic_to_ascii_cp285(0x5B) == 0xA3);

    // JCL COND Multi-step Chain
    assert(tsfi_cw_jcl_eval_cond_chain(12, 8, "LT", 16, "GT") == 1);

    // Y2K Month Bounds calculator
    int m_days = 0;
    assert(tsfi_cw_y2k_get_month_days(2000, 2, &m_days) == 0);
    assert(m_days == 29);
    assert(tsfi_cw_y2k_get_month_days(2100, 2, &m_days) == 0);
    assert(m_days == 28);

    // VSAM Key Compression Stats
    float key_ratio = 0.0f;
    assert(tsfi_cw_vsam_get_key_compression_ratio(&split_ksds, &key_ratio) == 0);
    assert(key_ratio > 0.0f);

    // COBOL REDEFINES Nested checks
    assert(tsfi_cw_parse_copybook_line("05 F2 REDEFINES F1 PIC X(3).", &cb_red) == 0);
    assert(tsfi_cw_parse_copybook_line("05 F3 REDEFINES F2 PIC X(2).", &cb_red) == 0);

    // EBCDIC CP273 Sterling Map Expansion
    assert(tsfi_cw_ascii_to_ebcdic_cp273_ex(0x9C) == 0x5B);
    assert(tsfi_cw_ebcdic_to_ascii_cp273_ex(0x5B) == 0xA3);

    // JCL Inline SYSIN DD * delimiter overrides
    const char *jcl_dlm_deck[] = {
        "//STEP1 EXEC PGM=IEFBR14",
        "//SYSIN DD *,DLM=$$",
        "DATA LINE 1",
        "$$",
        "//STEP2 EXEC PGM=IEFBR14"
    };
    char sysin_dlm_out[256];
    assert(tsfi_cw_run_jcl_sysin(jcl_dlm_deck, 5, sysin_dlm_out, sizeof(sysin_dlm_out)) == 12);
    assert(strcmp(sysin_dlm_out, "DATA LINE 1\n") == 0);

    // Y2K Gregorian-to-Julian Date Converter
    char jul_out[32];
    assert(tsfi_cw_gregorian_to_julian_y2k("2000-02-29", 50, jul_out, sizeof(jul_out)) == 0);
    assert(strcmp(jul_out, "00.060") == 0);

    // VSAM Key cache lookups
    uint8_t cache_d[2];
    int cache_out_len = 0;
    assert(tsfi_cw_vsam_read(&split_ksds, "K1", cache_d, 2, &cache_out_len) == 0);
    assert(tsfi_cw_vsam_read(&split_ksds, "K1", cache_d, 2, &cache_out_len) == 0);
    assert(tsfi_cw_vsam_get_cache_hits(&split_ksds) >= 1);

    // COBOL REDEFINES OCCURS Bounds Validator
    tsfi_cw_copybook cb_occ;
    memset(&cb_occ, 0, sizeof(cb_occ));
    assert(tsfi_cw_parse_copybook_line("05 F1 PIC X(4).", &cb_occ) == 0);
    assert(tsfi_cw_parse_copybook_line("05 F2 REDEFINES F1 PIC X(2) OCCURS 3.", &cb_occ) == -8);

    // EBCDIC CP278 Swedish translation
    assert(tsfi_cw_ascii_to_ebcdic_cp278(0xC5) == 0x5A);
    assert(tsfi_cw_ebcdic_to_ascii_cp278(0x5A) == 0xC5);

    // JCL SYSIN 80-Column Truncation warning
    const char *jcl_trunc_deck[] = {
        "//STEP1 EXEC PGM=IEFBR14",
        "//SYSIN DD *",
        "12345678901234567890123456789012345678901234567890123456789012345678901234567890EXTRA LONG PUNCH CARD LINE THAT WILL BE TRUNCATED",
        "/*"
    };
    char sysin_trunc_out[256];
    int truncated_count = 0;
    assert(tsfi_cw_run_jcl_sysin_ex(jcl_trunc_deck, 4, sysin_trunc_out, sizeof(sysin_trunc_out), &truncated_count) == 81);
    assert(truncated_count == 1);

    // Y2K Leap diagnostics performance
    tsfi_cw_y2k_diagnostics y2k_diag;
    tsfi_cw_y2k_get_diagnostics(&y2k_diag);
    assert(y2k_diag.leap_checks_performed > 0);

    // VSAM Lock Record extended
    assert(tsfi_cw_vsam_lock_record_ex(&split_ksds, "K1", 2) == 0);
    assert(tsfi_cw_vsam_lock_record_ex(&split_ksds, "K1", 2) == -7);
    assert(tsfi_cw_vsam_lock_record_ex(&split_ksds, "K1", 2) == -7);
    assert(tsfi_cw_vsam_lock_record_ex(&split_ksds, "K1", 2) == -10);

    // COBOL Alphanumeric justify right bounds
    assert(tsfi_cw_parse_copybook_line("05 F1 PIC X(3) JUSTIFIED RIGHT VALUE 'ABCD'.", &cb_occ) == -11);

    // JCL EXEC step parameter
    char parm_val[64];
    assert(tsfi_cw_jcl_parse_parm("//STEP1 EXEC PGM=PROG,PARM='HELLO'", parm_val, sizeof(parm_val)) == 0);
    assert(strcmp(parm_val, "HELLO") == 0);

    // Y2K format validator
    assert(tsfi_cw_y2k_validate_format("2026-07-18") == 0);
    assert(tsfi_cw_y2k_validate_format("2026/07/18") == -3);

    // VSAM Index Split Recovery
    tsfi_cw_vsam_ksds recovery_ksds;
    memset(&recovery_ksds, 0, sizeof(recovery_ksds));
    strcpy(recovery_ksds.filepath, "recovery_test.dat.bin");
    for (int i = 0; i < 128; i++) {
        recovery_ksds.index[i].active = 0;
    }
    recovery_ksds.entry_count = 128;
    assert(tsfi_cw_vsam_write(&recovery_ksds, "REC1", base_d, 2) == 0);
    assert(recovery_ksds.entry_count == 1);

    // COBOL Occurs dynamic check
    assert(tsfi_cw_cobol_validate_occurs_range(5, 10) == 0);
    assert(tsfi_cw_cobol_validate_occurs_range(12, 10) == -12);

    // JCL parameter substitution
    char subst_out[128];
    assert(tsfi_cw_jcl_substitute_symbol("//SYSIN DD DSN=&MYDSN", "MYDSN", "TEST.DATA", subst_out, sizeof(subst_out)) == 0);
    assert(strcmp(subst_out, "//SYSIN DD DSN=TEST.DATA") == 0);

    // Y2K Julian validator
    assert(tsfi_cw_y2k_validate_julian_day(2000, 366) == 0);
    assert(tsfi_cw_y2k_validate_julian_day(2100, 366) == -13);

    // EBCDIC CP935 Chinese Translation
    uint8_t cp935_buf[64];
    char utf8_buf[64];
    int cp935_len = tsfi_cw_utf8_to_ebcdic_cp935("中文", cp935_buf, sizeof(cp935_buf));
    assert(cp935_len == 8);
    assert(cp935_buf[0] == 0x0E); // Shift-Out
    assert(cp935_buf[1] == 0x4C);
    assert(cp935_buf[2] == 0x60); // 中
    assert(cp935_buf[3] == 0x0F); // Shift-In
    assert(cp935_buf[4] == 0x0E); // Shift-Out
    assert(cp935_buf[5] == 0x4C);
    assert(cp935_buf[6] == 0x64); // 文
    assert(cp935_buf[7] == 0x0F); // Shift-In
    assert(tsfi_cw_ebcdic_to_utf8_cp935(cp935_buf, cp935_len, utf8_buf, sizeof(utf8_buf)) == 6);
    assert(strcmp(utf8_buf, "中文") == 0);

    // VSAM Key Prefix Savings
    assert(tsfi_cw_vsam_get_key_prefix_savings(&split_ksds) > 0);

    // COBOL justified right dynamic check
    tsfi_cw_cobol_field f_dyn;
    memset(&f_dyn, 0, sizeof(f_dyn));
    f_dyn.justified_right = 1;
    f_dyn.length = 6;
    f_dyn.occurs = 2;
    assert(tsfi_cw_cobol_validate_justified_right_dynamic(&f_dyn, 1, "ABC") == 0);
    assert(tsfi_cw_cobol_validate_justified_right_dynamic(&f_dyn, 1, "ABCD") == -14);

    // EBCDIC CP937 Chinese Translation
    uint8_t cp937_buf[64];
    char utf8_cp937[64];
    int cp937_len = tsfi_cw_utf8_to_ebcdic_cp937("繁體", cp937_buf, sizeof(cp937_buf));
    assert(cp937_len == 8);
    assert(tsfi_cw_ebcdic_to_utf8_cp937(cp937_buf, cp937_len, utf8_cp937, sizeof(utf8_cp937)) == 6);
    assert(strcmp(utf8_cp937, "繁體") == 0);

    // JCL symbol validation rules
    assert(tsfi_cw_jcl_validate_symbol_name("VALID1") == 0);
    assert(tsfi_cw_jcl_validate_symbol_name("1INVALID") == -15);
    assert(tsfi_cw_jcl_substitute_symbol("//SYSIN DD DSN=&1INVALID", "1INVALID", "VAL", subst_out, sizeof(subst_out)) == -15);

    // Y2K Date difference century span check
    int diff_span = 0;
    assert(tsfi_cw_y2k_date_diff(0, 1, 1, 99, 1, 1, 50, &diff_span) == 0);
    assert(tsfi_cw_y2k_date_diff(0, 1, 1, 201, 1, 1, 50, &diff_span) == -16);

    // COBOL Picture Numeric bounds validator
    assert(tsfi_cw_cobol_validate_picture_numeric_bounds("9(3)", 999) == 0);
    assert(tsfi_cw_cobol_validate_picture_numeric_bounds("9(3)", 1000) == -17);

    // EBCDIC CP939 Chinese Translation
    uint8_t cp939_buf[64];
    char utf8_cp939[64];
    int cp939_len = tsfi_cw_utf8_to_ebcdic_cp939("中文~", cp939_buf, sizeof(cp939_buf));
    assert(cp939_len == 9);
    assert(cp939_buf[8] == 0xBA); // ~ in CP939
    assert(tsfi_cw_ebcdic_to_utf8_cp939(cp939_buf, cp939_len, utf8_cp939, sizeof(utf8_cp939)) == 7);
    assert(strcmp(utf8_cp939, "中文~") == 0);

    // JCL EXEC parameter unbalanced quotes check
    assert(tsfi_cw_jcl_check_parm_quotes("//STEP EXEC PARM='VAL'") == 0);
    assert(tsfi_cw_jcl_check_parm_quotes("//STEP EXEC PARM='VAL") == -18);
    char dummy_parm[32];
    assert(tsfi_cw_jcl_parse_parm("//STEP EXEC PARM='VAL", dummy_parm, sizeof(dummy_parm)) == -18);

    // Y2K leap adjustments count check
    assert(tsfi_cw_y2k_count_leap_adjustments(2000, 2004) == 2);

    // EBCDIC Zhumadian Cantonese Translation
    uint8_t zhumadian_buf[64];
    char utf8_zhumadian[64];
    int zhumadian_len = tsfi_cw_utf8_to_ebcdic_zhumadian_cantonese("駐馬店粵", zhumadian_buf, sizeof(zhumadian_buf));
    assert(zhumadian_len == 16);
    assert(tsfi_cw_ebcdic_to_utf8_zhumadian_cantonese(zhumadian_buf, zhumadian_len, utf8_zhumadian, sizeof(utf8_zhumadian)) == 12);
    assert(strcmp(utf8_zhumadian, "駐馬店粵") == 0);

    // EBCDIC CP935 Extended Single-byte translator
    assert(tsfi_cw_ebcdic_cp935_extended_translate(0xAD) == '[');

    // VSAM key length validator
    assert(tsfi_cw_vsam_validate_compressed_key_len("HELLOWORLD", "HW") == 0);
    assert(tsfi_cw_vsam_validate_compressed_key_len("HELLOWORLD", "LONGERHELLOWORLD") == -9);

    // JCL card limit check
    assert(tsfi_cw_jcl_sysin_limit_check(450) == 0);
    assert(tsfi_cw_jcl_sysin_limit_check(550) == -20);

    // COBOL record offset limits
    assert(tsfi_cw_cobol_validate_record_offset(100, 200) == 0);
    assert(tsfi_cw_cobol_validate_record_offset(250, 200) == -19);

    // Y2K Pivot Validation
    assert(tsfi_cw_y2k_validate_pivot_range(50) == 0);
    assert(tsfi_cw_y2k_validate_pivot_range(5) == -21);

    // VSAM key prefix decompression integrity validator
    char decomp_out_fail[32];
    assert(tsfi_cw_vsam_decompress_key("4KEY", "K1", decomp_out_fail, sizeof(decomp_out_fail)) == -22);

    // COBOL offset overlap
    assert(tsfi_cw_cobol_validate_offset_overlap(10, 8) == 0);
    assert(tsfi_cw_cobol_validate_offset_overlap(10, 12) == -23);

    // EBCDIC Escape translation
    assert(tsfi_cw_ebcdic_translate_control_escape(0x25) == '\n');

    // JCL multi symbol substitutions
    const char *names[] = {"S1", "S2"};
    const char *vals[] = {"VAL1", "VAL2"};
    char multi_out[128];
    assert(tsfi_cw_jcl_substitute_symbols_multi("//DD DSN=&S1.&S2", names, vals, 2, multi_out, sizeof(multi_out)) == 0);
    assert(strcmp(multi_out, "//DD DSN=VAL1.VAL2") == 0);

    // Y2K leap span chronological check
    assert(tsfi_cw_y2k_count_leap_adjustments(2004, 2000) == -24);

    // VSAM block checksum audit check
    uint8_t chk_data[4] = {0x01, 0x02, 0x03, 0x04};
    uint32_t chk = tsfi_cw_vsam_calculate_checksum(chk_data, 4);
    assert(chk == 10);
    assert(tsfi_cw_vsam_verify_record_checksum(chk_data, 4, 10) == 0);
    assert(tsfi_cw_vsam_verify_record_checksum(chk_data, 4, 9) == -25);

    // COBOL custom padding validator
    assert(tsfi_cw_cobol_validate_custom_padding('_') == 0);
    assert(tsfi_cw_cobol_validate_custom_padding('\n') == -26);

    // EBCDIC nesting validator
    uint8_t nested_dbcs[4] = {0x0E, 0x0E, 0x0F, 0x0F};
    assert(tsfi_cw_ebcdic_check_dbcs_nesting(nested_dbcs, 4) == -27);

    // JCL circular dependency checker
    const char *circ_names[] = {"A", "B"};
    const char *circ_vals[] = {"&B", "&A"};
    assert(tsfi_cw_jcl_detect_circular_symbols(circ_names, circ_vals, 2) == -29);
    assert(tsfi_cw_jcl_substitute_symbols_multi("//DD DSN=&A", circ_names, circ_vals, 2, multi_out, sizeof(multi_out)) == -29);

    // Y2K leap adjustments count diagnostics check
    int d_dummy = 0;
    tsfi_cw_y2k_diagnostics diag;
    tsfi_cw_y2k_date_diff(0, 1, 1, 10, 1, 1, 50, &d_dummy);
    tsfi_cw_y2k_get_diagnostics(&diag);
    assert(diag.span_leap_adjustments_tracked > 0);

    // COBOL custom padding ex bounds check
    assert(tsfi_cw_cobol_validate_custom_padding_ex('_', 5, 10) == 0);
    assert(tsfi_cw_cobol_validate_custom_padding_ex('_', 15, 10) == -30);

    // EBCDIC escape overrides
    assert(tsfi_cw_ebcdic_translate_control_escape_override(0x1F, 0x1F, 0x0D) == '\n');

    // JCL symbol recursion depth validator
    assert(tsfi_cw_jcl_validate_substitution_depth(3, 5) == 0);
    assert(tsfi_cw_jcl_validate_substitution_depth(6, 5) == -31);

    // Y2K chronological dates validation
    assert(tsfi_cw_y2k_validate_chronological_order(0, 1, 1, 10, 1, 1, 50) == 0);
    assert(tsfi_cw_y2k_validate_chronological_order(10, 1, 1, 0, 1, 1, 50) == -32);
    assert(tsfi_cw_y2k_get_chronological_violations() > 0);

    // COBOL padding map config check
    char mapped_pad = '\0';
    assert(tsfi_cw_cobol_map_custom_padding_byte('\0', &mapped_pad) == 0);
    assert(mapped_pad == ' ');

    // EBCDIC DBCS parity error check
    uint8_t nested_dbcs_odd[5] = {0x0E, 0x5D, 0x0F}; // 1 DBCS byte (odd length)
    assert(tsfi_cw_ebcdic_check_dbcs_nesting(nested_dbcs_odd, 3) == -33);

    // EBCDIC DBCS unbalanced shift-out check
    uint8_t nested_dbcs_unbalanced[4] = {0x0E, 0x5D, 0x5D}; // Missing Shift-In
    assert(tsfi_cw_ebcdic_check_dbcs_nesting(nested_dbcs_unbalanced, 3) == -35);

    // VSAM verify checksum cached
    tsfi_cw_vsam_ksds cached_ksds;
    cached_ksds.entry_count = 1;
    cached_ksds.index[0].checksum = 12345;
    uint8_t cached_dummy[4] = {0,0,0,0};
    assert(tsfi_cw_vsam_verify_checksum_cached(&cached_ksds, 0, cached_dummy, 4) == -25);

    // COBOL padding alignment
    assert(tsfi_cw_cobol_validate_padding_alignment(8, 4) == 0);
    assert(tsfi_cw_cobol_validate_padding_alignment(9, 4) == -34);

    // JCL step execution PROC depth recursion checker
    assert(tsfi_cw_jcl_validate_proc_recursion_depth(3, 5) == 0);
    assert(tsfi_cw_jcl_validate_proc_recursion_depth(6, 5) == -36);

    // Y2K Date Span violation diagnostic stats reset
    tsfi_cw_y2k_reset_chronological_violations();
    assert(tsfi_cw_y2k_get_chronological_violations() == 0);

    // VSAM index verification checks
    assert(tsfi_cw_vsam_verify_index_checksums(&cached_ksds) == 0);
    cached_ksds.index[0].checksum = 0xFFFFFFFF;
    cached_ksds.index[0].active = 1;
    assert(tsfi_cw_vsam_verify_index_checksums(&cached_ksds) == -25);

    // COBOL padding limit checks
    assert(tsfi_cw_cobol_validate_padding_limits(5, 10) == 0);
    assert(tsfi_cw_cobol_validate_padding_limits(15, 10) == -30);

    // EBCDIC overrides check
    tsfi_cw_ebcdic_override_dbcs_markers(0x0E, 0x0F);

    // JCL set recursion limit check
    tsfi_cw_jcl_set_custom_proc_recursion_limit(10);

    // Y2K query check
    uint32_t v_count = 999;
    assert(tsfi_cw_y2k_query_chronological_violations(&v_count) == 0);
    assert(v_count == 0);

    // COBOL padding alignment getter check
    int padding_bytes = 0;
    assert(tsfi_cw_cobol_get_alignment_padding(10, 4, &padding_bytes) == 0);
    assert(padding_bytes == 2);

    // EBCDIC parity checks stats
    uint32_t parity_cnt = 0;
    assert(tsfi_cw_ebcdic_get_parity_checks_count(&parity_cnt) == 0);

    // JCL recursion limit query
    int jcl_limit = 0;
    assert(tsfi_cw_jcl_query_recursion_limit(&jcl_limit) == 0);
    assert(jcl_limit == 10);

    // Y2K query and reset check
    assert(tsfi_cw_y2k_query_and_reset_violations(&v_count) == 0);
    assert(v_count == 0);

    // VSAM checksum audit query check
    uint32_t aud_p = 0, aud_m = 0;
    assert(tsfi_cw_vsam_get_checksum_audit_stats(&aud_p, &aud_m) == 0);

    // COBOL field alignment check
    assert(tsfi_cw_cobol_verify_field_alignment(8, 4, 4) == 0);
    assert(tsfi_cw_cobol_verify_field_alignment(9, 4, 4) == -34);

    // EBCDIC reset parity checks
    tsfi_cw_ebcdic_reset_parity_checks_count();
    assert(tsfi_cw_ebcdic_get_parity_checks_count(&parity_cnt) == 0);
    assert(parity_cnt == 0);

    // JCL multi symbol substitution limit check
    assert(tsfi_cw_jcl_substitute_symbols_multi_limit("//DD DSN=&A", circ_names, circ_vals, 2, multi_out, sizeof(multi_out), 5) == -29);

    // Y2K formatting violations check
    char y2k_format_buf[64];
    assert(tsfi_cw_y2k_format_chronological_violations(y2k_format_buf, sizeof(y2k_format_buf)) == 0);
    assert(strcmp(y2k_format_buf, "Chronological violations: 0") == 0);

    // VSAM checksum audit reset check
    tsfi_cw_vsam_reset_checksum_audit_stats();
    assert(tsfi_cw_vsam_get_checksum_audit_stats(&aud_p, &aud_m) == 0);
    assert(aud_p == 0);

    // COBOL padding limit checks ex
    assert(tsfi_cw_cobol_validate_padding_limits_ex(8, 10, 4) == 0);
    assert(tsfi_cw_cobol_validate_padding_limits_ex(9, 10, 4) == -34);

    // EBCDIC query and reset parity checks
    uint32_t ebcdic_parity_cnt = 0;
    assert(tsfi_cw_ebcdic_query_and_reset_parity_checks(&ebcdic_parity_cnt) == 0);
    assert(ebcdic_parity_cnt == 0);

    // JCL substitution limit configuration
    tsfi_cw_jcl_set_substitution_depth_limit(8);
    int jcl_sub_limit = 0;
    assert(tsfi_cw_jcl_get_substitution_depth_limit(&jcl_sub_limit) == 0);
    assert(jcl_sub_limit == 8);

    // Y2K format and reset violations check
    assert(tsfi_cw_y2k_format_and_reset_violations(y2k_format_buf, sizeof(y2k_format_buf)) == 0);
    assert(strcmp(y2k_format_buf, "Chronological violations: 0") == 0);

    // VSAM checksum format stats check
    char vsam_fmt[64];
    assert(tsfi_cw_vsam_format_checksum_stats(vsam_fmt, sizeof(vsam_fmt)) == 0);
    assert(strcmp(vsam_fmt, "VSAM Audits: 0, Mismatches: 0") == 0);

    // COBOL padding alignment config check
    tsfi_cw_cobol_padding_config pad_cfg;
    assert(tsfi_cw_cobol_configure_padding_alignment(&pad_cfg, ' ', 4, 16) == 0);
    assert(pad_cfg.pad_char == ' ');

    // EBCDIC parity diagnostics formatting
    char ebcdic_fmt[64];
    assert(tsfi_cw_ebcdic_format_parity_diagnostics(ebcdic_fmt, sizeof(ebcdic_fmt)) == 0);

    // JCL reset substitution limit check
    tsfi_cw_jcl_reset_substitution_depth_limit();
    assert(tsfi_cw_jcl_get_substitution_depth_limit(&jcl_sub_limit) == 0);
    assert(jcl_sub_limit == 5);

    // Y2K print diagnostic list check
    assert(tsfi_cw_y2k_print_diagnostic_violations_list(y2k_format_buf, sizeof(y2k_format_buf)) == 0);
    assert(strcmp(y2k_format_buf, "[Y2K Stats] Total Chronological Violations Tracked: 0") == 0);

    // VSAM format and reset checksum stats
    assert(tsfi_cw_vsam_format_and_reset_checksum_stats(vsam_fmt, sizeof(vsam_fmt)) == 0);
    assert(strcmp(vsam_fmt, "VSAM Audits: 0, Mismatches: 0") == 0);

    // COBOL padding configurations query
    char query_pad_char = '\0';
    int query_alignment = 0, query_max_size = 0;
    assert(tsfi_cw_cobol_query_padding_alignment(&pad_cfg, &query_pad_char, &query_alignment, &query_max_size) == 0);
    assert(query_pad_char == ' ');
    assert(query_alignment == 4);

    // EBCDIC diagnostics format and reset
    assert(tsfi_cw_ebcdic_format_and_reset_parity_diagnostics(ebcdic_fmt, sizeof(ebcdic_fmt)) == 0);

    // JCL query and reset substitution limit
    assert(tsfi_cw_jcl_query_and_reset_substitution_limit(&jcl_sub_limit) == 0);
    assert(jcl_sub_limit == 5);

    // Y2K print and reset list check
    assert(tsfi_cw_y2k_print_and_reset_violations_list(y2k_format_buf, sizeof(y2k_format_buf)) == 0);
    assert(strcmp(y2k_format_buf, "[Y2K Stats] Total Chronological Violations Tracked: 0") == 0);

    // VSAM record checksum query-and-reset
    assert(tsfi_cw_vsam_query_and_reset_checksum_audit_stats(&aud_p, &aud_m) == 0);
    assert(aud_p == 0);

    // COBOL padding config reset
    assert(tsfi_cw_cobol_reset_padding_alignment(&pad_cfg) == 0);
    assert(pad_cfg.pad_char == '\0');

    // EBCDIC metrics ex query
    tsfi_cw_ebcdic_parity_metrics ebcdic_met;
    assert(tsfi_cw_ebcdic_get_parity_metrics_ex(&ebcdic_met) == 0);
    assert(ebcdic_met.total_parity_checks == 0);

    // JCL set substitution depth limit boundary
    assert(tsfi_cw_jcl_set_substitution_depth_limit_boundary(10, 20) == 0);

    // Y2K violations print ex
    assert(tsfi_cw_y2k_print_diagnostic_violations_list_ex(y2k_format_buf, sizeof(y2k_format_buf), 50) == 0);
    assert(strcmp(y2k_format_buf, "[Y2K Pivot 50] Chronological Violations: 0") == 0);

    // VSAM checksum audit query ex
    char path_buf[128];
    tsfi_cw_vsam_ksds dummy_ksds;
    strcpy(dummy_ksds.filepath, "dummy.dat.bin");
    assert(tsfi_cw_vsam_get_checksum_audit_stats_ex(&dummy_ksds, path_buf, sizeof(path_buf), &aud_p, &aud_m) == 0);
    assert(strcmp(path_buf, "dummy.dat.bin") == 0);

    // COBOL verify field alignment ex
    assert(tsfi_cw_cobol_verify_field_alignment_ex(8, 4, 4, 16) == 0);
    assert(tsfi_cw_cobol_verify_field_alignment_ex(8, 4, 4, 10) == -30);

    // EBCDIC query and reset parity metrics ex
    assert(tsfi_cw_ebcdic_query_and_reset_parity_metrics_ex(&ebcdic_met) == 0);
    assert(ebcdic_met.total_parity_checks == 0);

    // JCL get substitution depth limit boundary
    int sub_lim = 0, sub_bound = 0;
    assert(tsfi_cw_jcl_get_substitution_depth_limit_boundary(&sub_lim, &sub_bound) == 0);
    assert(sub_lim == 10);
    assert(sub_bound == 20);

    // Y2K print and reset list ex check
    assert(tsfi_cw_y2k_print_and_reset_violations_list_ex(y2k_format_buf, sizeof(y2k_format_buf), 50) == 0);
    assert(strcmp(y2k_format_buf, "[Y2K Pivot 50] Chronological Violations: 0") == 0);

    // VSAM checksum reset ex
    assert(tsfi_cw_vsam_format_and_reset_checksum_stats_ex(&dummy_ksds, vsam_fmt, sizeof(vsam_fmt)) == 0);
    assert(strcmp(vsam_fmt, "File: dummy.dat.bin, Audits: 0, Mismatches: 0") == 0);

    // COBOL verify field alignment ex2
    assert(tsfi_cw_cobol_verify_field_alignment_ex2(8, 4, 4, 16, ' ') == 0);
    assert(tsfi_cw_cobol_verify_field_alignment_ex2(8, 4, 4, 16, '\0') == -32);

    // EBCDIC metrics ex format reset
    assert(tsfi_cw_ebcdic_format_and_reset_parity_metrics_ex(&ebcdic_met, ebcdic_fmt, sizeof(ebcdic_fmt)) == 0);

    // JCL get substitution depth limit boundary ex
    int fallback_val = 0;
    assert(tsfi_cw_jcl_get_substitution_depth_limit_boundary_ex(&sub_lim, &sub_bound, &fallback_val) == 0);
    assert(sub_lim == 10);
    assert(sub_bound == 20);
    assert(fallback_val == 5);

    // Y2K format and reset violations ex
    assert(tsfi_cw_y2k_format_and_reset_violations_ex(y2k_format_buf, sizeof(y2k_format_buf), 50) == 0);
    assert(strcmp(y2k_format_buf, "[Y2K Pivot 50] Chronological violations: 0") == 0);

    // VSAM checksum status formatting ex
    assert(tsfi_cw_vsam_format_checksum_status_ex(&dummy_ksds, vsam_fmt, sizeof(vsam_fmt), "OK") == 0);
    assert(strstr(vsam_fmt, "Status: OK") != NULL);

    // COBOL field alignment ex3
    assert(tsfi_cw_cobol_verify_field_alignment_ex3(8, 4, 4, 16, ' ', 2) == 0);
    assert(tsfi_cw_cobol_verify_field_alignment_ex3(8, 4, 4, 16, ' ', 8) == -30);

    // EBCDIC metrics ex2 format reset
    assert(tsfi_cw_ebcdic_format_and_reset_parity_metrics_ex2(&ebcdic_met, ebcdic_fmt, sizeof(ebcdic_fmt)) == 0);

    // JCL substitution stats query
    uint32_t jcl_subs = 99;
    assert(tsfi_cw_jcl_get_substitution_stats(&jcl_subs) == 0);
    assert(jcl_subs == 0);

    // Y2K print and reset list ex2
    assert(tsfi_cw_y2k_print_and_reset_violations_list_ex2(y2k_format_buf, sizeof(y2k_format_buf), 50, "[LOG]") == 0);
    assert(strcmp(y2k_format_buf, "[LOG] [Pivot 50] Chronological Violations: 0") == 0);

    // VSAM checksum status formatting ex reset
    assert(tsfi_cw_vsam_format_and_reset_checksum_status_ex(&dummy_ksds, vsam_fmt, sizeof(vsam_fmt), "OK") == 0);
    assert(strstr(vsam_fmt, "Status: OK") != NULL);

    // COBOL field alignment ex4
    assert(tsfi_cw_cobol_verify_field_alignment_ex4(8, 4, 4, 16, ' ', 2, 0) == 0);
    assert(tsfi_cw_cobol_verify_field_alignment_ex4(8, 4, 4, 16, '\0', 2, 1) == 0);

    // EBCDIC metrics ex3 format reset
    assert(tsfi_cw_ebcdic_format_and_reset_parity_metrics_ex3(&ebcdic_met, ebcdic_fmt, sizeof(ebcdic_fmt), "PRE:") == 0);

    // JCL substitution stats query reset
    assert(tsfi_cw_jcl_query_and_reset_substitution_stats(&jcl_subs) == 0);
    assert(jcl_subs == 0);

    // Y2K list query length ex2
    int viol_len = 0;
    assert(tsfi_cw_y2k_format_diagnostic_violations_list_ex2(y2k_format_buf, sizeof(y2k_format_buf), 50, "[LOG]", &viol_len) == 0);
    assert(viol_len > 0);

    // VSAM checksum status formatting ex2 reset
    int vsam_rc = 0;
    assert(tsfi_cw_vsam_format_and_reset_checksum_status_ex2(&dummy_ksds, vsam_fmt, sizeof(vsam_fmt), "OK", &vsam_rc) == 0);
    assert(vsam_rc > 0);

    // COBOL field alignment ex5
    assert(tsfi_cw_cobol_verify_field_alignment_ex5(8, 4, 4, 16, ' ', 2, 0, 10) == 0);
    assert(tsfi_cw_cobol_verify_field_alignment_ex5(12, 4, 4, 16, ' ', 2, 0, 10) == -28);

    // EBCDIC metrics ex4 format reset
    assert(tsfi_cw_ebcdic_format_and_reset_parity_metrics_ex4(&ebcdic_met, ebcdic_fmt, sizeof(ebcdic_fmt), "PRE:", &viol_len) == 0);
    assert(viol_len > 0);

    // JCL substitution stats query reset ex
    assert(tsfi_cw_jcl_query_and_reset_substitution_stats_ex(&jcl_subs, &sub_lim) == 0);
    assert(jcl_subs == 0);
    assert(sub_lim == 10);

    // Y2K list format-reset ex2 length
    assert(tsfi_cw_y2k_format_and_reset_violations_ex2(y2k_format_buf, sizeof(y2k_format_buf), 50, "[LOG]", &viol_len) == 0);
    assert(viol_len > 0);

    // VSAM checksum status formatting ex3 reset
    int vsam_len = 0;
    assert(tsfi_cw_vsam_format_and_reset_checksum_status_ex3(&dummy_ksds, vsam_fmt, sizeof(vsam_fmt), "OK", &vsam_rc, &vsam_len) == 0);
    assert(vsam_len > 0);

    // COBOL field alignment ex6
    int failure_offset = 0;
    assert(tsfi_cw_cobol_verify_field_alignment_ex6(8, 4, 4, 16, ' ', 2, 0, 10, &failure_offset) == 0);

    // EBCDIC metrics ex5 format reset
    int eb_status = 0;
    assert(tsfi_cw_ebcdic_format_and_reset_parity_metrics_ex5(&ebcdic_met, ebcdic_fmt, sizeof(ebcdic_fmt), "PRE:", &viol_len, &eb_status) == 0);
    assert(viol_len > 0);

    // JCL substitution stats query reset ex2
    sub_bound = 0;
    assert(tsfi_cw_jcl_query_and_reset_substitution_stats_ex2(&jcl_subs, &sub_lim, &sub_bound) == 0);
    assert(jcl_subs == 0);

    // Y2K list format-reset ex3 length
    uint32_t Y2k_viols = 0;
    assert(tsfi_cw_y2k_format_and_reset_violations_ex3(y2k_format_buf, sizeof(y2k_format_buf), 50, "[LOG]", &viol_len, &Y2k_viols) == 0);
    assert(viol_len > 0);

    // DoD 1967 COBOL Compliance Validation Suite test
    int dod_viols = 0;
    const char *compliant_src = "IDENTIFICATION DIVISION.\nENVIRONMENT DIVISION.\nDATA DIVISION.\nPROCEDURE DIVISION.\n";
    assert(tsfi_cw_cobol_validate_dod1967_compliance(compliant_src, &dod_viols) == 0);
    assert(dod_viols == 0);

    const char *non_compliant_src = "IDENTIFICATION DIVISION.\nDATA DIVISION.\n88 CONDITION-VAL PIC X.\n";
    assert(tsfi_cw_cobol_validate_dod1967_compliance(non_compliant_src, &dod_viols) == 0);
    assert(dod_viols > 0); // Missing divisions + Level 88 violation

    // OS/360 JCL Job Simulator test
    const char *test_jcl = "//MYJOB JOB 'ACCOUNTING'\n//STEP1 EXEC PGM=IEFBR14\n//DD1 DD DSN=SYS1.LINKLIB,DISP=SHR\n";
    char jcl_log[512];
    int job_status = 0;
    assert(tsfi_cw_jcl_simulate_s360_job(test_jcl, jcl_log, sizeof(jcl_log), &job_status) == 0);
    assert(job_status == 0);
    assert(strstr(jcl_log, "Job completed") != NULL);

    // Burroughs B5500 ALGOL Stack Simulator test
    tsfi_b5500_processor b5500_cpu;
    memset(&b5500_cpu, 0, sizeof(b5500_cpu));
    const char *b5500_ops[] = {
        "PUSH 10.5",
        "PUSH 4.5",
        "ADD",
        "PUSH 2.0",
        "MUL"
    };
    assert(tsfi_algol_execute_b5500(&b5500_cpu, b5500_ops, 5) == 0);
    assert(b5500_cpu.sp == 1);
    assert(b5500_cpu.operand_stack[0] == 30.0);

    // ALGOL DAT operation on 2-stack BTC Rails test
    tsfi_algol_dynamic_array test_dat = { 100, 10, 5 }; // key_start=100, size=5
    double btc_stack[32];
    double btc_altstack[32];
    int btc_sp = 0;
    int btc_asp = 0;
    assert(tsfi_algol_operate_btc_rails_dat(&b5500_cpu, &test_dat, btc_stack, &btc_sp, btc_altstack, &btc_asp, "LOAD_DAT_TO_BTC") == 0);
    assert(btc_sp == 5);
    assert(btc_stack[0] == 100.0);
    assert(btc_stack[4] == 104.0);

    assert(tsfi_algol_operate_btc_rails_dat(&b5500_cpu, &test_dat, btc_stack, &btc_sp, btc_altstack, &btc_asp, "STORE_BTC_TO_DAT") == 0);
    assert(btc_sp == 0);
    assert(test_dat.key_start == 100);

    // ALGOL execute BTC Rails Yul thunk test
    // Instructions: PUSH 5, PUSH 10, ADD, HALT
    uint8_t test_bytecode[] = { 0x01, 0x00, 0x00, 0x00, 0x05, 0x01, 0x00, 0x00, 0x00, 0x0A, 0x02, 0x06 };
    uint32_t pc = 0, sp = 0, asp = 0, halted = 0;
    assert(tsfi_algol_execute_btc_rails_yul(&test_dat, test_bytecode, sizeof(test_bytecode), 64, &pc, &sp, &asp, &halted) == 0);
    assert(halted == 1);
    assert(pc > 0);

    // PRT 2-3 Tree database & MSCW control language test
    tsfi_prt_database prt_db;
    assert(tsfi_algol_mscw_exec(&prt_db, "MSCW_INIT") == 0);
    assert(tsfi_algol_mscw_exec(&prt_db, "MSCW_INSERT 42 999") == 0);
    assert(tsfi_algol_mscw_exec(&prt_db, "MSCW_INSERT 20 555") == 0);
    assert(tsfi_algol_mscw_exec(&prt_db, "MSCW_INSERT 50 777") == 0);
    assert(tsfi_algol_mscw_exec(&prt_db, "MSCW_FIND 42") == 999);
    assert(tsfi_algol_mscw_exec(&prt_db, "MSCW_FIND 20") == 555);
    assert(tsfi_algol_mscw_exec(&prt_db, "MSCW_FIND 50") == 777);

    // Save and load .dat.bin test
    const char *db_path = "prt_test_db.dat.bin";
    assert(tsfi_algol_prt_save_dat(&prt_db, db_path) == 0);
    tsfi_prt_database loaded_db;
    assert(tsfi_algol_prt_load_dat(&loaded_db, db_path) == 0);
    assert(tsfi_algol_mscw_exec(&loaded_db, "MSCW_FIND 42") == 999);
    remove(db_path);

    // B5500 PRT descriptor resolver test
    uint64_t mem[16] = { 0, 0, 12345678ULL, 0 };
    // Descriptor: present (bit 47), limit=10 (bits 30-45), origin=2 (bits 0-29)
    uint64_t desc = (1ULL << 47) | (10ULL << 30) | 2ULL;
    uint64_t resolved = 0;
    assert(tsfi_b5500_prt_resolve_descriptor(desc, 0, mem, 16, &resolved) == 0);
    assert(resolved == 12345678ULL);

    // Honeywell BCD translation test
    uint8_t honey_bcd[4] = { 27, 28, 29, 30 }; // '0', '1', '2', '3'
    uint8_t ibm_ebcdic[4];
    assert(tsfi_cw_honeywell_bcd_to_ebcdic(honey_bcd, 4, ibm_ebcdic) == 0);
    assert(ibm_ebcdic[0] == 0xF0); // '0' in EBCDIC
    assert(ibm_ebcdic[3] == 0xF3); // '3' in EBCDIC

    // Univac HFP conversion test
    double test_val = 12.345;
    uint64_t u_val = tsfi_cw_double_to_univac(test_val);
    double test_val_back = tsfi_cw_univac_to_double(u_val);
    assert(fabs(test_val_back - test_val) < 0.01);

    // Punched Card Payroll Processing test
    // Columns: 1-6 = "EMP123", 10-14 = "042.5" (42.5 hours), 15-19 = "010.0" ($10.00/hr), 20 = "2" (2 exemptions)
    const char *payroll_card = "EMP123   042.5010.02";
    tsfi_cw_payroll_record pay_rec;
    assert(tsfi_cw_payroll_process_card(payroll_card, &pay_rec) == 0);
    // Gross: 40 * 10 + 2.5 * 15 = 400 + 37.5 = 437.5
    assert(pay_rec.gross_pay == 437.5);
    // FICA: 437.5 * 0.03 = 13.125
    assert(pay_rec.fica_withholding == 13.125);
    // Fed Tax: (437.5 - 26) * 0.18 = 411.5 * 0.18 = 74.07
    assert(fabs(pay_rec.fed_withholding - 74.07) < 0.01);

    // IBM RAMAC Inventory Control test
    tsfi_cw_ramac_stock stock = { "PART99", "GEAR ASM", 100, 5.50, 20 };
    int reorder = 0;
    // Query card
    assert(tsfi_cw_ramac_process_transaction(&stock, "PART99   Q 00000", &reorder) == 0);
    assert(stock.quantity_on_hand == 100);
    assert(reorder == 0);
    // Sale card triggering reorder
    assert(tsfi_cw_ramac_process_transaction(&stock, "PART99   S 00085", &reorder) == 0);
    assert(stock.quantity_on_hand == 15);
    assert(reorder == 1);

    // Corporate Salary Survey Analyzer test
    const char *survey_cards[] = {
        "DEPT01   PG    012000 F",
        "DEPT01   PG    014000 M",
        "DEPT02   OP    008000 M",
        "DEPT02   SA    018000 F"
    };
    tsfi_cw_survey_stats stats;
    assert(tsfi_cw_salary_process_cards(survey_cards, 4, &stats) == 0);
    assert(stats.avg_programmer_salary == 13000.0);
    assert(stats.avg_operator_salary == 8000.0);
    assert(stats.avg_analyst_salary == 18000.0);
    assert(stats.female_count == 2);
    assert(stats.male_count == 2);

    // Simplex Production Optimizer test
    // Maximize P = 40x1 + 30x2
    // Subject to:
    // 2x1 + 1x2 <= 20
    // 1x1 + 2x2 <= 16
    tsfi_cw_simplex_problem prob = {
        40.0, 30.0, // c1, c2
        2.0, 1.0, 20.0, // a11, a12, b1
        1.0, 2.0, 16.0  // a21, a22, b2
    };
    double x1 = 0.0, x2 = 0.0, profit = 0.0;
    assert(tsfi_cw_simplex_optimize(&prob, &x1, &x2, &profit) == 0);
    // Optimal intersection is at x1=8, x2=4 -> Profit = 40*8 + 30*4 = 440
    assert(fabs(x1 - 8.0) < 0.01);
    assert(fabs(x2 - 4.0) < 0.01);
    assert(fabs(profit - 440.0) < 0.01);

    // CPM Project Scheduler test
    tsfi_cw_cpm_task cpm_tasks[3] = {
        { 1, 5, {0}, 0, 0, 0, 0, 0, 0, 0 },           // Task 1: dur=5, no preds
        { 2, 10, {1}, 1, 0, 0, 0, 0, 0, 0 },          // Task 2: dur=10, pred=1
        { 3, 3, {1}, 1, 0, 0, 0, 0, 0, 0 }            // Task 3: dur=3, pred=1
    };
    assert(tsfi_cw_cpm_schedule(cpm_tasks, 3) == 0);
    assert(cpm_tasks[0].early_start == 0 && cpm_tasks[0].early_finish == 5);
    assert(cpm_tasks[1].early_start == 5 && cpm_tasks[1].early_finish == 15);
    assert(cpm_tasks[2].early_start == 5 && cpm_tasks[2].early_finish == 8);
    assert(cpm_tasks[1].is_critical == 1);
    assert(cpm_tasks[2].is_critical == 0 && cpm_tasks[2].slack == 7);

    // Accounts Receivable Ledger test
    // Cards: customer_id (cols 1-6), Date YYMMDD (cols 8-13), type C/P (col 15), amount (cols 17-22)
    const char *ar_cards[] = {
        "CUST01 600601 C 000100",  // Charge $100 on 60/06/01 (over 90 days from 60/10/01)
        "CUST01 600915 C 000050",  // Charge $50 on 60/09/15 (within 30 days)
        "CUST01 600928 P 000020"   // Payment of $20 on 60/09/28 (deducts from the $100 first)
    };
    tsfi_cw_ar_statement ar_stmts[16];
    int stmt_count = 0;
    assert(tsfi_cw_ar_process_ledger(ar_cards, 3, "601001", ar_stmts, &stmt_count) == 0);
    assert(stmt_count == 1);
    assert(strcmp(ar_stmts[0].customer_id, "CUST01") == 0);
    assert(ar_stmts[0].total_balance == 130.0);
    assert(ar_stmts[0].balance_90_days == 80.0);
    assert(ar_stmts[0].balance_current == 50.0);

    // General Ledger Balance Sheet compiler test
    // Cards: Account ID (cols 1-4), Type A/L/E/R/X (col 6), Amount (cols 8-13), D/C (col 15)
    const char *gl_cards[] = {
        "1000 A 010000 D", // Debit Cash $10,000 (Asset)
        "2000 L 004000 C", // Credit Accounts Payable $4,000 (Liability)
        "3000 E 005000 C", // Credit Common Stock $5,000 (Equity)
        "4000 R 002000 C", // Credit Sales Revenue $2,000 (Revenue)
        "5000 X 001000 D"  // Debit Rent Expense $1,000 (Expense)
    };
    tsfi_cw_gl_balance_sheet sheet;
    assert(tsfi_cw_gl_compile_balance_sheet(gl_cards, 5, &sheet) == 0);
    assert(sheet.total_assets == 10000.0);
    assert(sheet.total_liabilities == 4000.0);
    assert(sheet.net_income == 1000.0);
    assert(sheet.total_equity == 6000.0);

    // Mainframe EOQ Calculator test
    tsfi_cw_eoq_problem eoq_prob = { 1000.0, 50.0, 4.0 }; // D=1000, S=50, H=4
    double eoq = 0.0, total_cost = 0.0;
    assert(tsfi_cw_eoq_calculate(&eoq_prob, &eoq, &total_cost) == 0);
    // EOQ = sqrt((2 * 1000 * 50) / 4) = sqrt(25000) = 158.11
    assert(fabs(eoq - 158.11) < 0.01);
    assert(fabs(total_cost - 632.45) < 0.01);

    // PERT Project Scheduler test
    tsfi_cw_pert_task pert_tasks[3] = {
        { 1, 2.0, 5.0, 8.0, {0}, 0, 0, 0 },         // Task 1: Expected=(2+20+8)/6 = 5, Var=((8-2)/6)^2 = 1
        { 2, 6.0, 10.0, 14.0, {1}, 1, 0, 0 },       // Task 2: Expected=(6+40+14)/6 = 10, Var=((14-6)/6)^2 = 1.777
        { 3, 1.0, 3.0, 5.0, {1}, 1, 0, 0 }          // Task 3: Expected=(1+12+5)/6 = 3, Var=((5-1)/6)^2 = 0.444
    };
    double pert_len = 0.0, pert_var = 0.0;
    assert(tsfi_cw_pert_calculate(pert_tasks, 3, &pert_len, &pert_var) == 0);
    assert(fabs(pert_len - 15.0) < 0.01);
    // Critical path tasks are Task 1 and Task 2. Project variance = 1.0 + 1.777 = 2.777
    assert(fabs(pert_var - 2.777) < 0.01);

    // Punched Card Asset Depreciation test
    tsfi_cw_depreciation_asset asset = { "ASSET1", 10000.0, 1000.0, 5 };
    double dep_exp = 0.0, book_val = 0.0;
    // Straight Line: Exp = (10000-1000)/5 = 1800 per year. Year 2 Book value = 10000 - 3600 = 6400
    assert(tsfi_cw_depreciation_calculate(&asset, 'S', 2, &dep_exp, &book_val) == 0);
    assert(fabs(dep_exp - 1800.0) < 0.01);
    assert(fabs(book_val - 6400.0) < 0.01);
    // Double Declining: Rate = 2/5 = 40%. Year 1 Exp = 4000, BV = 6000. Year 2 Exp = 2400, BV = 3600
    assert(tsfi_cw_depreciation_calculate(&asset, 'D', 2, &dep_exp, &book_val) == 0);
    assert(fabs(dep_exp - 2400.0) < 0.01);
    assert(fabs(book_val - 3600.0) < 0.01);

    // Simplex Transportation Optimizer test
    tsfi_cw_transportation_problem trans_prob = {
        { 50.0, 50.0 }, // Supply
        { 40.0, 60.0 }, // Demand
        {
            { 2.0, 5.0 }, // Cost from S1 to D1, D2
            { 3.0, 4.0 }  // Cost from S2 to D1, D2
        }
    };
    double shipment[2][2];
    double trans_cost = 0.0;
    assert(tsfi_cw_transport_optimize(&trans_prob, shipment, &trans_cost) == 0);
    // S1 to D1: 40, S1 to D2: 10, S2 to D1: 0, S2 to D2: 50
    // Total Cost = 40*2 + 10*5 + 0*3 + 50*4 = 80 + 50 + 200 = 330
    assert(shipment[0][0] == 40.0);
    assert(shipment[0][1] == 10.0);
    assert(shipment[1][0] == 0.0);
    assert(shipment[1][1] == 50.0);
    assert(trans_cost == 330.0);

    // Three-Way PO Matcher test
    tsfi_cw_po_record po_rec = { "PO100", "PART9", 100, 1.50 };
    tsfi_cw_receiving_record rr_rec = { "PO100", "PART9", 100 };
    tsfi_cw_invoice_record inv_rec = { "PO100", "PART9", 100, 150.0 };
    tsfi_cw_match_result match_res;
    assert(tsfi_cw_three_way_match(&po_rec, &rr_rec, &inv_rec, &match_res) == 0);
    assert(match_res.matches == 1);
    assert(match_res.qty_mismatch == 0);
    assert(match_res.price_mismatch == 0);
    assert(match_res.status_approved == 1);

    // Resource Leveling test
    // Task 1: dur=3, ES=0, LS=0, rate=5
    // Task 2: dur=2, ES=0, LS=3, rate=3
    // Project horizon = 5 days.
    // If Task 2 starts at ES=0, peak on days 0-1 is 8.
    // If Task 2 starts at LS=3, peak is max(5, 3) = 5.
    tsfi_cw_leveling_task lev_tasks[2] = {
        { 1, 3, 0, 0, 5, 0 },
        { 2, 2, 0, 3, 3, 0 }
    };
    int peak_res = 0;
    assert(tsfi_cw_resource_level(lev_tasks, 2, 5, &peak_res) == 0);
    assert(peak_res == 5);
    assert(lev_tasks[1].scheduled_start == 3);

    // Safety Stock & ROP test
    tsfi_cw_rop_problem rop_prob = {
        1.65, // Z
        100.0, // D
        10.0,  // std_D
        10.0,  // LT
        2.0    // std_LT
    };
    double safety_stock = 0.0, rop = 0.0;
    assert(tsfi_cw_rop_calculate(&rop_prob, &safety_stock, &rop) == 0);
    // term1 = 10 * 100 = 1000
    // term2 = 10000 * 4 = 40000
    // safety_stock = 1.65 * sqrt(41000) = 1.65 * 202.484 = 334.1
    // ROP = 1000 + 334.1 = 1334.1
    assert(fabs(safety_stock - 334.1) < 0.1);
    assert(fabs(rop - 1334.1) < 0.1);

    // ICP Catalog & Award tests
    tsfi_cw_icp_product catalog[32];
    int catalog_size = 0;
    tsfi_cw_icp_product prod1 = { "PROD01", "MARK IV", "Informatics", "IBM 360", 15000.0, 50 };
    tsfi_cw_icp_product prod2 = { "PROD02", "AUTOFLOW", "ADR", "IBM 360", 8000.0, 150 };
    
    assert(tsfi_cw_icp_register_product(catalog, &catalog_size, &prod1) == 0);
    assert(tsfi_cw_icp_register_product(catalog, &catalog_size, &prod2) == 0);
    assert(catalog_size == 2);
    
    tsfi_cw_icp_award_status status;
    // MARK IV: 15,000 * 50 = 750,000 (Does not qualify)
    assert(tsfi_cw_icp_check_award(&catalog[0], &status) == 0);
    assert(status.total_revenue == 750000.0);
    assert(status.qualifies_for_million_dollar_award == 0);
    
    // AUTOFLOW: 8,000 * 150 = 1,200,000 (Qualifies)
    assert(tsfi_cw_icp_check_award(&catalog[1], &status) == 0);
    assert(status.total_revenue == 1200000.0);
    assert(status.qualifies_for_million_dollar_award == 1);

    // ICP Licensing Commission Ledger test
    tsfi_cw_icp_agent agent = { "AGNT99", 0.0, 3000.0, 0.0, 3000.0 };
    // Sale card: agent ID "AGNT99" (cols 1-6), sale amount "060000" (cols 8-13)
    assert(tsfi_cw_icp_process_agent_sale(&agent, "AGNT99 060000") == 0);
    assert(agent.total_sales == 60000.0);
    // Commission: 50,000 * 0.05 + 10,000 * 0.10 = 2500 + 1000 = 3500
    assert(agent.commission_earned == 3500.0);
    assert(agent.total_payout == 6500.0);

    // ICP Maintenance Contract Auditor test
    tsfi_cw_icp_contract contract = { "CON888", "PROD02", 500.0, 12, 4 };
    double tot_val = 0.0, rem_val = 0.0;
    assert(tsfi_cw_icp_audit_contract(&contract, &tot_val, &rem_val) == 0);
    assert(tot_val == 6000.0);
    assert(rem_val == 4000.0);

    // ICP Compatibility Matrix Auditor test
    tsfi_cw_icp_product comp_prod = { "PROD03", "System/360 OS", "IBM", "IBM 360", 45000.0, 300 };
    int is_compatible = 0;
    assert(tsfi_cw_icp_audit_compatibility(&comp_prod, "IBM 360 Model 40", &is_compatible) == 0);
    assert(is_compatible == 1);
    assert(tsfi_cw_icp_audit_compatibility(&comp_prod, "Burroughs B5500", &is_compatible) == 0);
    assert(is_compatible == 0);

    // ICP Software Royalty Distributor test
    double royalty_amt = 0.0;
    // System/360 OS: 45,000 * 300 = 13,500,000. 10% royalty = 1,350,000
    assert(tsfi_cw_icp_distribute_royalties(&comp_prod, 0.10, &royalty_amt) == 0);
    assert(royalty_amt == 1350000.0);

    // ICP Directory Search Engine test
    tsfi_cw_icp_product s_catalog[2] = {
        { "P01", "MARK IV", "Informatics", "IBM 360", 15000.0, 50 },
        { "P02", "AUTOFLOW", "ADR", "IBM 360", 8000.0, 150 }
    };
    tsfi_cw_icp_product search_res[2];
    int search_cnt = 0;
    assert(tsfi_cw_icp_search_directory(s_catalog, 2, "IBM 360", search_res, &search_cnt) == 0);
    assert(search_cnt == 2);

    // ICP Version Upgrade Auditor test
    tsfi_cw_icp_product old_v = { "V1", "AUTOFLOW v1", "ADR", "IBM 360", 8000.0, 150 };
    tsfi_cw_icp_product new_v = { "V2", "AUTOFLOW v2", "ADR", "IBM 360", 9500.0, 200 };
    int upgrade_allowed = 0;
    assert(tsfi_cw_icp_migration_audit(&old_v, &new_v, &upgrade_allowed) == 0);
    assert(upgrade_allowed == 1);

    // ICP Compliance test
    int warning = 0;
    assert(tsfi_cw_icp_audit_license_compliance(100, 120, &warning) == 0);
    assert(warning == 1);
    assert(tsfi_cw_icp_audit_license_compliance(100, 90, &warning) == 0);
    assert(warning == 0);

    // ICP Sales Quota test
    double bonus = 0.0;
    assert(tsfi_cw_icp_track_quota(10000.0, 8000.0, &bonus) == 0);
    assert(bonus == 0.0);
    assert(tsfi_cw_icp_track_quota(10000.0, 11000.0, &bonus) == 0);
    assert(bonus == 220.0); // 11000 * 0.02
    assert(tsfi_cw_icp_track_quota(10000.0, 12500.0, &bonus) == 0);
    assert(bonus == 625.0); // 12500 * 0.05

    // ICP Bundle Pricer test
    tsfi_cw_icp_product bundle[2] = {
        { "P01", "MARK IV", "Informatics", "IBM 360", 15000.0, 50 },
        { "P02", "AUTOFLOW", "ADR", "IBM 360", 8000.0, 150 }
    };
    double total_bundle_price = 0.0;
    assert(tsfi_cw_icp_calculate_bundle_price(bundle, 2, 0.15, &total_bundle_price) == 0);
    assert(total_bundle_price == 19550.0); // 23000 * 0.85

    // ICP Subscription Calculator test
    tsfi_cw_icp_subscription sub = { "SUB01", "P01", 1200.0, 12, 5 };
    double total_paid = 0.0, remaining_obligations = 0.0;
    assert(tsfi_cw_icp_subscription_status(&sub, &total_paid, &remaining_obligations) == 0);
    assert(total_paid == 6000.0);
    assert(remaining_obligations == 8400.0);

    // ICP Vendor Analyzer test
    double vend_rev = 0.0;
    int vend_awards = 0;
    assert(tsfi_cw_icp_analyze_vendor(bundle, 2, "Informatics", &vend_rev, &vend_awards) == 0);
    assert(vend_rev == 750000.0);
    assert(vend_awards == 0);

    // ICP Transfer Tax Calculator test
    double tax_amt = 0.0;
    assert(tsfi_cw_icp_calculate_transfer_tax(1200000.0, 0.04, &tax_amt) == 0);
    assert(tax_amt == 48000.0);

    // ICP Depreciation test
    double dep_val = 0.0;
    assert(tsfi_cw_icp_calculate_software_depreciation(10000.0, 1000.0, 5, 2, &dep_val) == 0);
    assert(fabs(dep_val - 6400.0) < 0.1); // 10000 - (1800 * 2)

    // ICP Renewal Penalty test
    double penalty_val = 0.0;
    assert(tsfi_cw_icp_calculate_renewal_penalty(1500.0, 10, 0.01, &penalty_val) == 0);
    assert(fabs(penalty_val - 1650.0) < 0.1); // 1500 * 1.10

    // ICP License Transfer Auditor test
    tsfi_cw_icp_product trans_prod = { "P99", "EASY-SORT", "OldCorp", "IBM 360", 20000.0, 80 };
    double transfer_fee = 0.0;
    assert(tsfi_cw_icp_audit_transfer(&trans_prod, "NewCorp", 0.05, &transfer_fee) == 0);
    assert(strcmp(trans_prod.vendor, "NewCorp") == 0);
    assert(fabs(transfer_fee - 1000.0) < 0.1); // 20000 * 0.05

    // ICP Software Royalty Ledger Consolidator test
    tsfi_cw_icp_product catalog_list[2] = {
        { "P01", "MARK IV", "Informatics", "IBM 360", 15000.0, 50 },
        { "P02", "AUTOFLOW", "ADR", "IBM 360", 8000.0, 150 }
    };
    double consolidated_payment = 0.0;
    assert(tsfi_cw_icp_consolidate_royalties(catalog_list, 2, "Informatics", 0.10, &consolidated_payment) == 0);
    assert(fabs(consolidated_payment - 75000.0) < 0.1); // 15000 * 50 * 0.10

    // ICP Expiration Monitor test
    int expired = 0, days_rem = 0;
    assert(tsfi_cw_icp_check_expiration(1972, 10, 15, 1973, 5, 1, &expired, &days_rem) == 0);
    assert(expired == 0);
    assert(days_rem == 201); // (1973-1972)*365 + (5-10)*30 + (1-15) = 365 - 150 - 14 = 201

    // ICP Payback Period test
    double payback_yrs = 0.0;
    assert(tsfi_cw_icp_calculate_payback(25000.0, 10000.0, &payback_yrs) == 0);
    assert(fabs(payback_yrs - 2.5) < 0.1);

    // ICP Software Release version control test
    tsfi_cw_icp_release_record releases[32];
    int release_cnt = 0;
    tsfi_cw_icp_release_record new_rel = { "P01", "v1.2", 1970, 8, 12 };
    assert(tsfi_cw_icp_register_release(releases, &release_cnt, &new_rel) == 0);
    assert(release_cnt == 1);

    // ICP Vendor Compliance Auditor test
    tsfi_cw_icp_vendor_record vendor = { "ADR", "US", 1, 0 };
    int compliant = 0;
    assert(tsfi_cw_icp_audit_vendor(&vendor, &compliant) == 0);
    assert(compliant == 1);

    // ICP Category Mapping test
    tsfi_cw_icp_category_map cat_maps[32];
    int cat_cnt = 0;
    assert(tsfi_cw_icp_assign_category(cat_maps, &cat_cnt, "P01", "UTILITY") == 0);
    assert(cat_cnt == 1);
    char cat_res[1][8];
    int cat_res_cnt = 0;
    assert(tsfi_cw_icp_query_category(cat_maps, cat_cnt, "UTILITY", cat_res, &cat_res_cnt) == 0);
    assert(cat_res_cnt == 1);
    assert(strcmp(cat_res[0], "P01") == 0);

    // ICP Grace Period test
    int suspended = 0;
    assert(tsfi_cw_icp_audit_grace_period(15, 30, &suspended) == 0);
    assert(suspended == 0);
    assert(tsfi_cw_icp_audit_grace_period(35, 30, &suspended) == 0);
    assert(suspended == 1);

    // ICP Upgrade Pricing test
    double up_price = 0.0;
    assert(tsfi_cw_icp_calculate_upgrade_price(8000.0, 10000.0, 0.20, &up_price) == 0);
    assert(fabs(up_price - 1600.0) < 0.1); // (10000 - 8000) * 0.80

    // ICP SLA Auditor test
    double rebate = 0.0;
    assert(tsfi_cw_icp_audit_support_sla(120, 60, 500.0, &rebate) == 0);
    assert(fabs(rebate - 25.0) < 0.1); // 500 * 0.05
    assert(tsfi_cw_icp_audit_support_sla(45, 60, 500.0, &rebate) == 0);
    assert(rebate == 0.0);

    // ESJ Paging Telemetry test
    tsfi_cw_esj_paging_metrics metric1 = { 10, 5, 45, 1000, 200 }; // Thrashing index: 15 / 801 = ~0.018. uic: 45. Nominal
    double thrash = 0.0;
    int alert = 0;
    assert(tsfi_cw_esj_analyze_paging(&metric1, &thrash, &alert) == 0);
    assert(fabs(thrash - 0.0187) < 0.001);
    assert(alert == 0);

    tsfi_cw_esj_paging_metrics metric2 = { 100, 200, 15, 1000, 200 }; // uic under 30. Alert!
    assert(tsfi_cw_esj_analyze_paging(&metric2, &thrash, &alert) == 0);
    assert(alert == 1);

    // Marist z/VM migration test
    tsfi_cw_marist_zvm_migration mig_config = { 2048.0, 50.0, 1000.0, 30.0 }; // memory: 2048MB, dirty: 50MB/s, network: 1000Mbps -> 125MB/s. Net: 75MB/s. Time: 27.3s. Nominal!
    double est_time = 0.0;
    int can_mig = 0;
    assert(tsfi_cw_marist_optimize_migration(&mig_config, &est_time, &can_mig) == 0);
    assert(fabs(est_time - 27.3) < 0.1);
    assert(can_mig == 1);

    // Marist SDN Flow Auditor test
    tsfi_cw_marist_sdn_rule rules[2] = {
        { 1, "10.0.0.1", "10.0.0.2", 1, 100 },
        { 2, "10.0.0.3", "10.0.0.4", 2, 50 }
    };
    int action = 0;
    assert(tsfi_cw_marist_audit_sdn(rules, 2, "10.0.0.1", "10.0.0.2", &action) == 0);
    assert(action == 1); // FORWARD
    assert(tsfi_cw_marist_audit_sdn(rules, 2, "10.0.0.5", "10.0.0.6", &action) == 0);
    assert(action == 2); // Default DROP

    // Marist LinuxONE Tenant Audit test
    tsfi_cw_marist_tenant tenant = { "TENANT01", 8.0, 4.0, 32.0, 16.0 };
    int nominal = 0;
    assert(tsfi_cw_marist_audit_tenant(&tenant, &nominal) == 0);
    assert(nominal == 1);

    tsfi_cw_marist_tenant tenant_over = { "TENANT02", 8.0, 10.0, 32.0, 16.0 };
    assert(tsfi_cw_marist_audit_tenant(&tenant_over, &nominal) == 0);
    assert(nominal == 0);

    // Marist z/VM Guest Isolation test
    tsfi_cw_marist_guest_profile profile_secure = { "GUEST01", 'G', 1, 0 }; // Class G, has inter-vm, no spool read
    int secure = 0;
    assert(tsfi_cw_marist_audit_isolation(&profile_secure, &secure) == 0);
    assert(secure == 1);

    tsfi_cw_marist_guest_profile profile_insecure = { "GUEST02", 'G', 1, 1 }; // Class G, has spool read -> INSECURE!
    assert(tsfi_cw_marist_audit_isolation(&profile_insecure, &secure) == 0);
    assert(secure == 0);

    // Marist Sysplex Coupling Facility test
    tsfi_cw_marist_sysplex_cf cf_ok = { 1000, 2, 80, 45.0 };
    int sysplex_alert = 0;
    assert(tsfi_cw_marist_audit_sysplex(&cf_ok, &sysplex_alert) == 0);
    assert(sysplex_alert == 0);

    tsfi_cw_marist_sysplex_cf cf_err = { 1000, 15, 80, 45.0 }; // failed requests > 10
    assert(tsfi_cw_marist_audit_sysplex(&cf_err, &sysplex_alert) == 0);
    assert(sysplex_alert == 1);

    // Marist z/VM Scheduler test
    tsfi_cw_marist_zvm_scheduler sched = { 3, { 100, 200, 200 }, 500 };
    double allocs[16];
    assert(tsfi_cw_marist_calc_cpu_shares(&sched, allocs) == 0);
    assert(fabs(allocs[0] - 0.20) < 0.01);
    assert(fabs(allocs[1] - 0.40) < 0.01);
    assert(fabs(allocs[2] - 0.40) < 0.01);

    // Marist RMF CPU test
    tsfi_cw_marist_rmf_cpu cpus[2] = {
        { 0, 10, 80.0, 0.0 },
        { 1, 35, 98.0, 0.0 }
    };
    double avg_busy = 0.0;
    int overloaded = 0;
    assert(tsfi_cw_marist_audit_rmf_cpu(cpus, 2, &avg_busy, &overloaded) == 0);
    assert(fabs(avg_busy - 89.0) < 0.1);
    assert(overloaded == 1);

    // Marist VSWITCH test
    tsfi_cw_marist_vswitch vsw_ok = { "VSW01", 0.5, 4, 1 };
    int needs_failover = 0;
    assert(tsfi_cw_marist_optimize_vswitch(&vsw_ok, &needs_failover) == 0);
    assert(needs_failover == 0);

    tsfi_cw_marist_vswitch vsw_bad = { "VSW02", 3.2, 4, 1 };
    assert(tsfi_cw_marist_optimize_vswitch(&vsw_bad, &needs_failover) == 0);
    assert(needs_failover == 1);

    // Marist WLM Service Class test
    tsfi_cw_marist_wlm_service srv_ok = { "BAT", 50.0, 0.0, 60, 40 }; // 60/101 = 59.4% velocity >= 50
    int needs_adj = 0;
    assert(tsfi_cw_marist_audit_wlm(&srv_ok, &needs_adj) == 0);
    assert(needs_adj == 0);

    tsfi_cw_marist_wlm_service srv_bad = { "INT", 80.0, 0.0, 30, 70 }; // 30/101 = 29.7% velocity < 80 -> Adjustment needed!
    assert(tsfi_cw_marist_audit_wlm(&srv_bad, &needs_adj) == 0);
    assert(needs_adj == 1);

    // Marist Cryptographic Coprocessor test
    tsfi_cw_marist_crypto_coproc crypto_cfg = { 1, { 2, 3, 5 }, 3, 10 };
    double crypto_allocs[16];
    assert(tsfi_cw_marist_alloc_crypto(&crypto_cfg, crypto_allocs) == 0);
    assert(fabs(crypto_allocs[0] - 0.20) < 0.01);
    assert(fabs(crypto_allocs[1] - 0.30) < 0.01);
    assert(fabs(crypto_allocs[2] - 0.50) < 0.01);

    // Marist Minidisk Cache test
    tsfi_cw_marist_minidisk_cache cache = { 200, 90, 50, 4096 }; // 90/200 = 45% hit ratio (< 60%) -> Resize needed!
    double hit_ratio = 0.0;
    int needs_resize = 0;
    assert(tsfi_cw_marist_audit_minidisk_cache(&cache, &hit_ratio, &needs_resize) == 0);
    assert(fabs(hit_ratio - 0.45) < 0.01);
    assert(needs_resize == 1);

    // Marist ISGLOCK test
    tsfi_cw_marist_isglock lock_ok = { 100, 5, 0, 10 };
    int lock_alert = 0;
    assert(tsfi_cw_marist_audit_isglock(&lock_ok, &lock_alert) == 0);
    assert(lock_alert == 0);

    tsfi_cw_marist_isglock lock_bad = { 100, 25, 0, 10 }; // 25% contention (> 15%)
    assert(tsfi_cw_marist_audit_isglock(&lock_bad, &lock_alert) == 0);
    assert(lock_alert == 1);

    // NIU HLASM test
    uint32_t regs[16] = {0};
    int overflow = 0;
    assert(tsfi_cw_niu_hlasm_exec(regs, "L", 3, 0, 500, &overflow) == 0);
    assert(regs[3] == 500);
    assert(tsfi_cw_niu_hlasm_exec(regs, "L", 4, 0, 200, &overflow) == 0);
    assert(regs[4] == 200);
    assert(tsfi_cw_niu_hlasm_exec(regs, "AR", 3, 4, 0, &overflow) == 0);
    assert(regs[3] == 700);
    assert(overflow == 0);

    // NIU JCL test
    int jcl_ok = 0;
    assert(tsfi_cw_niu_validate_jcl("//MYJOB JOB (1234),CLASS=A", &jcl_ok) == 0);
    assert(jcl_ok == 1);
    assert(tsfi_cw_niu_validate_jcl("INVALID JCL", &jcl_ok) == 0);
    assert(jcl_ok == 0);

    // NIU Macro preprocessor test
    char macro_out[64];
    assert(tsfi_cw_niu_expand_macro("LA 1, &PARAM", "MYVAL", macro_out, sizeof(macro_out)) == 0);
    assert(strcmp(macro_out, "LA 1, MYVAL") == 0);

    // NIU COBOL Copybook test
    tsfi_cw_niu_copybook_field fields[2] = {
        { "CUST-NAME", 5, "PIC X", 30 },
        { "CUST-ID", 5, "PIC 9", 8 }
    };
    int total_bytes = 0;
    assert(tsfi_cw_niu_audit_copybook(fields, 2, &total_bytes) == 0);
    assert(total_bytes == 38);

    // NIU JCL DD path test
    int dd_valid = 0;
    assert(tsfi_cw_niu_audit_jcl_dd("//INPUT DD DSN=SYS1.LINKLIB,DISP=SHR", &dd_valid) == 0);
    assert(dd_valid == 1);
    assert(tsfi_cw_niu_audit_jcl_dd("//INPUT DD DSN=123.BAD.NAME,DISP=SHR", &dd_valid) == 0);
    assert(dd_valid == 0);

    // NIU COBOL Working Storage test
    tsfi_cw_niu_cobol_var vars[2] = {
        { "WS-NAME", 1, 0, "SPACES" },
        { "WS-COUNT", 0, 1, "" } // Uninitialized
    };
    int uninit_cnt = 0;
    assert(tsfi_cw_niu_audit_working_storage(vars, 2, &uninit_cnt) == 0);
    assert(uninit_cnt == 1);

    // ISU LEAP defense test
    tsfi_cw_isu_leap_app app = { "LEAP01", 100.0, 50, 4 }; // Base: 100*0.05 + 50*0.1 = 10; NATO rating 4 -> factor (5-4)*15 = 15. Total base risk = 25.
    double crit_risk = 0.0;
    // Authorized call
    assert(tsfi_cw_isu_leap_defense_audit(&app, 3, "TX01", "ADMIN", &crit_risk) == 0); // DEFCON 3 -> mult 1.5 -> 25 * 1.5 = 37.5
    assert(fabs(crit_risk - 37.5) < 0.1);
    // Unauthorized call (adds 500 penalty)
    assert(tsfi_cw_isu_leap_defense_audit(&app, 3, "TX01", "GUEST", &crit_risk) == 0);
    assert(fabs(crit_risk - 537.5) < 0.1);

    // ISU ULID-SSA test
    int ssa_match = 0;
    assert(tsfi_cw_isu_ulid_ssa_match("gdecke4", "1234", "TX02", "ADMIN", &ssa_match) == 0); // Last char '4' matches '4'
    assert(ssa_match == 1);
    assert(tsfi_cw_isu_ulid_ssa_match("gdecke4", "1235", "TX02", "ADMIN", &ssa_match) == 0);
    assert(ssa_match == 0);
    // Unauthorized call returns security error code -3
    assert(tsfi_cw_isu_ulid_ssa_match("gdecke4", "1234", "TX02", "GUEST", &ssa_match) == -3);

    // ISU legacy email test
    tsfi_cw_isu_email_log mail = { "jqsmith", "test@ilstu.edu", 1000, 1 };
    int email_valid = 0;
    assert(tsfi_cw_isu_audit_email(&mail, &email_valid) == 0);
    assert(email_valid == 1);

    tsfi_cw_isu_email_log mail_bad = { "jqsmith", "test@ilstu.edu", 70000, 1 }; // > 64KB
    assert(tsfi_cw_isu_audit_email(&mail_bad, &email_valid) == 0);
    assert(email_valid == 0);

    // ISU State Farm SLA test
    tsfi_cw_isu_state_farm_sla sla = { "SFJOB01", 90, 100, 0 }; // elapsed (90) <= max (100)
    int sf_compliant = 0;
    assert(tsfi_cw_isu_audit_sf_sla(&sla, &sf_compliant) == 0);
    assert(sf_compliant == 1);

    tsfi_cw_isu_state_farm_sla sla_strict = { "SFJOB02", 90, 100, 1 }; // strict: limit reduced to 80 -> elapsed (90) > limit (80)
    assert(tsfi_cw_isu_audit_sf_sla(&sla_strict, &sf_compliant) == 0);
    assert(sf_compliant == 0);

    // ISU Course Enrollment test
    tsfi_cw_isu_course_enrollment enroll_ok = { "IT262", 25, 30, 5, 5 };
    int can_register = 0;
    assert(tsfi_cw_isu_audit_enrollment(&enroll_ok, &can_register) == 0);
    assert(can_register == 1);

    tsfi_cw_isu_course_enrollment enroll_full = { "IT262", 30, 30, 5, 5 };
    assert(tsfi_cw_isu_audit_enrollment(&enroll_full, &can_register) == 0);
    assert(can_register == 0);

    // ISU Gateway status test
    tsfi_cw_isu_gateway_status gw_ok = { "10.0.0.1", 1000, 5, 45 };
    int gw_alert = 0;
    assert(tsfi_cw_isu_audit_gateway(&gw_ok, &gw_alert) == 0);
    assert(gw_alert == 0);

    tsfi_cw_isu_gateway_status gw_bad = { "10.0.0.1", 1000, 80, 45 }; // 8% drop rate (> 5%)
    assert(tsfi_cw_isu_audit_gateway(&gw_bad, &gw_alert) == 0);
    assert(gw_alert == 1);

    // UNT Master the Mainframe test
    tsfi_cw_unt_mtm_contestant stud = { "McKee", 1, 1, 4, 1, 0 }; // 10 + 30 + 80 + 10 = 130
    int score = 0;
    assert(tsfi_cw_unt_score_contestant(&stud, &score) == 0);
    assert(score == 130);

    // UNT Cryptographic Pipeline test
    tsfi_cw_unt_crypto_pipeline pipe_ok = { 101, "AES-256", 0, 1 };
    int pipe_compliant = 0;
    assert(tsfi_cw_unt_audit_pipeline(&pipe_ok, &pipe_compliant) == 0);
    assert(pipe_compliant == 1);

    tsfi_cw_unt_crypto_pipeline pipe_bad = { 102, "DES", 0, 1 }; // DES encryption
    assert(tsfi_cw_unt_audit_pipeline(&pipe_bad, &pipe_compliant) == 0);
    assert(pipe_compliant == 0);

    // UNT VSAM optimization test
    tsfi_cw_unt_vsam_ksds unt_ksds = { "CUSTOMER.KSDS", 60, 2, 64 }; // CI splits > 50 -> 0x01; buffer < 128 -> 0x02. Total: 0x03
    int unt_flags = 0;
    assert(tsfi_cw_unt_optimize_vsam(&unt_ksds, &unt_flags) == 0);
    assert(unt_flags == 0x03);

    // UNT RACF test
    tsfi_cw_unt_racf_log logs[2] = {
        { "ADMIN01", "PAYROLL.FILE", "READ", 1 },
        { "GUEST01", "PAYROLL.FILE", "READ", 0 } // Unauthorized
    };
    int violations = 0;
    assert(tsfi_cw_unt_audit_racf(logs, 2, &violations) == 0);
    assert(violations == 1);

    // RAF VITAL test
    tsfi_cw_raf_vital_request req = { "ENG001", "AOG", 10, 2 };
    int approved = 0;
    int priority = 0;
    assert(tsfi_cw_raf_allocate_vital(&req, &approved, &priority) == 0);
    assert(approved == 1);
    assert(priority == 100);

    // RAF RIDELS test
    tsfi_cw_raf_ridels_audit items[2] = {
        { "SPARE01", 10, 12, 50.0 }, // diff 2 * 50 = 100.0
        { "SPARE02", 5, 5, 200.0 }
    };
    double discrepancy = 0.0;
    assert(tsfi_cw_raf_audit_ridels(items, 2, &discrepancy) == 0);
    assert(fabs(discrepancy - 100.0) < 0.01);

    // UNT PDS test
    tsfi_cw_unt_pds_status pds_ok = { "SOURCE.PDS", 20, 10, 50 };
    int needs_compress = 0;
    assert(tsfi_cw_unt_audit_pds(&pds_ok, &needs_compress) == 0);
    assert(needs_compress == 0);

    tsfi_cw_unt_pds_status pds_bad = { "SOURCE.PDS", 20, 18, 90 }; // 90% full
    assert(tsfi_cw_unt_audit_pds(&pds_bad, &needs_compress) == 0);
    assert(needs_compress == 1);

    // UNT CICS test
    tsfi_cw_unt_cics_tran trans[2] = {
        { "TRN1", 500, 10, 50 },
        { "TRN2", 1500, 30, 150 } // slow (> 1000)
    };
    double avg_resp = 0.0;
    int slow_cnt = 0;
    assert(tsfi_cw_unt_profile_cics(trans, 2, &avg_resp, &slow_cnt) == 0);
    assert(fabs(avg_resp - 1000.0) < 0.1);
    assert(slow_cnt == 1);

    // UNT CICS session and queue tests
    tsfi_cw_unt_cics_session session_ok = { "USER1", "T001", 1, 150 };
    int needs_reset = 0;
    assert(tsfi_cw_unt_cics_audit_session(&session_ok, &needs_reset) == 0);
    assert(needs_reset == 0);

    tsfi_cw_unt_cics_session session_bad = { "USER2", "T002", 1, 3000 };
    assert(tsfi_cw_unt_cics_audit_session(&session_bad, &needs_reset) == 0);
    assert(needs_reset == 1);

    tsfi_cw_unt_cics_queue queue_ok = { "TSQ01", 100, "TSQ", 1000000 };
    int cics_alert = 0;
    assert(tsfi_cw_unt_cics_audit_queue(&queue_ok, &cics_alert) == 0);
    assert(cics_alert == 0);

    tsfi_cw_unt_cics_queue queue_bad = { "TSQ02", 15000, "TSQ", 1000000 };
    assert(tsfi_cw_unt_cics_audit_queue(&queue_bad, &cics_alert) == 0);
    assert(cics_alert == 1);

    // Ballistic inject test
    tsfi_cw_unt_cics_queue q_ballistic = { "TDQ01", 10, "TDQ", 500 };
    int processed = 0;
    assert(tsfi_cw_unt_cics_inject_ballistic("BALLISTIC_PAYLOAD", 17, &q_ballistic, &processed) == 0);
    assert(processed == 17);
    assert(q_ballistic.item_count == 11);
    assert(q_ballistic.total_bytes == 517);

    // RMU IMS test
    tsfi_cw_rmu_ims_segment ims_db[2] = {
        { "ROOTSEG", "ROOT", 0x1000, 1 },
        { "CHILDSEG", "ROOTSEG", 0x2000, 1 }
    };
    int ims_violations = 0;
    assert(tsfi_cw_rmu_audit_ims(ims_db, 2, &ims_violations) == 0);
    assert(ims_violations == 0);

    tsfi_cw_rmu_ims_segment ims_db_bad[2] = {
        { "ROOTSEG", "ROOT", 0, 1 }, // Invalid pointer (0)
        { "CHILDSEG", "ORPHAN", 0x2000, 1 } // Parent not found in array
    };
    assert(tsfi_cw_rmu_audit_ims(ims_db_bad, 2, &ims_violations) == 0);
    assert(ims_violations == 2);

    // RMU CICS Web Services Gateway test
    tsfi_cw_rmu_cics_web_request web_req_ok = { "POST", 5000, "application/json", 1 };
    int web_compliant = 0;
    assert(tsfi_cw_rmu_audit_cics_web_gateway(&web_req_ok, &web_compliant) == 0);
    assert(web_compliant == 1);

    tsfi_cw_rmu_cics_web_request web_req_bad = { "POST", 2000000, "application/json", 1 }; // too large (> 1MB)
    assert(tsfi_cw_rmu_audit_cics_web_gateway(&web_req_bad, &web_compliant) == 0);
    assert(web_compliant == 0);

    // Chase Manhattan MICR test
    tsfi_cw_chase_micr_check check_ok = { "021000021", "123456789", "1001", 1500.50 };
    int check_valid = 0;
    assert(tsfi_cw_chase_audit_micr(&check_ok, &check_valid) == 0);
    assert(check_valid == 1);

    tsfi_cw_chase_micr_check check_bad = { "021000022", "123456789", "1001", 1500.50 }; // invalid routing check digit
    assert(tsfi_cw_chase_audit_micr(&check_bad, &check_valid) == 0);
    assert(check_valid == 0);

    // Chase Manhattan ATM test
    tsfi_cw_chase_atm_transaction atm_tx_ok = { "4111222233334444", "1234", "WITHDRAWAL", 200.0, 1000.0 };
    int atm_valid = 0;
    assert(tsfi_cw_chase_audit_atm(&atm_tx_ok, &atm_valid) == 0);
    assert(atm_valid == 1);

    tsfi_cw_chase_atm_transaction atm_tx_bad = { "4111222233334444", "123", "WITHDRAWAL", 200.0, 1000.0 }; // pin too short
    assert(tsfi_cw_chase_audit_atm(&atm_tx_bad, &atm_valid) == 0);
    assert(atm_valid == 0);

    // OMP Feilong test
    tsfi_cw_omp_feilong_guest guest;
    assert(tsfi_cw_omp_feilong_provision("GUEST01", 4, 8192, &guest) == 0);
    assert(strcmp(guest.guest_name, "GUEST01") == 0);
    assert(guest.cpu_count == 4);
    assert(guest.memory_mb == 8192);
    assert(strcmp(guest.lifecycle_state, "PROVISIONING") == 0);
    assert(tsfi_cw_omp_feilong_set_state(&guest, "ACTIVE") == 0);
    assert(strcmp(guest.lifecycle_state, "ACTIVE") == 0);

    // OMP Galasa test
    tsfi_cw_omp_galasa_run run;
    assert(tsfi_cw_omp_galasa_init_run("SuiteA", &run) == 0);
    assert(strcmp(run.test_suite_name, "SuiteA") == 0);
    assert(run.assertions_run == 0);
    assert(tsfi_cw_omp_galasa_assert(&run, 1) == 0); // Pass
    assert(tsfi_cw_omp_galasa_assert(&run, 0) == 0); // Fail
    assert(run.assertions_run == 2);
    assert(run.passes == 1);
    assert(run.assertions_failed == 1);

    // OMP Feilong Dispatch test
    char err_buf[128] = {0};
    assert(tsfi_cw_omp_feilong_dispatch("SETSTATE GUEST01 ACTIVE", &guest, err_buf, sizeof(err_buf)) == 0);
    assert(strcmp(guest.lifecycle_state, "ACTIVE") == 0);
    assert(tsfi_cw_omp_feilong_dispatch("INVALID_CMD", &guest, err_buf, sizeof(err_buf)) == 1);
    assert(strlen(err_buf) > 0);

    // Feilong SCALE command & ceiling memory warning threshold check
    err_buf[0] = 0;
    assert(tsfi_cw_omp_feilong_dispatch("SCALE GUEST01 8 32768", &guest, err_buf, sizeof(err_buf)) == 0);
    assert(guest.cpu_count == 8);
    assert(guest.memory_mb == 32768);
    assert(strstr(err_buf, "exceeds 16GB") != NULL);

    // OMP Galasa Diagnostics test
    char report_buf[256] = {0};
    assert(tsfi_cw_omp_galasa_run_diagnostics(&run, report_buf, sizeof(report_buf)) == 0);
    assert(strstr(report_buf, "RED_ALERT") != NULL);

    // OMP Galasa Assert with Retry test
    int attempts = 0;
    assert(tsfi_cw_omp_galasa_assert_with_retry(&run, omp_galasa_mock_eval, &attempts, 3) == 0);
    assert(attempts == 2);

    // OMP Feilong host summary test
    tsfi_cw_omp_feilong_guest guests[2];
    assert(tsfi_cw_omp_feilong_provision("GUEST01", 2, 4096, &guests[0]) == 0);
    assert(tsfi_cw_omp_feilong_set_state(&guests[0], "ACTIVE") == 0);
    assert(tsfi_cw_omp_feilong_provision("GUEST02", 4, 8192, &guests[1]) == 0);
    
    tsfi_cw_omp_feilong_summary summary;
    assert(tsfi_cw_omp_feilong_get_summary(guests, 2, &summary) == 0);
    assert(summary.active_guests == 1);
    assert(summary.total_cpus == 6);
    assert(summary.total_mem_mb == 12288);

    // OMP Galasa assert timed test
    assert(run.total_latency_ns == 0);
    assert(tsfi_cw_omp_galasa_assert_timed(&run, 1, 500) == 0);
    assert(run.total_latency_ns == 500);

    // Feilong de-provisioning command test
    assert(tsfi_cw_omp_feilong_dispatch("DEPROVISION GUEST01", &guests[0], err_buf, sizeof(err_buf)) == 0);
    assert(guests[0].guest_name[0] == 0);
    assert(guests[0].cpu_count == 0);
    assert(guests[0].memory_mb == 0);
    assert(strcmp(guests[0].lifecycle_state, "DEPROVISIONED") == 0);

    // Galasa fail-fast test
    tsfi_cw_omp_galasa_run ff_run;
    assert(tsfi_cw_omp_galasa_init_run("FailFastSuite", &ff_run) == 0);
    ff_run.fail_fast = 1;
    assert(tsfi_cw_omp_galasa_assert(&ff_run, 1) == 0); // Pass
    assert(ff_run.aborted == 0);
    assert(tsfi_cw_omp_galasa_assert(&ff_run, 0) == 0); // Fail
    assert(ff_run.aborted == 1);
    assert(tsfi_cw_omp_galasa_assert(&ff_run, 1) == 0); // Aborted, should bypass
    assert(ff_run.assertions_run == 2); // Assertion run count remains 2

    // Feilong oversubscription test
    tsfi_cw_omp_feilong_guest over_guests[1];
    assert(tsfi_cw_omp_feilong_provision("BIG_GUEST", 16, 70000, &over_guests[0]) == 0);
    assert(tsfi_cw_omp_feilong_get_summary(over_guests, 1, &summary) == 0);
    assert(summary.oversubscribed_alert == 1);

    // Galasa HTML report writer test
    const char *report_file = "galasa_test_report.html";
    assert(tsfi_cw_omp_galasa_write_html_report(&run, report_file) == 0);
    FILE *rep = fopen(report_file, "r");
    assert(rep != NULL);
    fclose(rep);
    remove(report_file);

    // OMP Software Discovery Tool Catalog test
    const char *raw_catalog = "PKG COBOL 15.2 ACTIVE\nPKG ASSEMBLY 2.1 OBSOLETE\n";
    tsfi_cw_omp_sdt_package pkgs[2];
    int pkg_count = 0;
    assert(tsfi_cw_omp_sdt_parse_catalog(raw_catalog, pkgs, 2, &pkg_count) == 0);
    assert(pkg_count == 2);
    assert(strcmp(pkgs[0].package_name, "COBOL") == 0);
    assert(pkgs[0].is_obsolete == 0);
    assert(strcmp(pkgs[1].package_name, "ASSEMBLY") == 0);
    assert(pkgs[1].is_obsolete == 1);

    // OMP Ambitus Command Translator test
    char mvs_cmd[64];
    assert(tsfi_cw_omp_ambitus_translate("ls", mvs_cmd, sizeof(mvs_cmd)) == 0);
    assert(strcmp(mvs_cmd, "D A,L") == 0);
    assert(tsfi_cw_omp_ambitus_translate("grep", mvs_cmd, sizeof(mvs_cmd)) == 0);
    assert(strcmp(mvs_cmd, "D TS") == 0);

    // OMP COBOL syntax verify test
    char cob_err[128] = {0};
    assert(tsfi_cw_omp_cobol_verify_syntax("01 WS-VAR PIC X(10)", cob_err, sizeof(cob_err)) == 1);
    assert(strstr(cob_err, "Missing ending period") != NULL);
    assert(tsfi_cw_omp_cobol_verify_syntax("01 WS-VAR PIC X(10).", cob_err, sizeof(cob_err)) == 0);
    assert(strlen(cob_err) == 0);

    // OMP Software Hub test
    tsfi_cw_omp_hub_pkg repo[2] = {
        { "zowe-cli", "nodejs", 0 },
        { "nodejs", "", 1 } // installed
    };
    int can_install = 0;
    assert(tsfi_cw_omp_hub_check_install("zowe-cli", repo, 2, &can_install) == 0);
    assert(can_install == 1);
    
    repo[1].is_installed = 0; // uninstall nodejs
    assert(tsfi_cw_omp_hub_check_install("zowe-cli", repo, 2, &can_install) == 0);
    assert(can_install == 0);

    // Sammet FORMAC test
    char algebra_res[64];
    assert(tsfi_cw_sammet_formac_eval("DERIVATIVE(x^2, x)", algebra_res, sizeof(algebra_res)) == 0);
    assert(strcmp(algebra_res, "2*x") == 0);
    assert(tsfi_cw_sammet_formac_eval("EXPAND((x+1)*(x+2))", algebra_res, sizeof(algebra_res)) == 0);
    assert(strcmp(algebra_res, "x^2 + 3*x + 2") == 0);

    // Sammet ACM Election Ballot test
    tsfi_cw_sammet_ballot ballots[2] = {
        { "Jean Sammet", 1000, 1 },
        { "Candidate B", 500, 1 }
    };
    int total_votes = 0;
    char winner[32] = {0};
    assert(tsfi_cw_sammet_audit_ballots(ballots, 2, &total_votes, winner, sizeof(winner)) == 0);
    assert(total_votes == 1500);
    assert(strcmp(winner, "Jean Sammet") == 0);

    // NBS Demographic Stratification test
    tsfi_cw_nbs_voter voters[3] = {
        { "NY", 25, 1, 0 },
        { "CA", 45, 1, 0 },
        { "TX", 70, 0, 0 } // not registered
    };
    tsfi_cw_nbs_demographics dem;
    assert(tsfi_cw_nbs_audit_demographics(voters, 3, &dem) == 0);
    assert(dem.age_18_to_35 == 1);
    assert(dem.age_36_to_60 == 1);
    assert(dem.age_over_60 == 0);

    // NBS Polling wait-time optimizer test
    int booths = 0;
    assert(tsfi_cw_nbs_optimize_booths(1000, 10, &booths) == 0);
    assert(booths >= 1);

    // NBS Regional Turnout test
    voters[0].has_voted = 1; // CA has voted
    voters[1].has_voted = 0; // NY hasn't voted
    double turnout = 0.0;
    assert(tsfi_cw_nbs_calc_regional_turnout(voters, 3, "NY", &turnout) == 0);
    assert(turnout == 1.0); // 1 registered voter, 1 voted
    
    // NBS Margin Verification test
    int passes_audit = 0;
    assert(tsfi_cw_nbs_verify_margin(1000, 5, 0.01, &passes_audit) == 0);
    assert(passes_audit == 1); // 5/1000 = 0.005 <= 0.01
    assert(tsfi_cw_nbs_verify_margin(1000, 15, 0.01, &passes_audit) == 0);
    assert(passes_audit == 0); // 15/1000 = 0.015 > 0.01

    // Sammet Language Taxonomy test
    tsfi_cw_sammet_lang lang;
    assert(tsfi_cw_sammet_classify_lang("COBOL", &lang) == 0);
    assert(strcmp(lang.category, "Business Oriented") == 0);
    assert(lang.year == 1959);

    // CODASYL Committee Design Register test
    tsfi_cw_sammet_codasyl_vote vote;
    assert(tsfi_cw_sammet_register_codasyl_vote("ADD DEFINE CLAUSE", 12, 4, &vote) == 0);
    assert(vote.approved == 1);
    assert(strcmp(vote.proposal, "ADD DEFINE CLAUSE") == 0);

    // Kendrick Switch test
    tsfi_cw_kendrick_switch sw = { 0, 5, 101 };
    int connected = 0;
    assert(tsfi_cw_kendrick_route_call(4, &sw, &connected) == 0);
    assert(connected == 1);
    assert(sw.active_lines == 1);

    // Kendrick Integrator test
    double integral = 0.0;
    assert(tsfi_cw_kendrick_integrate(0.1, 1.0, 10, &integral) == 0);
    assert(integral > 2.0); // 1.1^10 ~ 2.59

    // Kendrick Multiplexer test
    int inputs[3] = { 10, 20, 30 };
    int out_combined = 0;
    assert(tsfi_cw_kendrick_multiplex(3, inputs, &out_combined) == 0);
    assert(out_combined == 60);

    // Kendrick Diagnostic Log test
    tsfi_cw_kendrick_log_entry k_logs[2];
    int count = 0;
    assert(tsfi_cw_kendrick_log_event(5, "BUSY", k_logs, 2, &count) == 0);
    assert(count == 1);
    assert(k_logs[0].line_id == 5);
    assert(strcmp(k_logs[0].status, "BUSY") == 0);

    // Keating Core selector test
    tsfi_cw_keating_core cores[1] = { { 4, 8, 0 } };
    assert(tsfi_cw_keating_write_core(4, 8, 1, cores, 1) == 0);
    assert(cores[0].state == 1);

    // Keating Frequency Synthesizer test
    double freq = 0.0;
    assert(tsfi_cw_keating_synthesize_freq(100.0, 5, &freq) == 0);
    assert(freq == 500.0);

    // Keating Bandpass Filter test
    float out_sig = 0.0f;
    assert(tsfi_cw_keating_bandpass_filter(1000.0f, 1000.0f, 100.0f, 6.0f, &out_sig) == 0);
    assert(out_sig > 1900.0f); // 1000 * 10^(6/20) ~ 1995
    assert(tsfi_cw_keating_bandpass_filter(500.0f, 1000.0f, 100.0f, 6.0f, &out_sig) == 0);
    assert(out_sig < 10.0f); // attenuated

    // Hainaut DDL reverse engineering test
    const char *ddl = "CREATE TABLE CUSTOMERS ( CUST_ID PK , ORDER_ID FK REFERENCES ORDERS )\n";
    tsfi_cw_hainaut_table parsed_tbls[1];
    int tbl_count = 0;
    assert(tsfi_cw_hainaut_reverse_engineer(ddl, parsed_tbls, 1, &tbl_count) == 0);
    assert(tbl_count == 1);
    assert(strcmp(parsed_tbls[0].table_name, "CUSTOMERS") == 0);
    assert(strcmp(parsed_tbls[0].primary_key, "CUST_ID") == 0);
    assert(strcmp(parsed_tbls[0].foreign_key, "ORDER_ID") == 0);
    assert(strcmp(parsed_tbls[0].references_table, "ORDERS") == 0);

    // Hainaut schema table split test
    tsfi_cw_hainaut_table tgt_a, tgt_b;
    assert(tsfi_cw_hainaut_migrate_table_split(&parsed_tbls[0], &tgt_a, &tgt_b) == 0);
    assert(strcmp(tgt_a.table_name, "CUSTOMERS_BASE") == 0);
    assert(strcmp(tgt_b.table_name, "CUSTOMERS_EXT") == 0);
    assert(strcmp(tgt_b.references_table, "CUSTOMERS_BASE") == 0);

    // Hainaut ER translation test
    tsfi_cw_hainaut_entity ent = { "User", "ID" };
    tsfi_cw_hainaut_table trsf_tbl;
    assert(tsfi_cw_hainaut_transform_er_to_relational(&ent, &trsf_tbl) == 0);
    assert(strcmp(trsf_tbl.table_name, "T_User") == 0);
    assert(strcmp(trsf_tbl.primary_key, "PK_ID") == 0);

    // Hainaut Schema Merger test
    tsfi_cw_hainaut_table mrg_tbl;
    assert(tsfi_cw_hainaut_merge_schemas(&tgt_a, &tgt_b, &mrg_tbl) == 0);
    assert(strcmp(mrg_tbl.table_name, "CUSTOMERS_BASE_MG") == 0);
    assert(strcmp(mrg_tbl.primary_key, "CUST_ID") == 0);
    assert(strcmp(mrg_tbl.foreign_key, "CUST_ID") == 0);

    // Hainaut Attribute Promoter test
    tsfi_cw_hainaut_table promoted_tbl;
    assert(tsfi_cw_hainaut_promote_attribute(&trsf_tbl, "Email", &promoted_tbl) == 0);
    assert(strcmp(promoted_tbl.table_name, "T_User_Email") == 0);
    assert(strcmp(promoted_tbl.primary_key, "PK_Email") == 0);

    // Hainaut Redundancy Checker test
    tsfi_cw_hainaut_table redundant_list[3];
    // Table 2 (t_i) references Table 1 (t_j)
    strcpy(redundant_list[0].table_name, "Table1");
    redundant_list[0].foreign_key[0] = 0;
    redundant_list[0].references_table[0] = 0;
    
    strcpy(redundant_list[1].table_name, "Table2");
    strcpy(redundant_list[1].foreign_key, "T1_ID");
    strcpy(redundant_list[1].references_table, "Table1");
    
    // Table 3 (t_k) references Table 2 (t_j) which references Table 1
    strcpy(redundant_list[2].table_name, "Table3");
    strcpy(redundant_list[2].foreign_key, "T2_ID");
    strcpy(redundant_list[2].references_table, "Table2");
    
    int redundancy = 0;
    assert(tsfi_cw_hainaut_check_redundancy(redundant_list, 3, &redundancy) == 0);
    assert(redundancy == 1); // Table3 -> Table2 -> Table1 detected

    // Hainaut Attribute Splitter test
    char part_a[32], part_b[32];
    assert(tsfi_cw_hainaut_split_attribute(&trsf_tbl, "FIRST_LAST", part_a, part_b, 32) == 0);
    assert(strcmp(part_a, "FIRST") == 0);
    assert(strcmp(part_b, "LAST") == 0);

    // Hainaut Schema View Generator test
    tsfi_cw_hainaut_view view;
    assert(tsfi_cw_hainaut_generate_view(&trsf_tbl, "PK_ID,FIRST_NAME", &view) == 0);
    assert(strcmp(view.view_name, "V_T_User") == 0);
    assert(strcmp(view.source_table, "T_User") == 0);
    assert(strcmp(view.projected_attributes, "PK_ID,FIRST_NAME") == 0);

    // Hainaut generalization hierarchy transformation test
    tsfi_cw_hainaut_table super_tbl = { "EMPLOYEE", "EMP_ID", "", "" };
    tsfi_cw_hainaut_table sub_tbl = { "HOURLY_EMP", "EMP_ID", "DEPT_ID", "DEPARTMENTS" };
    tsfi_cw_hainaut_table flat_tbl;
    assert(tsfi_cw_hainaut_transform_generalization(&super_tbl, &sub_tbl, &flat_tbl) == 0);
    assert(strcmp(flat_tbl.table_name, "EMPLOYEE_FLAT") == 0);
    assert(strcmp(flat_tbl.primary_key, "EMP_ID") == 0);
    assert(strcmp(flat_tbl.foreign_key, "DEPT_ID") == 0);
    assert(strcmp(flat_tbl.references_table, "DEPARTMENTS") == 0); // inherits references

    // Hainaut Key Degrader test
    tsfi_cw_hainaut_table degraded_tbl;
    assert(tsfi_cw_hainaut_degrade_key(&sub_tbl, &degraded_tbl) == 0);
    assert(strcmp(degraded_tbl.table_name, "HOURLY_EMP_DEG") == 0);
    assert(degraded_tbl.foreign_key[0] == 0);

    // Hainaut Multivalued Promoter test
    tsfi_cw_hainaut_table mult_tbl;
    assert(tsfi_cw_hainaut_promote_multivalued(&super_tbl, "PHONES", &mult_tbl) == 0);
    assert(strcmp(mult_tbl.table_name, "EMPLOYEE_PHONES_1NF") == 0);
    assert(strcmp(mult_tbl.primary_key, "PK_PHONES") == 0);

    // Hainaut Functional Dependency Mapper test
    tsfi_cw_hainaut_fd fd_list[1] = { { "DEPT_ID", "DEPT_NAME" } };
    int fd_violation = 0;
    assert(tsfi_cw_hainaut_map_dependencies(&super_tbl, fd_list, 1, &fd_violation) == 0);
    assert(fd_violation == 1); // DEPT_ID is not EMP_ID (primary key of super_tbl)

    // Hainaut Entity-to-Relationship Demoter test
    tsfi_cw_hainaut_table tgt_io = { "TGT", "TGT_ID", "", "" };
    assert(tsfi_cw_hainaut_demote_entity(&super_tbl, &tgt_io) == 0);
    assert(strcmp(tgt_io.foreign_key, "EMP_ID") == 0);
    assert(strcmp(tgt_io.references_table, "EMPLOYEE") == 0);

    // Hainaut DDL Schema Exporter test
    char ddl_buf[256];
    assert(tsfi_cw_hainaut_export_ddl(&tgt_io, ddl_buf, 256) == 0);
    assert(strcmp(ddl_buf, "CREATE TABLE TGT ( TGT_ID PK , EMP_ID FK REFERENCES EMPLOYEE )") == 0);
}

int main(void) {
    printf("=== RUNNING COMPUTERWORLD MAINFRAME COMPLIANCE TESTS ===\n");
    
    // Initialize firmware and deploy btc_rails_vm so the execution thunks exist
    tsfi_wire_firmware_init();
    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);
    const char *yul_path = "../solidity/bin/btc_rails_vm.yul";
    FILE *f_yul = fopen(yul_path, "r");
    if (f_yul) {
        fclose(f_yul);
    } else {
        yul_path = "solidity/bin/btc_rails_vm.yul";
    }
    char init_cmd[256];
    snprintf(init_cmd, sizeof(init_cmd), "YULINIT \"btc_rails_vm\", \"%s\", 1024", yul_path);
    tsfi_zmm_vm_exec(&vm, init_cmd);

    test_y2k_windowing();
    test_ebcdic_translation();
    test_punch_card_parsing();
    test_cobol_comp3();
    test_new_mainframe_features();
    
    tsfi_zmm_vm_destroy(&vm);
    printf("=== ALL COMPUTERWORLD COMPLIANCE TESTS PASSED ===\n");
    return 0;
}
