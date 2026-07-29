#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_xplos_kernel.h"
#include "tsfi_xplos_shell_cbt_extra.h"

int main(void) {
    printf("====================================================================\n");
    printf("        TSFi XplOS CBT TAPE ADDITIONAL FEATURES TEST SUITE           \n");
    printf("====================================================================\n");

    // Initialize Scheduler and VFS
    XplosScheduler sched;
    tsfi_xplos_init_scheduler(&sched);
    
    extern XplosVirtualDisk g_vfs;
    extern bool g_vfs_initialized;
    tsfi_xplos_init_vfs(&g_vfs);
    g_vfs_initialized = true;

    XplosShell shell;
    tsfi_xplos_init_shell(&shell);

    // 1. Test Usenet-over-SNA (cbtnet)
    printf("[TEST] Testing Usenet-over-SNA status command...\n");
    bool status_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtnet status");
    assert(status_ok == true);
    tsfi_xplos_run(&sched);

    printf("[TEST] Testing Usenet-over-SNA post command...\n");
    bool post_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtnet post net.books | Sally | Sprite Graphics | VIC-II registers");
    assert(post_ok == true);
    tsfi_xplos_run(&sched);

    printf("[TEST] Testing Usenet-over-SNA read command...\n");
    bool read_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtnet read net.books");
    assert(read_ok == true);
    tsfi_xplos_run(&sched);

    // 2. Test REXX Script Interpreter (cbtrexx)
    printf("[TEST] Creating mock REXX script in VFS...\n");
    assert(tsfi_xplos_create_file(&g_vfs, "TESTREP.dat.bin", 1024) == true);
    XplosFile *rexx_file = &g_vfs.files[g_vfs.count - 1];
    strcpy(rexx_file->data,
           "/* REXX */\n"
           "SAY \"Running dynamic report\"\n"
           "MYVAR = \"SUCCESS\"\n"
           "SAY MYVAR\n"
           "EXIT\n");
    rexx_file->size_bytes = (uint32_t)strlen(rexx_file->data);

    printf("[TEST] Executing REXX script from VFS...\n");
    bool rexx_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtrexx TESTREP my_args");
    assert(rexx_ok == true);
    tsfi_xplos_run(&sched);

    // 3. Test JCL running with step conditional bypass (jclrun)
    printf("[TEST] Creating mock JCL member in VFS...\n");
    assert(tsfi_xplos_create_file(&g_vfs, "RUNJOB.dat.bin", 2048) == true);
    XplosFile *jcl_file = &g_vfs.files[g_vfs.count - 1];
    strcpy(jcl_file->data,
           "//RUNJOB JOB 'CBT TEST',CLASS=A\n"
           "//STEP1 EXEC PGM=IEBCOPY\n"
           "//SYSUT1 DD DSN=CBT.V510.FILE002,DISP=SHR\n"
           "// IF (RC = 0) THEN\n"
           "//STEP2 EXEC PGM=IBHDRPLY\n"
           "// ENDIF\n");
    jcl_file->size_bytes = (uint32_t)strlen(jcl_file->data);

    printf("[TEST] Executing mock JCL stream...\n");
    bool jcl_ok = tsfi_xplos_shell_exec(&shell, &sched, "jclrun RUNJOB");
    assert(jcl_ok == true);
    tsfi_xplos_run(&sched);

    // 4. Test ISPF Menu Options
    printf("[TEST] Querying ISPF primary options...\n");
    bool ispf_opt0 = tsfi_xplos_shell_exec(&shell, &sched, "ispfmenu 0");
    bool ispf_opt1 = tsfi_xplos_shell_exec(&shell, &sched, "ispfmenu 1");
    bool ispf_opts = tsfi_xplos_shell_exec(&shell, &sched, "ispfmenu s");
    assert(ispf_opt0 == true);
    assert(ispf_opt1 == true);
    assert(ispf_opts == true);
    tsfi_xplos_run(&sched);

    // 5. Test IEBUPDTE Emulation
    printf("[TEST] Creating SYSIN member for IEBUPDTE...\n");
    assert(tsfi_xplos_create_file(&g_vfs, "UPDIN.dat.bin", 2048) == true);
    XplosFile *upd_file = &g_vfs.files[g_vfs.count - 1];
    strcpy(upd_file->data,
           "./ ADD NAME=NEWMEM\n"
           "SAY \"Compiled via iebupdte\"\n"
           "./ ADD NAME=NEWMEM2\n"
           "SAY \"Second member\"\n");
    upd_file->size_bytes = (uint32_t)strlen(upd_file->data);

    printf("[TEST] Running IEBUPDTE...\n");
    bool ieb_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebupdte UPDIN");
    assert(ieb_ok == true);
    tsfi_xplos_run(&sched);

    // 6. Test SUBMIT JCL
    printf("[TEST] Creating JCL member for SUBMIT...\n");
    assert(tsfi_xplos_create_file(&g_vfs, "SUBJOB.dat.bin", 2048) == true);
    XplosFile *sub_file = &g_vfs.files[g_vfs.count - 1];
    strcpy(sub_file->data,
           "//SUBJOB JOB 'SUBMIT TEST',CLASS=A\n"
           "//STEP1 EXEC PGM=IEBCOPY\n");
    sub_file->size_bytes = (uint32_t)strlen(sub_file->data);

    printf("[TEST] Submitting JCL job...\n");
    bool sub_ok = tsfi_xplos_shell_exec(&shell, &sched, "submit SUBJOB");
    assert(sub_ok == true);
    tsfi_xplos_run(&sched);

    // 7. Test SMF Dump
    printf("[TEST] Running SMFDUMP...\n");
    bool smf_ok = tsfi_xplos_shell_exec(&shell, &sched, "smfdump");
    assert(smf_ok == true);
    tsfi_xplos_run(&sched);

    // 8. Test VTAM Logon Virtualization
    printf("[TEST] Logging on to CICS and USENET application sessions...\n");
    bool logon_cics = tsfi_xplos_shell_exec(&shell, &sched, "logon APPLID(cics)");
    assert(logon_cics == true);
    tsfi_xplos_run(&sched);

    bool logon_use = tsfi_xplos_shell_exec(&shell, &sched, "logon APPLID(usenet)");
    assert(logon_use == true);
    tsfi_xplos_run(&sched);

    // 9. Test REXX Shared Variable Pool
    printf("[TEST] Checking REXX Shared Variable Pool (vput/vget)...\n");
    bool vput_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtrexx vput SYSVAR 953467954114363");
    assert(vput_ok == true);
    tsfi_xplos_run(&sched);

    bool vget_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtrexx vget SYSVAR");
    assert(vget_ok == true);
    tsfi_xplos_run(&sched);

    // 10. Test IEBGENER Sequential Copy
    printf("[TEST] Creating source dataset for IEBGENER...\n");
    assert(tsfi_xplos_create_file(&g_vfs, "GENSRC.dat.bin", 1024) == true);
    XplosFile *gen_src = &g_vfs.files[g_vfs.count - 1];
    strcpy(gen_src->data, "IEBGENER sequential data line 1\nIEBGENER sequential data line 2\n");
    gen_src->size_bytes = (uint32_t)strlen(gen_src->data);

    printf("[TEST] Running IEBGENER copy...\n");
    bool gen_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebgener GENSRC GENTGT E2A");
    assert(gen_ok == true);
    tsfi_xplos_run(&sched);

    // 11. Test TSO HELP Guide
    printf("[TEST] Running TSO HELP...\n");
    bool help_ok = tsfi_xplos_shell_exec(&shell, &sched, "help submit");
    assert(help_ok == true);
    tsfi_xplos_run(&sched);

    // 12. Test HASP Spool Log reader, Priorities, Hold/Release, and Purge
    printf("[TEST] Testing HASP Hold, Release, Priority, and Purge spool cycles...\n");
    
    // Create JCL file with TYPRUN=HOLD and PRTY=12
    assert(tsfi_xplos_create_file(&g_vfs, "HLDJOB.dat.bin", 2048) == true);
    XplosFile *hld_file = &g_vfs.files[g_vfs.count - 1];
    strcpy(hld_file->data,
           "//HLDJOB JOB 'HOLD TEST',CLASS=A,TYPRUN=HOLD,PRTY=12\n"
           "//STEP1 EXEC PGM=IEBCOPY\n");
    hld_file->size_bytes = (uint32_t)strlen(hld_file->data);

    // Submit held job
    bool sub_hld = tsfi_xplos_shell_exec(&shell, &sched, "submit HLDJOB");
    assert(sub_hld == true);
    tsfi_xplos_run(&sched);

    // Verify it is HELD
    bool hasp_stat1 = tsfi_xplos_shell_exec(&shell, &sched, "cbthasp status");
    assert(hasp_stat1 == true);
    tsfi_xplos_run(&sched);

    // Release it to dispatch by priority
    bool hasp_rel = tsfi_xplos_shell_exec(&shell, &sched, "cbthasp release JOB0104");
    assert(hasp_rel == true);
    tsfi_xplos_run(&sched);

    // Trigger interrupt
    bool hasp_int = tsfi_xplos_shell_exec(&shell, &sched, "cbthasp interrupt");
    assert(hasp_int == true);
    tsfi_xplos_run(&sched);

    // Purge job
    bool hasp_prg = tsfi_xplos_shell_exec(&shell, &sched, "cbthasp purge JOB0104");
    assert(hasp_prg == true);
    tsfi_xplos_run(&sched);

    // 13. Test CICS Transient Data Queue (TDQ) Stream Injection
    printf("[TEST] Testing CICS Transient Data Queue (TDQ) stream injection...\n");
    bool tdq_w1 = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicstd write SBM1 //TDQJOB JOB 'CICS SUBMIT',CLASS=A");
    assert(tdq_w1 == true);
    tsfi_xplos_run(&sched);

    bool tdq_w2 = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicstd write SBM1 //STEP1 EXEC PGM=IEBCOPY");
    assert(tdq_w2 == true);
    tsfi_xplos_run(&sched);

    bool tdq_sbm = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicstd sbmj SBM1");
    assert(tdq_sbm == true);
    tsfi_xplos_run(&sched);

    // 14. Test IEBCOMPR Dataset Comparison
    printf("[TEST] Testing IEBCOMPR Dataset comparison...\n");
    bool comp_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebcompr GENSRC GENTGT");
    assert(comp_ok == true);
    tsfi_xplos_run(&sched);

    // 15. Test VTAM 3270 Data Stream Formatter
    printf("[TEST] Testing VTAM 3270 Terminal Screen buffer...\n");
    bool v3270_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtam3270 CICS");
    assert(v3270_ok == true);
    tsfi_xplos_run(&sched);

    // 16. Test IKJEFT01 Terminal Monitor Program (TMP) execution via JCL
    printf("[TEST] Testing IKJEFT01 execution via JCL step...\n");
    assert(tsfi_xplos_create_file(&g_vfs, "IKJJOB.dat.bin", 2048) == true);
    XplosFile *ikj_file = &g_vfs.files[g_vfs.count - 1];
    strcpy(ikj_file->data,
           "//IKJJOB JOB 'TSO TMP TEST',CLASS=A\n"
           "//STEP1 EXEC PGM=IKJEFT01\n"
           "//SYSTSIN DD *\n"
           "cbtrexx vput SYSVAR 953467954114363\n"
           "/*\n");
    ikj_file->size_bytes = (uint32_t)strlen(ikj_file->data);

    bool ikj_run = tsfi_xplos_shell_exec(&shell, &sched, "jclrun IKJJOB");
    assert(ikj_run == true);
    tsfi_xplos_run(&sched);

    // 17. Test IEFBR14 dummy program step
    printf("[TEST] Testing IEFBR14 execution via JCL step...\n");
    assert(tsfi_xplos_create_file(&g_vfs, "BR14JOB.dat.bin", 2048) == true);
    XplosFile *br_file = &g_vfs.files[g_vfs.count - 1];
    strcpy(br_file->data,
           "//BR14JOB JOB 'DUMMY TEST',CLASS=A\n"
           "//STEP1 EXEC PGM=IEFBR14\n"
           "//DD1 DD DSN=TEMP.DAT,DISP=(NEW,DELETE)\n");
    br_file->size_bytes = (uint32_t)strlen(br_file->data);

    bool br_run = tsfi_xplos_shell_exec(&shell, &sched, "jclrun BR14JOB");
    assert(br_run == true);
    tsfi_xplos_run(&sched);

    // 18. Test TSO dynamic dataset allocations (cbtalloc)
    printf("[TEST] Testing TSO dynamic allocations...\n");
    bool alloc_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtalloc alloc DD1 MYDATA.DAT");
    bool free_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtalloc free DD1");
    assert(alloc_ok == true);
    assert(free_ok == true);
    tsfi_xplos_run(&sched);

    // 19. Test CICS Transient Storage Queues (TSQ)
    printf("[TEST] Testing CICS TSQ read/write...\n");
    bool tsq_w = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicsts write Q1 UserDataInfo");
    bool tsq_r = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicsts read Q1");
    assert(tsq_w == true);
    assert(tsq_r == true);
    tsfi_xplos_run(&sched);

    // 20. Test VTAM Net/USS Logon Solicit Screen
    printf("[TEST] Testing VTAM USS Logon Solicit screen...\n");
    bool uss_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamuss");
    assert(uss_ok == true);
    tsfi_xplos_run(&sched);

    // 21. Test TSO delete and rename
    printf("[TEST] Testing TSO cbtdelete operations...\n");
    bool del_rn = tsfi_xplos_shell_exec(&shell, &sched, "cbtdelete rename Q1 Q2");
    bool del_dl = tsfi_xplos_shell_exec(&shell, &sched, "cbtdelete delete Q2");
    assert(del_rn == true);
    assert(del_dl == true);
    tsfi_xplos_run(&sched);

    // 22. Test IEBDG test data generator
    printf("[TEST] Testing IEBDG dataset generator...\n");
    bool dg_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebdg DGDATA SEQ");
    assert(dg_ok == true);
    tsfi_xplos_run(&sched);

    // 23. Test CICS Terminal Input receive
    printf("[TEST] Testing CICS RECEIVE command...\n");
    bool trm_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicstrm receive TSO_PROMPT");
    assert(trm_ok == true);
    tsfi_xplos_run(&sched);

    // 24. Test VTAM Session Statistics analyzer
    printf("[TEST] Testing VTAM network statistics...\n");
    bool stat_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamstat");
    assert(stat_ok == true);
    tsfi_xplos_run(&sched);

    printf("\n=== ALL CBT TAPE EXTRA FEATURE TESTS PASSED ===\n");
    return 0;
}
