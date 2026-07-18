#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
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

static void test_new_mainframe_features(void) {
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
}

int main(void) {
    printf("=== RUNNING COMPUTERWORLD MAINFRAME COMPLIANCE TESTS ===\n");
    test_y2k_windowing();
    test_ebcdic_translation();
    test_punch_card_parsing();
    test_cobol_comp3();
    test_new_mainframe_features();
    printf("=== ALL COMPUTERWORLD COMPLIANCE TESTS PASSED ===\n");
    return 0;
}
