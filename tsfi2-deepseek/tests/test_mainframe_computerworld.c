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
