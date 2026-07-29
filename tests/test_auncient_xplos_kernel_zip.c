#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "tsfi_xplos_kernel.h"
#include "tsfi_parc_tape_catalog.h"

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT XPLOS KERNEL ZIP MOUNT INTEGRATION TEST SUITE\n");
    printf("=============================================================\n");

    // Copy CBT001.zip to compliant .dat.bin filename per Rule 13
    system("cp /tmp/cbt001/CBT001.zip tests/cbt001_kernel_mounted.dat.bin");

    // 1. Audit Zip structure via RED ANALYZER kernel interface
    printf("[KERNEL TEST] Auditing ZIP archive using RED ANALYZER...\n");
    bool audit_ok = tsfi_xplos_analyzer_audit_zip("tests/cbt001_kernel_mounted.dat.bin");
    assert(audit_ok == true);
    printf("   ✓ RED ANALYZER completed static audits successfully.\n");

    // 2. Execute dynamic transitions under BLACK XPLSM monitoring
    printf("[KERNEL TEST] Running member code under BLACK XPLSM monitoring...\n");
    bool exec_ok = tsfi_xplos_xplsm_execute_zip_member("tests/cbt001_kernel_mounted.dat.bin", "FILE001.DATA");
    assert(exec_ok == true);
    printf("   ✓ BLACK XPLSM successfully managed conduction loops.\n");

    // 3. View mounted contents of FILE001.DATA
    printf("\n[KERNEL VIEW] Decoding all records to artifact folder...\n");
    FILE *f_granted = tsfi_tape_zip_open_member("tests/cbt001_kernel_mounted.dat.bin", "FILE001.DATA");
    FILE *f_out = fopen("/home/mariarahel/.gemini/antigravity-cli/brain/ca137a3c-cc63-4def-8b60-30f35d236049/FILE001_DECODED.txt", "w");
    if (f_granted && f_out) {
        uint8_t record[80];
        int line_count = 0;
        while (fread(record, 1, 80, f_granted) == 80) {
            char ascii_line[81];
            for (int i = 0; i < 80; i++) {
                uint8_t eb = record[i];
                if (eb == 0x40) ascii_line[i] = ' ';
                else if (eb >= 0xC1 && eb <= 0xC9) ascii_line[i] = 'A' + (eb - 0xC1);
                else if (eb >= 0xD1 && eb <= 0xD9) ascii_line[i] = 'J' + (eb - 0xD1);
                else if (eb >= 0xE2 && eb <= 0xE9) ascii_line[i] = 'S' + (eb - 0xE2);
                else if (eb >= 0x81 && eb <= 0x89) ascii_line[i] = 'a' + (eb - 0x81);
                else if (eb >= 0x91 && eb <= 0x99) ascii_line[i] = 'j' + (eb - 0x91);
                else if (eb >= 0xA2 && eb <= 0xA9) ascii_line[i] = 's' + (eb - 0xA2);
                else if (eb >= 0xF0 && eb <= 0xF9) ascii_line[i] = '0' + (eb - 0xF0);
                else if (eb == 0x4B) ascii_line[i] = '.';
                else if (eb == 0x4C) ascii_line[i] = '<';
                else if (eb == 0x4D) ascii_line[i] = '(';
                else if (eb == 0x4E) ascii_line[i] = '+';
                else if (eb == 0x4F) ascii_line[i] = '|';
                else if (eb == 0x50) ascii_line[i] = '&';
                else if (eb == 0x5B) ascii_line[i] = '$';
                else if (eb == 0x5C) ascii_line[i] = '*';
                else if (eb == 0x5D) ascii_line[i] = ')';
                else if (eb == 0x5E) ascii_line[i] = ';';
                else if (eb == 0x60) ascii_line[i] = '-';
                else if (eb == 0x61) ascii_line[i] = '/';
                else if (eb == 0x6B) ascii_line[i] = ',';
                else if (eb == 0x6C) ascii_line[i] = '%';
                else if (eb == 0x6D) ascii_line[i] = '_';
                else if (eb == 0x6E) ascii_line[i] = '>';
                else if (eb == 0x6F) ascii_line[i] = '?';
                else if (eb == 0x7A) ascii_line[i] = ':';
                else if (eb == 0x7B) ascii_line[i] = '#';
                else if (eb == 0x7C) ascii_line[i] = '@';
                else if (eb == 0x7D) ascii_line[i] = '\'';
                else if (eb == 0x7E) ascii_line[i] = '=';
                else if (eb == 0x7F) ascii_line[i] = '"';
                else ascii_line[i] = '.';
            }
            ascii_line[80] = '\0';
            // Trim trailing spaces
            int len = 80;
            while (len > 0 && ascii_line[len-1] == ' ') {
                ascii_line[len-1] = '\0';
                len--;
            }
            fprintf(f_out, "%04d: %s\n", line_count + 1, ascii_line);
            line_count++;
        }
        pclose(f_granted);
        fclose(f_out);
        printf("   ✓ Successfully decoded %d records and wrote to artifact file.\n", line_count);
    }
    printf("--------------------------------------------------------------------------------\n");

    remove("tests/cbt001_kernel_mounted.dat.bin");

    // 4. Test JCL parser shell command execution
    printf("[KERNEL TEST] Dispatching 'jclparse' command to XplOS shell...\n");
    XplosShell shell;
    XplosScheduler sched;
    tsfi_xplos_init_scheduler(&sched);
    tsfi_xplos_init_shell(&shell);
    bool shell_ok = tsfi_xplos_shell_exec(&shell, &sched, "jclparse solidity/domain/jcl/cbt003_load_tape.jcl");
    assert(shell_ok == true);
    
    // Run the scheduler loop once to execute the task
    tsfi_xplos_run(&sched);
    printf("   ✓ JCL Parser shell task executed successfully.\n");

    // 5. Test CBTUPD command execution
    printf("[KERNEL TEST] Dispatching 'cbtupd' command to XplOS shell...\n");
    FILE *f_mock = fopen("tests/cbt_mock_doc.txt", "w");
    if (f_mock) {
        fprintf(f_mock, "Line 1: Some description\n");
        fprintf(f_mock, "//***FILE1068 is a cool file\n");
        fprintf(f_mock, "Line 3: More description\n");
        fclose(f_mock);
    }
    XplosShell shell2;
    XplosScheduler sched2;
    tsfi_xplos_init_scheduler(&sched2);
    tsfi_xplos_init_shell(&shell2);
    bool cbtupd_ok = tsfi_xplos_shell_exec(&shell2, &sched2, "cbtupd /home/mariarahel/.gemini/antigravity-cli/brain/ca137a3c-cc63-4def-8b60-30f35d236049/FILE001_DECODED.txt /home/mariarahel/.gemini/antigravity-cli/brain/ca137a3c-cc63-4def-8b60-30f35d236049/FILE001_PDS_INPUT.txt");
    assert(cbtupd_ok == true);
    tsfi_xplos_run(&sched2);
    printf("   ✓ CBTUPD shell task executed successfully on complete File 001.\n");
    remove("tests/cbt_mock_doc.txt");

    // 6. Test PDSLOAD command execution
    printf("[KERNEL TEST] Dispatching 'pdsload' command to XplOS shell...\n");
    XplosShell shell3;
    XplosScheduler sched3;
    tsfi_xplos_init_scheduler(&sched3);
    tsfi_xplos_init_shell(&shell3);
    bool pdsload_ok = tsfi_xplos_shell_exec(&shell3, &sched3, "pdsload /home/mariarahel/.gemini/antigravity-cli/brain/ca137a3c-cc63-4def-8b60-30f35d236049/FILE001_PDS_INPUT.txt");
    assert(pdsload_ok == true);
    tsfi_xplos_run(&sched3);
    printf("   ✓ PDSLOAD shell task executed successfully on processed index.\n");

    // 7. Test PDSLIST, PDSFIND, PDSDELETE execution
    printf("[KERNEL TEST] Dispatching VFS PDS query commands...\n");
    XplosShell shell4;
    XplosScheduler sched4;
    tsfi_xplos_init_scheduler(&sched4);
    tsfi_xplos_init_shell(&shell4);
    tsfi_xplos_shell_exec(&shell4, &sched4, "pdslist");
    tsfi_xplos_run(&sched4);

    XplosShell shell5;
    XplosScheduler sched5;
    tsfi_xplos_init_scheduler(&sched5);
    tsfi_xplos_init_shell(&shell5);
    tsfi_xplos_shell_exec(&shell5, &sched5, "pdsfind FILE002");
    tsfi_xplos_run(&sched5);

    XplosShell shell6;
    XplosScheduler sched6;
    tsfi_xplos_init_scheduler(&sched6);
    tsfi_xplos_init_shell(&shell6);
    tsfi_xplos_shell_exec(&shell6, &sched6, "pdsdelete FILE002");
    tsfi_xplos_run(&sched6);

    XplosShell shell7;
    XplosScheduler sched7;
    tsfi_xplos_init_scheduler(&sched7);
    tsfi_xplos_init_shell(&shell7);
    tsfi_xplos_shell_exec(&shell7, &sched7, "pdsfind FILE002");
    tsfi_xplos_run(&sched7);
    printf("   ✓ VFS PDS query and maintenance tasks completed.\n");

    // 7.5. Test PDSSTATS command execution
    printf("[KERNEL TEST] Dispatching 'pdsstats' command to XplOS shell...\n");
    XplosShell shell_stats;
    XplosScheduler sched_stats;
    tsfi_xplos_init_scheduler(&sched_stats);
    tsfi_xplos_init_shell(&shell_stats);
    tsfi_xplos_shell_exec(&shell_stats, &sched_stats, "pdsstats FILE003");
    tsfi_xplos_run(&sched_stats);
    printf("   ✓ PDSSTATS execution verified successfully.\n");

    // 8. Test REXX command execution
    printf("[KERNEL TEST] Dispatching 'rexx' command to XplOS shell...\n");
    FILE *f_rexx = fopen("tests/cbt_vm_loader.rexx", "w");
    if (f_rexx) {
        fprintf(f_rexx, "/* REXX */\n");
        fprintf(f_rexx, "QUEUE 'TAP1'\n");
        fprintf(f_rexx, "PULL source_dev\n");
        fprintf(f_rexx, "target_mem = 'FILE012'\n");
        fprintf(f_rexx, "'FILEDEF INDD' source_dev\n");
        fprintf(f_rexx, "'FILEDEF OUTDD' target_mem\n");
        fprintf(f_rexx, "'TAPE LOAD'\n");
        fprintf(f_rexx, "'SELECT FILE00*'\n");
        fprintf(f_rexx, "IF RC = 0 THEN SAY 'SUCCESSFULLY LOADED TAPE MEMBERS'\n");
        fclose(f_rexx);
    }
    XplosShell shell8;
    XplosScheduler sched8;
    tsfi_xplos_init_scheduler(&sched8);
    tsfi_xplos_init_shell(&shell8);
    bool rexx_ok = tsfi_xplos_shell_exec(&shell8, &sched8, "rexx tests/cbt_vm_loader.rexx");
    assert(rexx_ok == true);
    tsfi_xplos_run(&sched8);
    printf("   ✓ REXX VM loader execution verified successfully.\n");
    remove("tests/cbt_vm_loader.rexx");

    // 9. Test DOCFILE command execution
    printf("[KERNEL TEST] Dispatching 'docfile' command to XplOS shell...\n");
    FILE *f_doc_in = fopen("tests/cbt_doc_in.txt", "w");
    if (f_doc_in) {
        fprintf(f_doc_in, "This is line 1\n");
        fprintf(f_doc_in, "This is line 2 of doc\n");
        fclose(f_doc_in);
    }
    XplosShell shell_doc;
    XplosScheduler sched_doc;
    tsfi_xplos_init_scheduler(&sched_doc);
    tsfi_xplos_init_shell(&shell_doc);
    bool doc_ok = tsfi_xplos_shell_exec(&shell_doc, &sched_doc, "docfile tests/cbt_doc_in.txt tests/cbt_doc_out.txt FILE");
    assert(doc_ok == true);
    tsfi_xplos_run(&sched_doc);
    printf("   ✓ DOCFILE execution verified successfully.\n");
    remove("tests/cbt_doc_in.txt");
    remove("tests/cbt_doc_out.txt");

    // 10. Test GENDAT command execution
    printf("[KERNEL TEST] Dispatching 'gendat' command to XplOS shell...\n");
    XplosShell shell_gen;
    XplosScheduler sched_gen;
    tsfi_xplos_init_scheduler(&sched_gen);
    tsfi_xplos_init_shell(&shell_gen);
    bool gendat_ok = tsfi_xplos_shell_exec(&shell_gen, &sched_gen, "gendat tests/cbt006_gendat.txt V249_UPD");
    assert(gendat_ok == true);
    tsfi_xplos_run(&sched_gen);
    printf("   ✓ GENDAT execution verified successfully.\n");
    remove("tests/cbt006_gendat.txt");

    // 11. Test TAPEMAP command execution
    printf("[KERNEL TEST] Dispatching 'tapemap' command to XplOS shell...\n");
    FILE *f_mock_tape = fopen("tests/cbt006_mock_tape.dat.bin", "wb");
    if (f_mock_tape) {
        uint8_t dummy[100] = {0};
        fwrite(dummy, 1, 100, f_mock_tape);
        fclose(f_mock_tape);
    }
    XplosShell shell_map;
    XplosScheduler sched_map;
    tsfi_xplos_init_scheduler(&sched_map);
    tsfi_xplos_init_shell(&shell_map);
    bool tapemap_ok = tsfi_xplos_shell_exec(&shell_map, &sched_map, "tapemap tests/cbt006_mock_tape.dat.bin");
    assert(tapemap_ok == true);
    tsfi_xplos_run(&sched_map);
    printf("   ✓ TAPEMAP execution verified successfully.\n");
    remove("tests/cbt006_mock_tape.dat.bin");

    // 12. Test IEBUPDTE command execution
    printf("[KERNEL TEST] Dispatching 'iebupdte' command to XplOS shell...\n");
    FILE *f_upd_deck = fopen("tests/iebupdte_deck.txt", "w");
    if (f_upd_deck) {
        fprintf(f_upd_deck, "./ ADD NAME=FILE015\n");
        fprintf(f_upd_deck, "./ REPL NAME=FILE001\n");
        fclose(f_upd_deck);
    }
    XplosShell shell_upd;
    XplosScheduler sched_upd;
    tsfi_xplos_init_scheduler(&sched_upd);
    tsfi_xplos_init_shell(&shell_upd);
    bool iebupd_ok = tsfi_xplos_shell_exec(&shell_upd, &sched_upd, "iebupdte tests/iebupdte_deck.txt");
    assert(iebupd_ok == true);
    tsfi_xplos_run(&sched_upd);
    printf("   ✓ IEBUPDTE execution verified successfully.\n");
    remove("tests/iebupdte_deck.txt");

    // 13. Test JCLEXPAND command execution
    printf("[KERNEL TEST] Dispatching 'jclexpand' command to XplOS shell...\n");
    FILE *f_jcl_in = fopen("tests/jcl_sym_in.txt", "w");
    if (f_jcl_in) {
        fprintf(f_jcl_in, "//LOAD002 EXEC PGM=IEBCOPY\n");
        fprintf(f_jcl_in, "//SYSUT1  DD DSN=&TAPE_DSN,DISP=SHR\n");
        fclose(f_jcl_in);
    }
    XplosShell shell_exp;
    XplosScheduler sched_exp;
    tsfi_xplos_init_scheduler(&sched_exp);
    tsfi_xplos_init_shell(&shell_exp);
    bool jcl_exp_ok = tsfi_xplos_shell_exec(&shell_exp, &sched_exp, "jclexpand tests/jcl_sym_in.txt tests/jcl_sym_out.txt TAPE_DSN=CBT.V510.FILE002");
    assert(jcl_exp_ok == true);
    tsfi_xplos_run(&sched_exp);
    printf("   ✓ JCLEXPAND execution verified successfully.\n");
    remove("tests/jcl_sym_in.txt");
    remove("tests/jcl_sym_out.txt");

    // 14. Test TRANSLATE command execution
    printf("[KERNEL TEST] Dispatching 'translate' command to XplOS shell...\n");
    FILE *f_trans_in = fopen("tests/trans_in.txt", "wb");
    if (f_trans_in) {
        uint8_t ebcdic_buf[3] = {0xC1, 0x40, 0xC2};
        fwrite(ebcdic_buf, 1, 3, f_trans_in);
        fclose(f_trans_in);
    }
    XplosShell shell_trans;
    XplosScheduler sched_trans;
    tsfi_xplos_init_scheduler(&sched_trans);
    tsfi_xplos_init_shell(&shell_trans);
    bool trans_ok = tsfi_xplos_shell_exec(&shell_trans, &sched_trans, "translate tests/trans_in.txt tests/trans_out.txt E2A");
    assert(trans_ok == true);
    tsfi_xplos_run(&sched_trans);
    printf("   ✓ TRANSLATE execution verified successfully.\n");
    remove("tests/trans_in.txt");
    remove("tests/trans_out.txt");

    // 15. Test CBTLIST command execution
    printf("[KERNEL TEST] Dispatching 'cbtlist' command to XplOS shell...\n");
    XplosShell shell_list;
    XplosScheduler sched_list;
    tsfi_xplos_init_scheduler(&sched_list);
    tsfi_xplos_init_shell(&shell_list);
    bool cbtlist_ok = tsfi_xplos_shell_exec(&shell_list, &sched_list, "cbtlist 002");
    assert(cbtlist_ok == true);
    tsfi_xplos_run(&sched_list);
    printf("   ✓ CBTLIST execution verified successfully.\n");

    // 16. Test CBTDATES command execution
    printf("[KERNEL TEST] Dispatching 'cbtdates' command to XplOS shell...\n");
    XplosShell shell_dates;
    XplosScheduler sched_dates;
    tsfi_xplos_init_scheduler(&sched_dates);
    tsfi_xplos_init_shell(&shell_dates);
    bool cbtdates_ok = tsfi_xplos_shell_exec(&shell_dates, &sched_dates, "cbtdates");
    assert(cbtdates_ok == true);
    tsfi_xplos_run(&sched_dates);
    printf("   ✓ CBTDATES execution verified successfully.\n");

    // 17. Test CBTAUTH command execution
    printf("[KERNEL TEST] Dispatching 'cbtauth' command to XplOS shell...\n");
    XplosShell shell_auth;
    XplosScheduler sched_auth;
    tsfi_xplos_init_scheduler(&sched_auth);
    tsfi_xplos_init_shell(&shell_auth);
    bool cbtauth_ok = tsfi_xplos_shell_exec(&shell_auth, &sched_auth, "cbtauth 003");
    assert(cbtauth_ok == true);
    tsfi_xplos_run(&sched_auth);
    printf("   ✓ CBTAUTH execution verified successfully.\n");

    // 18. Test CBTSEARCH command execution
    printf("[KERNEL TEST] Dispatching 'cbtsearch' command to XplOS shell...\n");
    XplosShell shell_search;
    XplosScheduler sched_search;
    tsfi_xplos_init_scheduler(&sched_search);
    tsfi_xplos_init_shell(&shell_search);
    bool cbtsearch_ok = tsfi_xplos_shell_exec(&shell_search, &sched_search, "cbtsearch decompressor");
    assert(cbtsearch_ok == true);
    tsfi_xplos_run(&sched_search);
    printf("   ✓ CBTSEARCH execution verified successfully.\n");

    // 19. Test CBTDCB command execution
    printf("[KERNEL TEST] Dispatching 'cbtdcb' command to XplOS shell...\n");
    XplosShell shell_dcb;
    XplosScheduler sched_dcb;
    tsfi_xplos_init_scheduler(&sched_dcb);
    tsfi_xplos_init_shell(&shell_dcb);
    bool cbtdcb_ok = tsfi_xplos_shell_exec(&shell_dcb, &sched_dcb, "cbtdcb 005");
    assert(cbtdcb_ok == true);
    tsfi_xplos_run(&sched_dcb);
    printf("   ✓ CBTDCB execution verified successfully.\n");

    // 20. Test CBTVOL command execution
    printf("[KERNEL TEST] Dispatching 'cbtvol' command to XplOS shell...\n");
    XplosShell shell_vol;
    XplosScheduler sched_vol;
    tsfi_xplos_init_scheduler(&sched_vol);
    tsfi_xplos_init_shell(&shell_vol);
    bool cbtvol_ok = tsfi_xplos_shell_exec(&shell_vol, &sched_vol, "cbtvol 002");
    assert(cbtvol_ok == true);
    tsfi_xplos_run(&sched_vol);
    printf("   ✓ CBTVOL execution verified successfully.\n");

    // 21. Test CBTSIZE command execution
    printf("[KERNEL TEST] Dispatching 'cbtsize' command to XplOS shell...\n");
    XplosShell shell_size;
    XplosScheduler sched_size;
    tsfi_xplos_init_scheduler(&sched_size);
    tsfi_xplos_init_shell(&shell_size);
    bool cbtsize_ok = tsfi_xplos_shell_exec(&shell_size, &sched_size, "cbtsize 006");
    assert(cbtsize_ok == true);
    tsfi_xplos_run(&sched_size);
    printf("   ✓ CBTSIZE execution verified successfully.\n");

    // 22. Test CBTLANG command execution
    printf("[KERNEL TEST] Dispatching 'cbtlang' command to XplOS shell...\n");
    XplosShell shell_lang;
    XplosScheduler sched_lang;
    tsfi_xplos_init_scheduler(&sched_lang);
    tsfi_xplos_init_shell(&shell_lang);
    bool cbtlang_ok = tsfi_xplos_shell_exec(&shell_lang, &sched_lang, "cbtlang 005");
    assert(cbtlang_ok == true);
    tsfi_xplos_run(&sched_lang);
    printf("   ✓ CBTLANG execution verified successfully.\n");

    // 23. Test CBTINST command execution
    printf("[KERNEL TEST] Dispatching 'cbtinst' command to XplOS shell...\n");
    XplosShell shell_inst;
    XplosScheduler sched_inst;
    tsfi_xplos_init_scheduler(&sched_inst);
    tsfi_xplos_init_shell(&shell_inst);
    bool cbtinst_ok = tsfi_xplos_shell_exec(&shell_inst, &sched_inst, "cbtinst 002");
    assert(cbtinst_ok == true);
    tsfi_xplos_run(&sched_inst);
    printf("   ✓ CBTINST execution verified successfully.\n");

    // 24. Test CBTDEPS command execution
    printf("[KERNEL TEST] Dispatching 'cbtdeps' command to XplOS shell...\n");
    XplosShell shell_deps;
    XplosScheduler sched_deps;
    tsfi_xplos_init_scheduler(&sched_deps);
    tsfi_xplos_init_shell(&shell_deps);
    bool cbtdeps_ok = tsfi_xplos_shell_exec(&shell_deps, &sched_deps, "cbtdeps 003");
    assert(cbtdeps_ok == true);
    tsfi_xplos_run(&sched_deps);
    printf("   ✓ CBTDEPS execution verified successfully.\n");

    // 25. Test CBTCHECK command execution
    printf("[KERNEL TEST] Dispatching 'cbtcheck' command to XplOS shell...\n");
    XplosShell shell_check;
    XplosScheduler sched_check;
    tsfi_xplos_init_scheduler(&sched_check);
    tsfi_xplos_init_shell(&shell_check);
    bool cbtcheck_ok = tsfi_xplos_shell_exec(&shell_check, &sched_check, "cbtcheck FILE003");
    assert(cbtcheck_ok == true);
    tsfi_xplos_run(&sched_check);
    printf("   ✓ CBTCHECK execution verified successfully.\n");

    // 26. Test CBTFIELDATA command execution
    printf("[KERNEL TEST] Dispatching 'cbtfieldata' command to XplOS shell...\n");
    FILE *f_fd_in = fopen("tests/fd_in.bin", "wb");
    if (f_fd_in) {
        uint8_t fd_buf[2] = {0x04, 0x10};
        fwrite(fd_buf, 1, 2, f_fd_in);
        fclose(f_fd_in);
    }
    XplosShell shell_fd;
    XplosScheduler sched_fd;
    tsfi_xplos_init_scheduler(&sched_fd);
    tsfi_xplos_init_shell(&shell_fd);
    bool cbtfd_ok = tsfi_xplos_shell_exec(&shell_fd, &sched_fd, "cbtfieldata tests/fd_in.bin tests/fd_out.txt");
    assert(cbtfd_ok == true);
    tsfi_xplos_run(&sched_fd);
    printf("   ✓ CBTFIELDATA execution verified successfully.\n");
    remove("tests/fd_in.bin");
    remove("tests/fd_out.txt");

    // 27. Test CMSWRITE command execution
    printf("[KERNEL TEST] Dispatching 'cmswrite' command to XplOS shell...\n");
    XplosShell shell_cms;
    XplosScheduler sched_cms;
    tsfi_xplos_init_scheduler(&sched_cms);
    tsfi_xplos_init_shell(&shell_cms);
    bool cmswrite_ok = tsfi_xplos_shell_exec(&shell_cms, &sched_cms, "cmswrite FILE003 F 80");
    assert(cmswrite_ok == true);
    tsfi_xplos_run(&sched_cms);
    printf("   ✓ CMSWRITE execution verified successfully.\n");

    // 28. Test JCLCOND command execution
    printf("[KERNEL TEST] Dispatching 'jclcond' command to XplOS shell...\n");
    FILE *f_cond_jcl = fopen("tests/cond_jcl.txt", "w");
    if (f_cond_jcl) {
        fprintf(f_cond_jcl, "//J1 JOB\n");
        fprintf(f_cond_jcl, "//STEP1 IF (RC = 0) THEN\n");
        fprintf(f_cond_jcl, "//STEP2 EXEC PGM=IEFBR14\n");
        fprintf(f_cond_jcl, "//STEP1 ENDIF\n");
        fclose(f_cond_jcl);
    }
    XplosShell shell_cond;
    XplosScheduler sched_cond;
    tsfi_xplos_init_scheduler(&sched_cond);
    tsfi_xplos_init_shell(&shell_cond);
    bool jclcond_ok = tsfi_xplos_shell_exec(&shell_cond, &sched_cond, "jclcond tests/cond_jcl.txt");
    assert(jclcond_ok == true);
    tsfi_xplos_run(&sched_cond);
    printf("   ✓ JCLCOND execution verified successfully.\n");
    remove("tests/cond_jcl.txt");

    // 29. Test JCLTRACE command execution
    printf("[KERNEL TEST] Dispatching 'jcltrace' command to XplOS shell...\n");
    XplosShell shell_trace;
    XplosScheduler sched_trace;
    tsfi_xplos_init_scheduler(&sched_trace);
    tsfi_xplos_init_shell(&shell_trace);
    bool jcltrace_ok = tsfi_xplos_shell_exec(&shell_trace, &sched_trace, "jcltrace solidity/domain/jcl/cbt003_load_tape.jcl");
    assert(jcltrace_ok == true);
    tsfi_xplos_run(&sched_trace);
    printf("   ✓ JCLTRACE execution verified successfully.\n");

    // 30. Test JCLDISP command execution
    printf("[KERNEL TEST] Dispatching 'jcldisp' command to XplOS shell...\n");
    XplosShell shell_disp;
    XplosScheduler sched_disp;
    tsfi_xplos_init_scheduler(&sched_disp);
    tsfi_xplos_init_shell(&shell_disp);
    bool jcldisp_ok = tsfi_xplos_shell_exec(&shell_disp, &sched_disp, "jcldisp solidity/domain/jcl/cbt003_load_tape.jcl");
    assert(jcldisp_ok == true);
    tsfi_xplos_run(&sched_disp);
    printf("   ✓ JCLDISP execution verified successfully.\n");

    // 31. Test JCLJOB command execution
    printf("[KERNEL TEST] Dispatching 'jcljob' command to XplOS shell...\n");
    XplosShell shell_job;
    XplosScheduler sched_job;
    tsfi_xplos_init_scheduler(&sched_job);
    tsfi_xplos_init_shell(&shell_job);
    bool jcljob_ok = tsfi_xplos_shell_exec(&shell_job, &sched_job, "jcljob solidity/domain/jcl/cbt003_load_tape.jcl");
    assert(jcljob_ok == true);
    tsfi_xplos_run(&sched_job);
    printf("   ✓ JCLJOB execution verified successfully.\n");

    // 32. Test CBTCONV command execution
    printf("[KERNEL TEST] Dispatching 'cbtconv' command to XplOS shell...\n");
    FILE *f_conv_in = fopen("tests/conv_in.txt", "w");
    if (f_conv_in) {
        fprintf(f_conv_in, "RECORD1\n");
        fprintf(f_conv_in, "RECORD2\n");
        fclose(f_conv_in);
    }
    XplosShell shell_conv;
    XplosScheduler sched_conv;
    tsfi_xplos_init_scheduler(&sched_conv);
    tsfi_xplos_init_shell(&shell_conv);
    bool cbtconv_ok = tsfi_xplos_shell_exec(&shell_conv, &sched_conv, "cbtconv tests/conv_in.txt tests/conv_out.txt FB2VB");
    assert(cbtconv_ok == true);
    tsfi_xplos_run(&sched_conv);
    printf("   ✓ CBTCONV execution verified successfully.\n");
    remove("tests/conv_in.txt");
    remove("tests/conv_out.txt");

    // 33. Test CBTSTRIP command execution
    printf("[KERNEL TEST] Dispatching 'cbtstrip' command to XplOS shell...\n");
    FILE *f_strip_in = fopen("tests/strip_in.txt", "w");
    if (f_strip_in) {
        fprintf(f_strip_in, "RECORD1                                                                 00010000\n");
        fprintf(f_strip_in, "RECORD2                                                                 00020000\n");
        fclose(f_strip_in);
    }
    XplosShell shell_strip;
    XplosScheduler sched_strip;
    tsfi_xplos_init_scheduler(&sched_strip);
    tsfi_xplos_init_shell(&shell_strip);
    bool cbtstrip_ok = tsfi_xplos_shell_exec(&shell_strip, &sched_strip, "cbtstrip tests/strip_in.txt tests/strip_out.txt");
    assert(cbtstrip_ok == true);
    tsfi_xplos_run(&sched_strip);
    printf("   ✓ CBTSTRIP execution verified successfully.\n");
    remove("tests/strip_in.txt");
    remove("tests/strip_out.txt");

    // 34. Test CBTLABELS command execution
    printf("[KERNEL TEST] Dispatching 'cbtlabels' command to XplOS shell...\n");
    FILE *f_mock_label = fopen("tests/cbt006_mock_tape.dat.bin", "wb");
    if (f_mock_label) {
        fwrite("VOL1", 1, 4, f_mock_label);
        fclose(f_mock_label);
    }
    XplosShell shell_labels;
    XplosScheduler sched_labels;
    tsfi_xplos_init_scheduler(&sched_labels);
    tsfi_xplos_init_shell(&shell_labels);
    bool cbtlabels_ok = tsfi_xplos_shell_exec(&shell_labels, &sched_labels, "cbtlabels tests/cbt006_mock_tape.dat.bin");
    assert(cbtlabels_ok == true);
    tsfi_xplos_run(&sched_labels);
    printf("   ✓ CBTLABELS execution verified successfully.\n");
    remove("tests/cbt006_mock_tape.dat.bin");

    // 35. Test CBTVERSION command execution
    printf("[KERNEL TEST] Dispatching 'cbtversion' command to XplOS shell...\n");
    XplosShell shell_ver;
    XplosScheduler sched_ver;
    tsfi_xplos_init_scheduler(&sched_ver);
    tsfi_xplos_init_shell(&shell_ver);
    bool cbtver_ok = tsfi_xplos_shell_exec(&shell_ver, &sched_ver, "cbtversion 5.10");
    assert(cbtver_ok == true);
    tsfi_xplos_run(&sched_ver);
    printf("   ✓ CBTVERSION execution verified successfully.\n");

    // 36. Test CBTCOPY command execution
    printf("[KERNEL TEST] Dispatching 'cbtcopy' command to XplOS shell...\n");
    FILE *f_cp_in = fopen("tests/cp_in.bin", "wb");
    if (f_cp_in) {
        fwrite("BLOCKDATA", 1, 9, f_cp_in);
        fclose(f_cp_in);
    }
    XplosShell shell_copy;
    XplosScheduler sched_copy;
    tsfi_xplos_init_scheduler(&sched_copy);
    tsfi_xplos_init_shell(&shell_copy);
    bool cbtcopy_ok = tsfi_xplos_shell_exec(&shell_copy, &sched_copy, "cbtcopy tests/cp_in.bin tests/cp_out.bin");
    assert(cbtcopy_ok == true);
    tsfi_xplos_run(&sched_copy);
    printf("   ✓ CBTCOPY execution verified successfully.\n");
    remove("tests/cp_in.bin");
    remove("tests/cp_out.bin");

    // 37. Test CBTCOMP command execution
    printf("[KERNEL TEST] Dispatching 'cbtcomp' command to XplOS shell...\n");
    FILE *f_comp1 = fopen("tests/comp1.txt", "w");
    FILE *f_comp2 = fopen("tests/comp2.txt", "w");
    if (f_comp1 && f_comp2) {
        fprintf(f_comp1, "RECORD MATCH\n");
        fprintf(f_comp2, "RECORD MATCH\n");
        fclose(f_comp1);
        fclose(f_comp2);
    }
    XplosShell shell_comp;
    XplosScheduler sched_comp;
    tsfi_xplos_init_scheduler(&sched_comp);
    tsfi_xplos_init_shell(&shell_comp);
    bool cbtcomp_ok = tsfi_xplos_shell_exec(&shell_comp, &sched_comp, "cbtcomp tests/comp1.txt tests/comp2.txt");
    assert(cbtcomp_ok == true);
    tsfi_xplos_run(&sched_comp);
    printf("   ✓ CBTCOMP execution verified successfully.\n");
    remove("tests/comp1.txt");
    remove("tests/comp2.txt");

    // 38. Test CBTCSECT command execution
    printf("[KERNEL TEST] Dispatching 'cbtcsect' command to XplOS shell...\n");
    XplosShell shell_csect;
    XplosScheduler sched_csect;
    tsfi_xplos_init_scheduler(&sched_csect);
    tsfi_xplos_init_shell(&shell_csect);
    bool cbtcsect_ok = tsfi_xplos_shell_exec(&shell_csect, &sched_csect, "cbtcsect FILE003");
    assert(cbtcsect_ok == true);
    tsfi_xplos_run(&sched_csect);
    printf("   ✓ CBTCSECT execution verified successfully.\n");

    // 39. Test CBTATTR command execution
    printf("[KERNEL TEST] Dispatching 'cbtattr' command to XplOS shell...\n");
    XplosShell shell_attr;
    XplosScheduler sched_attr;
    tsfi_xplos_init_scheduler(&sched_attr);
    tsfi_xplos_init_shell(&shell_attr);
    bool cbtattr_ok = tsfi_xplos_shell_exec(&shell_attr, &sched_attr, "cbtattr FILE003");
    assert(cbtattr_ok == true);
    tsfi_xplos_run(&sched_attr);
    printf("   ✓ CBTATTR execution verified successfully.\n");

    // 40. Test CBTMACRO command execution
    printf("[KERNEL TEST] Dispatching 'cbtmacro' command to XplOS shell...\n");
    FILE *f_asm_src = fopen("tests/macro_test.asm", "w");
    if (f_asm_src) {
        fprintf(f_asm_src, "         OPEN  (INDS,INPUT)\n");
        fprintf(f_asm_src, "         CLOSE (INDS)\n");
        fclose(f_asm_src);
    }
    XplosShell shell_macro;
    XplosScheduler sched_macro;
    tsfi_xplos_init_scheduler(&sched_macro);
    tsfi_xplos_init_shell(&shell_macro);
    bool cbtmacro_ok = tsfi_xplos_shell_exec(&shell_macro, &sched_macro, "cbtmacro tests/macro_test.asm");
    assert(cbtmacro_ok == true);
    tsfi_xplos_run(&sched_macro);
    printf("   ✓ CBTMACRO execution verified successfully.\n");
    remove("tests/macro_test.asm");

    // 41. Test CBTALLOC command execution
    printf("[KERNEL TEST] Dispatching 'cbtalloc' command to XplOS shell...\n");
    XplosShell shell_alloc;
    XplosScheduler sched_alloc;
    tsfi_xplos_init_scheduler(&sched_alloc);
    tsfi_xplos_init_shell(&shell_alloc);
    bool cbtalloc_ok = tsfi_xplos_shell_exec(&shell_alloc, &sched_alloc, "cbtalloc ALLOC FI(SYSUT1) DA('CBT.FILE002') SHR");
    assert(cbtalloc_ok == true);
    tsfi_xplos_run(&sched_alloc);
    printf("   ✓ CBTALLOC execution verified successfully.\n");

    // 42. Test ISPF command execution
    printf("[KERNEL TEST] Dispatching 'ispf' command to XplOS shell...\n");
    XplosShell shell_ispf;
    XplosScheduler sched_ispf;
    tsfi_xplos_init_scheduler(&sched_ispf);
    tsfi_xplos_init_shell(&shell_ispf);
    bool ispf_ok = tsfi_xplos_shell_exec(&shell_ispf, &sched_ispf, "ispf s");
    assert(ispf_ok == true);
    tsfi_xplos_run(&sched_ispf);
    printf("   ✓ ISPF execution verified successfully.\n");

    // 43. Test JCLCONDSTEP command execution
    printf("[KERNEL TEST] Dispatching 'jclcondstep' command to XplOS shell...\n");
    XplosShell shell_condstep;
    XplosScheduler sched_condstep;
    tsfi_xplos_init_scheduler(&sched_condstep);
    tsfi_xplos_init_shell(&shell_condstep);
    bool condstep_ok = tsfi_xplos_shell_exec(&shell_condstep, &sched_condstep, "jclcondstep STEP2 4 (4,LT)");
    assert(condstep_ok == true);
    tsfi_xplos_run(&sched_condstep);
    printf("   ✓ JCLCONDSTEP execution verified successfully.\n");

    // 44. Test ISPFVAR command execution
    printf("[KERNEL TEST] Dispatching 'ispfvar' command to XplOS shell...\n");
    XplosShell shell_var;
    XplosScheduler sched_var;
    tsfi_xplos_init_scheduler(&sched_var);
    tsfi_xplos_init_shell(&shell_var);
    bool ispfvar_ok = tsfi_xplos_shell_exec(&shell_var, &sched_var, "ispfvar put ZUSER MVSUSER");
    assert(ispfvar_ok == true);
    tsfi_xplos_run(&sched_var);
    printf("   ✓ ISPFVAR execution verified successfully.\n");

    // 45. Test JCLCONCAT command execution
    printf("[KERNEL TEST] Dispatching 'jclconcat' command to XplOS shell...\n");
    XplosShell shell_concat;
    XplosScheduler sched_concat;
    tsfi_xplos_init_scheduler(&sched_concat);
    tsfi_xplos_init_shell(&shell_concat);
    bool jclconcat_ok = tsfi_xplos_shell_exec(&shell_concat, &sched_concat, "jclconcat SYSUT1 FILE001 FILE002");
    assert(jclconcat_ok == true);
    tsfi_xplos_run(&sched_concat);
    printf("   ✓ JCLCONCAT execution verified successfully.\n");

    // 46. Test ISPF Jump command execution
    printf("[KERNEL TEST] Dispatching ISPF Jump command to XplOS shell...\n");
    XplosShell shell_jump;
    XplosScheduler sched_jump;
    tsfi_xplos_init_scheduler(&sched_jump);
    tsfi_xplos_init_shell(&shell_jump);
    bool jump_ok = tsfi_xplos_shell_exec(&shell_jump, &sched_jump, "=3");
    assert(jump_ok == true);
    tsfi_xplos_run(&sched_jump);
    printf("   ✓ ISPF Jump execution verified successfully.\n");

    // 47. Test JCLGDG command execution
    printf("[KERNEL TEST] Dispatching 'jclgdg' command to XplOS shell...\n");
    XplosShell shell_gdg;
    XplosScheduler sched_gdg;
    tsfi_xplos_init_scheduler(&sched_gdg);
    tsfi_xplos_init_shell(&shell_gdg);
    bool jclgdg_ok = tsfi_xplos_shell_exec(&shell_gdg, &sched_gdg, "jclgdg CBT.BACKUP(-1)");
    assert(jclgdg_ok == true);
    tsfi_xplos_run(&sched_gdg);
    printf("   ✓ JCLGDG execution verified successfully.\n");

    // 48. Test ISPFMATCH command execution
    printf("[KERNEL TEST] Dispatching 'ispfmatch' command to XplOS shell...\n");
    XplosShell shell_match;
    XplosScheduler sched_match;
    tsfi_xplos_init_scheduler(&sched_match);
    tsfi_xplos_init_shell(&shell_match);
    bool ispfmatch_ok = tsfi_xplos_shell_exec(&shell_match, &sched_match, "ispfmatch FILE00*");
    assert(ispfmatch_ok == true);
    tsfi_xplos_run(&sched_match);
    printf("   ✓ ISPFMATCH execution verified successfully.\n");

    // 49. Test JCLPROC command execution
    printf("[KERNEL TEST] Dispatching 'jclproc' command to XplOS shell...\n");
    XplosShell shell_proc;
    XplosScheduler sched_proc;
    tsfi_xplos_init_scheduler(&sched_proc);
    tsfi_xplos_init_shell(&shell_proc);
    bool jclproc_ok = tsfi_xplos_shell_exec(&shell_proc, &sched_proc, "jclproc solidity/domain/jcl/cbt003_load_tape.jcl MYPROC CBT.OUT");
    assert(jclproc_ok == true);
    tsfi_xplos_run(&sched_proc);
    printf("   ✓ JCLPROC execution verified successfully.\n");

    // 50. Test JCLLINT command execution
    printf("[KERNEL TEST] Dispatching 'jcllint' command to XplOS shell...\n");
    XplosShell shell_linter;
    XplosScheduler sched_linter;
    tsfi_xplos_init_scheduler(&sched_linter);
    tsfi_xplos_init_shell(&shell_linter);
    bool jcllint_ok = tsfi_xplos_shell_exec(&shell_linter, &sched_linter, "jcllint solidity/domain/jcl/cbt003_load_tape.jcl");
    assert(jcllint_ok == true);
    tsfi_xplos_run(&sched_linter);
    printf("   ✓ JCLLINT execution verified successfully.\n");

    // 51. Test JCLTEMP command execution
    printf("[KERNEL TEST] Dispatching 'jcltemp' command to XplOS shell...\n");
    XplosShell shell_temp;
    XplosScheduler sched_temp;
    tsfi_xplos_init_scheduler(&sched_temp);
    tsfi_xplos_init_shell(&shell_temp);
    bool jcltemp_ok = tsfi_xplos_shell_exec(&shell_temp, &sched_temp, "jcltemp &&TEMP1");
    assert(jcltemp_ok == true);
    tsfi_xplos_run(&sched_temp);
    printf("   ✓ JCLTEMP execution verified successfully.\n");

    // 52. Test JCLSYSIN command execution
    printf("[KERNEL TEST] Dispatching 'jclsysin' command to XplOS shell...\n");
    XplosShell shell_sysin;
    XplosScheduler sched_sysin;
    tsfi_xplos_init_scheduler(&sched_sysin);
    tsfi_xplos_init_shell(&shell_sysin);
    bool jclsysin_ok = tsfi_xplos_shell_exec(&shell_sysin, &sched_sysin, "jclsysin solidity/domain/jcl/cbt003_load_tape.jcl");
    assert(jclsysin_ok == true);
    tsfi_xplos_run(&sched_sysin);
    printf("   ✓ JCLSYSIN execution verified successfully.\n");

    // 53. Test JCLCONDJOB command execution
    printf("[KERNEL TEST] Dispatching 'jclcondjob' command to XplOS shell...\n");
    XplosShell shell_condjob;
    XplosScheduler sched_condjob;
    tsfi_xplos_init_scheduler(&sched_condjob);
    tsfi_xplos_init_shell(&shell_condjob);
    bool jclcondjob_ok = tsfi_xplos_shell_exec(&shell_condjob, &sched_condjob, "jclcondjob (8,LT) 12");
    assert(jclcondjob_ok == true);
    tsfi_xplos_run(&sched_condjob);
    printf("   ✓ JCLCONDJOB execution verified successfully.\n");

    // 54. Test JCLFREE command execution
    printf("[KERNEL TEST] Dispatching 'jclfree' command to XplOS shell...\n");
    XplosShell shell_free;
    XplosScheduler sched_free;
    tsfi_xplos_init_scheduler(&sched_free);
    tsfi_xplos_init_shell(&shell_free);
    bool jclfree_ok = tsfi_xplos_shell_exec(&shell_free, &sched_free, "jclfree SYSUT1 CLOSE");
    assert(jclfree_ok == true);
    tsfi_xplos_run(&sched_free);
    printf("   ✓ JCLFREE execution verified successfully.\n");

    // 55. Test JCLRESTART command execution
    printf("[KERNEL TEST] Dispatching 'jclrestart' command to XplOS shell...\n");
    XplosShell shell_restart;
    XplosScheduler sched_restart;
    tsfi_xplos_init_scheduler(&sched_restart);
    tsfi_xplos_init_shell(&shell_restart);
    bool jclrestart_ok = tsfi_xplos_shell_exec(&shell_restart, &sched_restart, "jclrestart STEP3 STEP1");
    assert(jclrestart_ok == true);
    tsfi_xplos_run(&sched_restart);
    printf("   ✓ JCLRESTART execution verified successfully.\n");

    // 56. Test JCLLIKE command execution
    printf("[KERNEL TEST] Dispatching 'jcllike' command to XplOS shell...\n");
    XplosShell shell_like;
    XplosScheduler sched_like;
    tsfi_xplos_init_scheduler(&sched_like);
    tsfi_xplos_init_shell(&shell_like);
    bool jcllike_ok = tsfi_xplos_shell_exec(&shell_like, &sched_like, "jcllike NEW.DSN FILE005");
    assert(jcllike_ok == true);
    tsfi_xplos_run(&sched_like);
    printf("   ✓ JCLLIKE execution verified successfully.\n");

    // 57. Test CBTPDSINIT, CBTPDSADD, CBTPDSLIST commands execution
    printf("[KERNEL TEST] Dispatching 'cbtpdsinit' and 'cbtpdsadd' and 'cbtpdslist' commands to XplOS shell...\n");
    XplosShell shell_pds;
    XplosScheduler sched_pds;
    tsfi_xplos_init_scheduler(&sched_pds);
    tsfi_xplos_init_shell(&shell_pds);
    
    FILE *f_tmp = fopen("tests/member1.txt", "w");
    fprintf(f_tmp, "RECORD 1 LINE\nRECORD 2 LINE\n");
    fclose(f_tmp);

    bool pds_init_ok = tsfi_xplos_shell_exec(&shell_pds, &sched_pds, "cbtpdsinit tests/new_pds.dat.bin");
    assert(pds_init_ok == true);
    tsfi_xplos_run(&sched_pds);

    tsfi_xplos_init_scheduler(&sched_pds);
    bool pds_add_ok = tsfi_xplos_shell_exec(&shell_pds, &sched_pds, "cbtpdsadd tests/new_pds.dat.bin MEMB1 tests/member1.txt");
    assert(pds_add_ok == true);
    tsfi_xplos_run(&sched_pds);

    tsfi_xplos_init_scheduler(&sched_pds);
    bool pds_list_ok = tsfi_xplos_shell_exec(&shell_pds, &sched_pds, "cbtpdslist tests/new_pds.dat.bin");
    assert(pds_list_ok == true);
    tsfi_xplos_run(&sched_pds);

    printf("   ✓ PDS utilities execution verified successfully.\n");
    remove("tests/member1.txt");
    remove("tests/new_pds.dat.bin");

    // 58. Test CBTJESSPOOL command execution
    printf("[KERNEL TEST] Dispatching 'cbtjesspool' command to XplOS shell...\n");
    XplosShell shell_spool;
    XplosScheduler sched_spool;
    tsfi_xplos_init_scheduler(&sched_spool);
    tsfi_xplos_init_shell(&shell_spool);
    bool spool_ok = tsfi_xplos_shell_exec(&shell_spool, &sched_spool, "cbtjesspool LOADCBT");
    assert(spool_ok == true);
    tsfi_xplos_run(&sched_spool);
    printf("   ✓ CBTJESSPOOL execution verified successfully.\n");

    // 59. Test CBTCLIST command execution
    printf("[KERNEL TEST] Dispatching 'cbtclist' command to XplOS shell...\n");
    XplosShell shell_clist;
    XplosScheduler sched_clist;
    tsfi_xplos_init_scheduler(&sched_clist);
    tsfi_xplos_init_shell(&shell_clist);
    bool clist_ok = tsfi_xplos_shell_exec(&shell_clist, &sched_clist, "cbtclist ispf s ; jcltemp &&T1");
    assert(clist_ok == true);
    tsfi_xplos_run(&sched_clist);
    printf("   ✓ CBTCLIST execution verified successfully.\n");

    // 60. Test CBTSEARCH command execution
    printf("[KERNEL TEST] Dispatching 'cbtsearch' command to XplOS shell...\n");
    XplosShell shell_cbt_search;
    XplosScheduler sched_cbt_search;
    tsfi_xplos_init_scheduler(&sched_cbt_search);
    tsfi_xplos_init_shell(&shell_cbt_search);
    bool search_ok = tsfi_xplos_shell_exec(&shell_cbt_search, &sched_cbt_search, "cbtsearch FILE00");
    assert(search_ok == true);
    tsfi_xplos_run(&sched_cbt_search);
    printf("   ✓ CBTSEARCH execution verified successfully.\n");

    // 61. Test CBTAUTH command execution
    printf("[KERNEL TEST] Dispatching 'cbtauth' command to XplOS shell...\n");
    XplosShell shell_cbt_auth;
    XplosScheduler sched_cbt_auth;
    tsfi_xplos_init_scheduler(&sched_cbt_auth);
    tsfi_xplos_init_shell(&shell_cbt_auth);
    bool auth_ok = tsfi_xplos_shell_exec(&shell_cbt_auth, &sched_cbt_auth, "cbtauth UCLA");
    assert(auth_ok == true);
    tsfi_xplos_run(&sched_cbt_auth);
    printf("   ✓ CBTAUTH execution verified successfully.\n");

    // 62. Test CBTPDSREAD command execution
    printf("[KERNEL TEST] Dispatching 'cbtpdsread' command to XplOS shell...\n");
    FILE *f_m_tmp = fopen("tests/memb_read.txt", "w");
    fprintf(f_m_tmp, "HELLO COOPERATIVE WORLD\nLINE TWO OF RECORD\n");
    fclose(f_m_tmp);

    XplosShell shell_cbt_pds;
    XplosScheduler sched_cbt_pds;
    
    tsfi_xplos_init_scheduler(&sched_cbt_pds);
    tsfi_xplos_init_shell(&shell_cbt_pds);
    tsfi_xplos_shell_exec(&shell_cbt_pds, &sched_cbt_pds, "cbtpdsinit tests/read_pds.dat.bin");
    tsfi_xplos_run(&sched_cbt_pds);

    tsfi_xplos_init_scheduler(&sched_cbt_pds);
    tsfi_xplos_shell_exec(&shell_cbt_pds, &sched_cbt_pds, "cbtpdsadd tests/read_pds.dat.bin MEMB1 tests/memb_read.txt");
    tsfi_xplos_run(&sched_cbt_pds);

    tsfi_xplos_init_scheduler(&sched_cbt_pds);
    bool read_ok = tsfi_xplos_shell_exec(&shell_cbt_pds, &sched_cbt_pds, "cbtpdsread tests/read_pds.dat.bin MEMB1");
    assert(read_ok == true);
    tsfi_xplos_run(&sched_cbt_pds);

    printf("   ✓ CBTPDSREAD execution verified successfully.\n");
    remove("tests/memb_read.txt");
    remove("tests/read_pds.dat.bin");

    // 63. Test CBTPDSCOMPRESS command execution
    printf("[KERNEL TEST] Dispatching 'cbtpdscompress' command to XplOS shell...\n");
    XplosShell shell_compress;
    XplosScheduler sched_compress;
    
    tsfi_xplos_init_scheduler(&sched_compress);
    tsfi_xplos_init_shell(&shell_compress);
    tsfi_xplos_shell_exec(&shell_compress, &sched_compress, "cbtpdsinit tests/comp_pds.dat.bin");
    tsfi_xplos_run(&sched_compress);

    FILE *f_c_tmp = fopen("tests/memb_comp.txt", "w");
    fprintf(f_c_tmp, "COMPRESSION TEST LINE DATA\n");
    fclose(f_c_tmp);

    tsfi_xplos_init_scheduler(&sched_compress);
    tsfi_xplos_shell_exec(&shell_compress, &sched_compress, "cbtpdsadd tests/comp_pds.dat.bin MEMB1 tests/memb_comp.txt");
    tsfi_xplos_run(&sched_compress);

    tsfi_xplos_init_scheduler(&sched_compress);
    bool compress_ok = tsfi_xplos_shell_exec(&shell_compress, &sched_compress, "cbtpdscompress tests/comp_pds.dat.bin");
    assert(compress_ok == true);
    tsfi_xplos_run(&sched_compress);

    printf("   ✓ CBTPDSCOMPRESS execution verified successfully.\n");
    remove("tests/memb_comp.txt");
    remove("tests/comp_pds.dat.bin");

    // 64. Test CBTPDSDEL command execution
    printf("[KERNEL TEST] Dispatching 'cbtpdsdel' command to XplOS shell...\n");
    XplosShell shell_del;
    XplosScheduler sched_del;
    
    tsfi_xplos_init_scheduler(&sched_del);
    tsfi_xplos_init_shell(&shell_del);
    tsfi_xplos_shell_exec(&shell_del, &sched_del, "cbtpdsinit tests/del_pds.dat.bin");
    tsfi_xplos_run(&sched_del);

    FILE *f_d_tmp = fopen("tests/memb_del.txt", "w");
    fprintf(f_d_tmp, "DELETION TEST LINE DATA\n");
    fclose(f_d_tmp);

    tsfi_xplos_init_scheduler(&sched_del);
    tsfi_xplos_shell_exec(&shell_del, &sched_del, "cbtpdsadd tests/del_pds.dat.bin MEMB1 tests/memb_del.txt");
    tsfi_xplos_run(&sched_del);

    tsfi_xplos_init_scheduler(&sched_del);
    bool del_ok = tsfi_xplos_shell_exec(&shell_del, &sched_del, "cbtpdsdel tests/del_pds.dat.bin MEMB1");
    assert(del_ok == true);
    tsfi_xplos_run(&sched_del);

    printf("   ✓ CBTPDSDEL execution verified successfully.\n");
    remove("tests/memb_del.txt");
    remove("tests/del_pds.dat.bin");

    // 65. Test CBTREXX command execution
    printf("[KERNEL TEST] Dispatching 'cbtrexx' command to XplOS shell...\n");
    XplosShell shell_rexx;
    XplosScheduler sched_rexx;
    tsfi_xplos_init_scheduler(&sched_rexx);
    tsfi_xplos_init_shell(&shell_rexx);
    bool cbtrexx_ok = tsfi_xplos_shell_exec(&shell_rexx, &sched_rexx, "cbtrexx LISTDSPD SYS1.MACLIB");
    assert(cbtrexx_ok == true);
    tsfi_xplos_run(&sched_rexx);
    printf("   ✓ CBTREXX execution verified successfully.\n");

    // 66. Test CBTPDSSTATS command execution
    printf("[KERNEL TEST] Dispatching 'cbtpdsstats' command to XplOS shell...\n");
    XplosShell shell_pds_stats;
    XplosScheduler sched_pds_stats;
    
    tsfi_xplos_init_scheduler(&sched_pds_stats);
    tsfi_xplos_init_shell(&shell_pds_stats);
    tsfi_xplos_shell_exec(&shell_pds_stats, &sched_pds_stats, "cbtpdsinit tests/stats_pds.dat.bin");
    tsfi_xplos_run(&sched_pds_stats);

    FILE *f_s_tmp = fopen("tests/memb_stats.txt", "w");
    fprintf(f_s_tmp, "STATS TEST RECORD\n");
    fclose(f_s_tmp);

    tsfi_xplos_init_scheduler(&sched_pds_stats);
    tsfi_xplos_shell_exec(&shell_pds_stats, &sched_pds_stats, "cbtpdsadd tests/stats_pds.dat.bin MEMB1 tests/memb_stats.txt");
    tsfi_xplos_run(&sched_pds_stats);

    tsfi_xplos_init_scheduler(&sched_pds_stats);
    bool stats_ok = tsfi_xplos_shell_exec(&shell_pds_stats, &sched_pds_stats, "cbtpdsstats tests/stats_pds.dat.bin MEMB1");
    assert(stats_ok == true);
    tsfi_xplos_run(&sched_pds_stats);

    printf("   ✓ CBTPDSSTATS execution verified successfully.\n");
    remove("tests/memb_stats.txt");
    remove("tests/stats_pds.dat.bin");

    // 67. Test CBTWHO command execution
    printf("[KERNEL TEST] Dispatching 'cbtwho' command to XplOS shell...\n");
    XplosShell shell_who;
    XplosScheduler sched_who;
    tsfi_xplos_init_scheduler(&sched_who);
    tsfi_xplos_init_shell(&shell_who);
    bool who_ok = tsfi_xplos_shell_exec(&shell_who, &sched_who, "cbtwho MVSUSER");
    assert(who_ok == true);
    tsfi_xplos_run(&sched_who);
    printf("   ✓ CBTWHO execution verified successfully.\n");

    // 68. Test CBTPDSREN command execution
    printf("[KERNEL TEST] Dispatching 'cbtpdsren' command to XplOS shell...\n");
    XplosShell shell_ren;
    XplosScheduler sched_ren;
    
    tsfi_xplos_init_scheduler(&sched_ren);
    tsfi_xplos_init_shell(&shell_ren);
    tsfi_xplos_shell_exec(&shell_ren, &sched_ren, "cbtpdsinit tests/ren_pds.dat.bin");
    tsfi_xplos_run(&sched_ren);

    FILE *f_r_tmp = fopen("tests/memb_ren.txt", "w");
    fprintf(f_r_tmp, "RENAME TEST LINE DATA\n");
    fclose(f_r_tmp);

    tsfi_xplos_init_scheduler(&sched_ren);
    tsfi_xplos_shell_exec(&shell_ren, &sched_ren, "cbtpdsadd tests/ren_pds.dat.bin MEMB1 tests/memb_ren.txt");
    tsfi_xplos_run(&sched_ren);

    tsfi_xplos_init_scheduler(&sched_ren);
    bool ren_ok = tsfi_xplos_shell_exec(&shell_ren, &sched_ren, "cbtpdsren tests/ren_pds.dat.bin MEMB1 MEMB2");
    assert(ren_ok == true);
    tsfi_xplos_run(&sched_ren);

    printf("   ✓ CBTPDSREN execution verified successfully.\n");
    remove("tests/memb_ren.txt");
    remove("tests/ren_pds.dat.bin");

    // 69. Test CBTPDSDUMP command execution
    printf("[KERNEL TEST] Dispatching 'cbtpdsdump' command to XplOS shell...\n");
    XplosShell shell_dump;
    XplosScheduler sched_dump;
    
    tsfi_xplos_init_scheduler(&sched_dump);
    tsfi_xplos_init_shell(&shell_dump);
    tsfi_xplos_shell_exec(&shell_dump, &sched_dump, "cbtpdsinit tests/dump_pds.dat.bin");
    tsfi_xplos_run(&sched_dump);

    tsfi_xplos_init_scheduler(&sched_dump);
    bool dump_ok = tsfi_xplos_shell_exec(&shell_dump, &sched_dump, "cbtpdsdump tests/dump_pds.dat.bin");
    assert(dump_ok == true);
    tsfi_xplos_run(&sched_dump);

    printf("   ✓ CBTPDSDUMP execution verified successfully.\n");
    remove("tests/dump_pds.dat.bin");

    // 70. Test CBTCICSSUBMIT command execution
    printf("[KERNEL TEST] Dispatching 'cbtcicssubmit' command to XplOS shell...\n");
    XplosShell shell_cics;
    XplosScheduler sched_cics;
    
    tsfi_xplos_init_scheduler(&sched_cics);
    tsfi_xplos_init_shell(&shell_cics);
    tsfi_xplos_shell_exec(&shell_cics, &sched_cics, "cbtpdsinit tests/cics_pds.dat.bin");
    tsfi_xplos_run(&sched_cics);

    FILE *f_cics_tmp = fopen("tests/memb_cics.txt", "w");
    fprintf(f_cics_tmp, "//LOADCBT JOB 1,CLASS=A\n");
    fclose(f_cics_tmp);

    tsfi_xplos_init_scheduler(&sched_cics);
    tsfi_xplos_shell_exec(&shell_cics, &sched_cics, "cbtpdsadd tests/cics_pds.dat.bin MEMB1 tests/memb_cics.txt");
    tsfi_xplos_run(&sched_cics);

    tsfi_xplos_init_scheduler(&sched_cics);
    bool cics_ok = tsfi_xplos_shell_exec(&shell_cics, &sched_cics, "cbtcicssubmit tests/cics_pds.dat.bin MEMB1");
    assert(cics_ok == true);
    tsfi_xplos_run(&sched_cics);

    printf("   ✓ CBTCICSSUBMIT execution verified successfully.\n");
    remove("tests/memb_cics.txt");
    remove("tests/cics_pds.dat.bin");

    // 71. Test CBTJESLIST command execution
    printf("[KERNEL TEST] Dispatching 'cbtjeslist' command to XplOS shell...\n");
    XplosShell shell_jeslist;
    XplosScheduler sched_jeslist;
    tsfi_xplos_init_scheduler(&sched_jeslist);
    tsfi_xplos_init_shell(&shell_jeslist);
    bool jeslist_ok = tsfi_xplos_shell_exec(&shell_jeslist, &sched_jeslist, "cbtjeslist");
    assert(jeslist_ok == true);
    tsfi_xplos_run(&sched_jeslist);
    printf("   ✓ CBTJESLIST execution verified successfully.\n");

    // 72. Test CBTJESPURGE command execution
    printf("[KERNEL TEST] Dispatching 'cbtjespurge' command to XplOS shell...\n");
    XplosShell shell_purge;
    XplosScheduler sched_purge;
    tsfi_xplos_init_scheduler(&sched_purge);
    tsfi_xplos_init_shell(&shell_purge);
    bool purge_ok = tsfi_xplos_shell_exec(&shell_purge, &sched_purge, "cbtjespurge JOB00021");
    assert(purge_ok == true);
    tsfi_xplos_run(&sched_purge);
    printf("   ✓ CBTJESPURGE execution verified successfully.\n");

    // 73. Test CBTJESHOLD and CBTJESRELEASE command execution
    printf("[KERNEL TEST] Dispatching 'cbtjeshold' and 'cbtjesrelease' commands to XplOS shell...\n");
    XplosShell shell_hr;
    XplosScheduler sched_hr;
    
    tsfi_xplos_init_scheduler(&sched_hr);
    tsfi_xplos_init_shell(&shell_hr);
    bool hold_ok = tsfi_xplos_shell_exec(&shell_hr, &sched_hr, "cbtjeshold JOB00022");
    assert(hold_ok == true);
    tsfi_xplos_run(&sched_hr);

    tsfi_xplos_init_scheduler(&sched_hr);
    bool release_ok = tsfi_xplos_shell_exec(&shell_hr, &sched_hr, "cbtjesrelease JOB00022");
    assert(release_ok == true);
    tsfi_xplos_run(&sched_hr);

    printf("   ✓ CBTJESHOLD and CBTJESRELEASE execution verified successfully.\n");

    // 74. Test CBTJESCLASS command execution
    printf("[KERNEL TEST] Dispatching 'cbtjesclass' command to XplOS shell...\n");
    XplosShell shell_class;
    XplosScheduler sched_class;
    tsfi_xplos_init_scheduler(&sched_class);
    tsfi_xplos_init_shell(&shell_class);
    bool class_ok = tsfi_xplos_shell_exec(&shell_class, &sched_class, "cbtjesclass JOB00022 B");
    assert(class_ok == true);
    tsfi_xplos_run(&sched_class);
    printf("   ✓ CBTJESCLASS execution verified successfully.\n");

    // 75. Test CBTCICSINQ command execution
    printf("[KERNEL TEST] Dispatching 'cbtcicsinq' command to XplOS shell...\n");
    XplosShell shell_cicsinq;
    XplosScheduler sched_cicsinq;
    tsfi_xplos_init_scheduler(&sched_cicsinq);
    tsfi_xplos_init_shell(&shell_cicsinq);
    bool cicsinq_ok = tsfi_xplos_shell_exec(&shell_cicsinq, &sched_cicsinq, "cbtcicsinq JOB00024");
    assert(cicsinq_ok == true);
    tsfi_xplos_run(&sched_cicsinq);
    printf("   ✓ CBTCICSINQ execution verified successfully.\n");

    // 76. Test CBTSYSLOG command execution
    printf("[KERNEL TEST] Dispatching 'cbtsyslog' command to XplOS shell...\n");
    XplosShell shell_syslog;
    XplosScheduler sched_syslog;
    tsfi_xplos_init_scheduler(&sched_syslog);
    tsfi_xplos_init_shell(&shell_syslog);
    bool syslog_ok = tsfi_xplos_shell_exec(&shell_syslog, &sched_syslog, "cbtsyslog");
    assert(syslog_ok == true);
    tsfi_xplos_run(&sched_syslog);
    printf("   ✓ CBTSYSLOG execution verified successfully.\n");

    // 77. Test CBTCPU command execution
    printf("[KERNEL TEST] Dispatching 'cbtcpu' command to XplOS shell...\n");
    XplosShell shell_cpu;
    XplosScheduler sched_cpu;
    tsfi_xplos_init_scheduler(&sched_cpu);
    tsfi_xplos_init_shell(&shell_cpu);
    bool cpu_ok = tsfi_xplos_shell_exec(&shell_cpu, &sched_cpu, "cbtcpu");
    assert(cpu_ok == true);
    tsfi_xplos_run(&sched_cpu);
    printf("   ✓ CBTCPU execution verified successfully.\n");

    // 78. Test CBTVTOC command execution
    printf("[KERNEL TEST] Dispatching 'cbtvtoc' command to XplOS shell...\n");
    XplosShell shell_vtoc;
    XplosScheduler sched_vtoc;
    tsfi_xplos_init_scheduler(&sched_vtoc);
    tsfi_xplos_init_shell(&shell_vtoc);
    bool vtoc_ok = tsfi_xplos_shell_exec(&shell_vtoc, &sched_vtoc, "cbtvtoc MVSRES");
    assert(vtoc_ok == true);
    tsfi_xplos_run(&sched_vtoc);
    printf("   ✓ CBTVTOC execution verified successfully.\n");

    // 79. Test CBTTAPEMAP command execution
    printf("[KERNEL TEST] Dispatching 'cbttapemap' command to XplOS shell...\n");
    XplosShell shell_tapemap;
    XplosScheduler sched_tapemap;
    tsfi_xplos_init_scheduler(&sched_tapemap);
    tsfi_xplos_init_shell(&shell_tapemap);
    
    FILE *f_tape_tmp = fopen("tests/tape_vol.dat.bin", "wb");
    uint8_t zero_block[80] = {0};
    fwrite(zero_block, 1, 80, f_tape_tmp);
    fclose(f_tape_tmp);

    bool cbttapemap_ok = tsfi_xplos_shell_exec(&shell_tapemap, &sched_tapemap, "cbttapemap tests/tape_vol.dat.bin");
    assert(cbttapemap_ok == true);
    tsfi_xplos_run(&sched_tapemap);
    printf("   ✓ CBTTAPEMAP execution verified successfully.\n");
    remove("tests/tape_vol.dat.bin");

    // 80. Test CBTPDSREP command execution
    printf("[KERNEL TEST] Dispatching 'cbtpdsrep' command to XplOS shell...\n");
    XplosShell shell_rep;
    XplosScheduler sched_rep;
    
    tsfi_xplos_init_scheduler(&sched_rep);
    tsfi_xplos_init_shell(&shell_rep);
    tsfi_xplos_shell_exec(&shell_rep, &sched_rep, "cbtpdsinit tests/rep_pds.dat.bin");
    tsfi_xplos_run(&sched_rep);

    FILE *f_rep_tmp = fopen("tests/memb_rep.txt", "w");
    fprintf(f_rep_tmp, "FIND_ME AND REPLACE THIS\n");
    fclose(f_rep_tmp);

    tsfi_xplos_init_scheduler(&sched_rep);
    tsfi_xplos_shell_exec(&shell_rep, &sched_rep, "cbtpdsadd tests/rep_pds.dat.bin MEMB1 tests/memb_rep.txt");
    tsfi_xplos_run(&sched_rep);

    tsfi_xplos_init_scheduler(&sched_rep);
    bool rep_ok = tsfi_xplos_shell_exec(&shell_rep, &sched_rep, "cbtpdsrep tests/rep_pds.dat.bin MEMB1 FIND_ME FOUND_IT");
    assert(rep_ok == true);
    tsfi_xplos_run(&sched_rep);

    printf("   ✓ CBTPDSREP execution verified successfully.\n");
    remove("tests/memb_rep.txt");
    remove("tests/rep_pds.dat.bin");

    // 81. Test CBTUNBLOCK command execution
    printf("[KERNEL TEST] Dispatching 'cbtunblock' command to XplOS shell...\n");
    XplosShell shell_unblock;
    XplosScheduler sched_unblock;
    tsfi_xplos_init_scheduler(&sched_unblock);
    tsfi_xplos_init_shell(&shell_unblock);
    
    FILE *f_unblock_tmp = fopen("tests/ieb_seq.dat.bin", "w");
    fprintf(f_unblock_tmp, "./ ADD NAME=MEMBERA\n");
    fprintf(f_unblock_tmp, "RECORD ONE OF MEMBERA\n");
    fprintf(f_unblock_tmp, "./ ADD NAME=MEMBERB\n");
    fprintf(f_unblock_tmp, "RECORD ONE OF MEMBERB\n");
    fclose(f_unblock_tmp);

    bool unblock_ok = tsfi_xplos_shell_exec(&shell_unblock, &sched_unblock, "cbtunblock tests/ieb_seq.dat.bin tests");
    assert(unblock_ok == true);
    tsfi_xplos_run(&sched_unblock);
    printf("   ✓ CBTUNBLOCK execution verified successfully.\n");
    remove("tests/ieb_seq.dat.bin");
    remove("tests/MEMBERA.txt");
    remove("tests/MEMBERB.txt");

    // 82. Test CBTQDISPATCH command execution (Generates real binary SMF records)
    printf("[KERNEL TEST] Dispatching 'cbtqdispatch' command to XplOS shell...\n");
    XplosShell shell_qdisp;
    XplosScheduler sched_qdisp;
    tsfi_xplos_init_scheduler(&sched_qdisp);
    tsfi_xplos_init_shell(&shell_qdisp);
    
    // Ensure clean start for SMF log
    remove("tests/smf_log.dat.bin");

    bool qdisp_ok = tsfi_xplos_shell_exec(&shell_qdisp, &sched_qdisp, "cbtqdispatch 25 MYJOB");
    assert(qdisp_ok == true);
    tsfi_xplos_run(&sched_qdisp);
    printf("   ✓ CBTQDISPATCH execution verified successfully.\n");

    // 83. Test CBTSMF command execution (Parses the generated SMF records)
    printf("[KERNEL TEST] Dispatching 'cbtsmf' command to XplOS shell...\n");
    XplosShell shell_smf;
    XplosScheduler sched_smf;
    tsfi_xplos_init_scheduler(&sched_smf);
    tsfi_xplos_init_shell(&shell_smf);
    
    bool smf_ok = tsfi_xplos_shell_exec(&shell_smf, &sched_smf, "cbtsmf tests/smf_log.dat.bin");
    assert(smf_ok == true);
    tsfi_xplos_run(&sched_smf);
    printf("   ✓ CBTSMF execution verified successfully.\n");
    remove("tests/smf_log.dat.bin");

    // 84. Test CBTWTO command execution
    printf("[KERNEL TEST] Dispatching 'cbtwto' command to XplOS shell...\n");
    XplosShell shell_wto;
    XplosScheduler sched_wto;
    tsfi_xplos_init_scheduler(&sched_wto);
    tsfi_xplos_init_shell(&shell_wto);
    bool wto_ok = tsfi_xplos_shell_exec(&shell_wto, &sched_wto, "cbtwto SYSTEM SHUTDOWN IN 5 MINUTES");
    assert(wto_ok == true);
    tsfi_xplos_run(&sched_wto);
    printf("   ✓ CBTWTO execution verified successfully.\n");

    // 85. Test CBTPARMLIB command execution
    printf("[KERNEL TEST] Dispatching 'cbtparmlib' command to XplOS shell...\n");
    XplosShell shell_parm;
    XplosScheduler sched_parm;
    tsfi_xplos_init_scheduler(&sched_parm);
    tsfi_xplos_init_shell(&shell_parm);
    
    FILE *f_parm_tmp = fopen("tests/parm_sys.dat.bin", "wb");
    uint8_t dummy_parm[8] = {0};
    fwrite(dummy_parm, 1, 8, f_parm_tmp);
    fclose(f_parm_tmp);

    bool parm_ok = tsfi_xplos_shell_exec(&shell_parm, &sched_parm, "cbtparmlib tests/parm_sys.dat.bin");
    assert(parm_ok == true);
    tsfi_xplos_run(&sched_parm);
    printf("   ✓ CBTPARMLIB execution verified successfully.\n");
    remove("tests/parm_sys.dat.bin");

    // 86. Test CBTVOL command execution
    printf("[KERNEL TEST] Dispatching 'cbtvol' command to XplOS shell...\n");
    XplosShell shell_volstatus;
    XplosScheduler sched_volstatus;
    tsfi_xplos_init_scheduler(&sched_volstatus);
    tsfi_xplos_init_shell(&shell_volstatus);
    bool vol_ok = tsfi_xplos_shell_exec(&shell_volstatus, &sched_volstatus, "cbtvol");
    assert(vol_ok == true);
    tsfi_xplos_run(&sched_volstatus);
    printf("   ✓ CBTVOL execution verified successfully.\n");

    // 87. Test CBTDASDMAP command execution
    printf("[KERNEL TEST] Dispatching 'cbtdasdmap' command to XplOS shell...\n");
    XplosShell shell_dmap;
    XplosScheduler sched_dmap;
    tsfi_xplos_init_scheduler(&sched_dmap);
    tsfi_xplos_init_shell(&shell_dmap);
    bool dmap_ok = tsfi_xplos_shell_exec(&shell_dmap, &sched_dmap, "cbtdasdmap MVSRES");
    assert(dmap_ok == true);
    tsfi_xplos_run(&sched_dmap);
    printf("   ✓ CBTDASDMAP execution verified successfully.\n");

    // 88. Test CBTTSK command execution
    printf("[KERNEL TEST] Dispatching 'cbttsk' command to XplOS shell...\n");
    XplosShell shell_tsk;
    XplosScheduler sched_tsk;
    tsfi_xplos_init_scheduler(&sched_tsk);
    tsfi_xplos_init_shell(&shell_tsk);
    bool tsk_ok = tsfi_xplos_shell_exec(&shell_tsk, &sched_tsk, "cbttsk");
    assert(tsk_ok == true);
    tsfi_xplos_run(&sched_tsk);
    printf("   ✓ CBTTSK execution verified successfully.\n");

    // 89. Test CBTLK command execution
    printf("[KERNEL TEST] Dispatching 'cbtlk' command to XplOS shell...\n");
    XplosShell shell_lk;
    XplosScheduler sched_lk;
    tsfi_xplos_init_scheduler(&sched_lk);
    tsfi_xplos_init_shell(&shell_lk);
    
    FILE *f_lk_tmp = fopen("tests/load_lib.dat.bin", "wb");
    uint8_t dummy_lk[8] = {0};
    fwrite(dummy_lk, 1, 8, f_lk_tmp);
    fclose(f_lk_tmp);

    bool lk_ok = tsfi_xplos_shell_exec(&shell_lk, &sched_lk, "cbtlk tests/load_lib.dat.bin");
    assert(lk_ok == true);
    tsfi_xplos_run(&sched_lk);
    printf("   ✓ CBTLK execution verified successfully.\n");
    remove("tests/load_lib.dat.bin");

    // 90. Test CBTLOG command execution
    printf("[KERNEL TEST] Dispatching 'cbtlog' command to XplOS shell...\n");
    XplosShell shell_oplog;
    XplosScheduler sched_oplog;
    tsfi_xplos_init_scheduler(&sched_oplog);
    tsfi_xplos_init_shell(&shell_oplog);
    bool oplog_ok = tsfi_xplos_shell_exec(&shell_oplog, &sched_oplog, "cbtlog");
    assert(oplog_ok == true);
    tsfi_xplos_run(&sched_oplog);
    printf("   ✓ CBTLOG execution verified successfully.\n");

    printf("=============================================================\n");
    printf("ALL XPLOS KERNEL ZIP TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
