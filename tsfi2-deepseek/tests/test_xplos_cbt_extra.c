#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_xplos_kernel.h"
#include "tsfi_xplos_shell_cbt_jes.h"
#include "tsfi_xplos_shell_cbt_jcl.h"
#include "tsfi_xplos_shell_cbt_vtam.h"
#include "tsfi_xplos_shell_cbt_cics.h"
#include "tsfi_xplos_shell_cbt_tso.h"

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

    // 25. Test TSO JCL Syntax Checker (cbtjclchk)
    printf("[TEST] Testing TSO JCL syntax checker...\n");
    // Setup a mock JCL file in VFS
    assert(tsfi_xplos_create_file(&g_vfs, "CHKJOB.dat.bin", 2048) == true);
    XplosFile *chk_file = &g_vfs.files[g_vfs.count - 1];
    strcpy(chk_file->data,
           "//CHKJOB JOB 'CHECK',CLASS=A\n"
           "//STEP1 EXEC PGM=IEFBR14\n");
    chk_file->size_bytes = (uint32_t)strlen(chk_file->data);

    bool jclchk_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtjclchk CHKJOB");
    assert(jclchk_ok == true);
    tsfi_xplos_run(&sched);

    // 26. Test IEBIMAGE printer buffer formatter
    printf("[TEST] Testing IEBIMAGE printer formatter...\n");
    bool img_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebimage PRT01 10");
    assert(img_ok == true);
    tsfi_xplos_run(&sched);

    // 27. Test CICS Transient Storage directory explorer
    printf("[TEST] Testing CICS TSQ directory explorer...\n");
    bool tsd_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicstsd");
    assert(tsd_ok == true);
    tsfi_xplos_run(&sched);

    // 28. Test VTAM Virtual Route Controller
    printf("[TEST] Testing VTAM path tunnel routing controller...\n");
    bool route_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamroute");
    assert(route_ok == true);
    tsfi_xplos_run(&sched);

    // 29. Test IEBDATR Data Transmission format converter
    printf("[TEST] Testing IEBDATR data transmission converter...\n");
    bool datr_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebdatr SEQ1 SEQOUT");
    assert(datr_ok == true);
    tsfi_xplos_run(&sched);

    // 30. Test TSO Submission Checker (cbtsubchk)
    printf("[TEST] Testing cbtsubchk parameters scanner...\n");
    bool subchk_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsubchk CHKJOB");
    assert(subchk_ok == true);
    tsfi_xplos_run(&sched);

    // 31. Test CICS Transient Storage Compression (cbtcicscmp)
    printf("[TEST] Testing CICS TSQ compression compaction...\n");
    bool cmp_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicscmp LOGS");
    assert(cmp_ok == true);
    tsfi_xplos_run(&sched);

    // 32. Test VTAM Buffer Frame Tracer (vtambuf)
    printf("[TEST] Testing VTAM network buffer tracer...\n");
    bool buf_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtambuf");
    assert(buf_ok == true);
    tsfi_xplos_run(&sched);

    // 33. Test IEBISAM Indexed Sequential dataset converter
    printf("[TEST] Testing IEBISAM indexed sequential converter...\n");
    bool isam_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebisam SEQ1 VSAM1");
    assert(isam_ok == true);
    tsfi_xplos_run(&sched);

    // 34. Test TSO Job spool monitor (cbtsublist)
    printf("[TEST] Testing cbtsublist spool tracker...\n");
    bool sublist_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsublist");
    assert(sublist_ok == true);
    tsfi_xplos_run(&sched);

    // 35. Test CICS Transient Storage Purge (cbtcicspurge)
    printf("[TEST] Testing CICS TSQ purge allocator...\n");
    bool purge_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicspurge LOGS");
    assert(purge_ok == true);
    tsfi_xplos_run(&sched);

    // 36. Test VTAM Route Hop Tracer (vtamtrace)
    printf("[TEST] Testing VTAM path routing trace...\n");
    bool trace_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamtrace");
    assert(trace_ok == true);
    tsfi_xplos_run(&sched);

    // 37. Test IEBDGPAT Alphanumeric pattern generator
    printf("[TEST] Testing IEBDGPAT alphanumeric pattern generator...\n");
    bool dgpat_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebdgpat SEQ1 AAAA-9999");
    assert(dgpat_ok == true);
    tsfi_xplos_run(&sched);

    // 38. Test TSO submission release spool (cbtsubrel)
    printf("[TEST] Testing cbtsubrel spool release...\n");
    bool subrel_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsubrel JOB00023");
    assert(subrel_ok == true);
    tsfi_xplos_run(&sched);

    // 39. Test CICS Queue limits reader (cbtcicslim)
    printf("[TEST] Testing CICS TSQ limits explorer...\n");
    bool lim_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicslim LOGS");
    assert(lim_ok == true);
    tsfi_xplos_run(&sched);

    // 40. Test VTAM Solicit panel refresher (vtamrefr)
    printf("[TEST] Testing VTAM solicit refresh controller...\n");
    bool refr_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamrefr");
    assert(refr_ok == true);
    tsfi_xplos_run(&sched);

    // 41. Test IEBCOMPRLIM comparison threshold config
    printf("[TEST] Testing IEBCOMPRLIM threshold limit configuration...\n");
    bool comprlim_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebcomprlim 5");
    assert(comprlim_ok == true);
    tsfi_xplos_run(&sched);

    // 42. Test TSO spool output redirector (cbtsubout)
    printf("[TEST] Testing cbtsubout spool output redirector...\n");
    bool subout_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsubout JOB00021 OUTSEQ");
    assert(subout_ok == true);
    tsfi_xplos_run(&sched);

    // 43. Test CICS transaction execution trace list (cbtcicstrc)
    printf("[TEST] Testing CICS transaction history tracer...\n");
    bool trc_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicstrc");
    assert(trc_ok == true);
    tsfi_xplos_run(&sched);

    // 44. Test VTAM real-time buffer tracing toggle (vtamtracebuf)
    printf("[TEST] Testing VTAM real-time buffer trace toggle...\n");
    bool tracebuf_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamtracebuf on");
    assert(tracebuf_ok == true);
    tsfi_xplos_run(&sched);

    // 45. Test IEBUPDTESTSRCH PDS update search
    printf("[TEST] Testing IEBUPDTESTSRCH member update deck scanner...\n");
    bool updtesrch_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebupdtesrch MOCKMEM UPDIN");
    assert(updtesrch_ok == true);
    tsfi_xplos_run(&sched);

    // 46. Test TSO submission spool trace step monitor (cbtsubtrc)
    printf("[TEST] Testing cbtsubtrc spool step tracer...\n");
    bool subtrc_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsubtrc JOB00021");
    assert(subtrc_ok == true);
    tsfi_xplos_run(&sched);

    // 47. Test CICS transaction trace filter (cbtcicstrcflt)
    printf("[TEST] Testing CICS transaction history trace filter...\n");
    bool trcflt_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicstrcflt SBMJ");
    assert(trcflt_ok == true);
    tsfi_xplos_run(&sched);

    // 48. Test VTAM buffer detailed hex trace dumps (vtambufdet)
    printf("[TEST] Testing VTAM buffer detailed hex tracer...\n");
    bool bufdet_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtambufdet FRM00241");
    assert(bufdet_ok == true);
    tsfi_xplos_run(&sched);

    // 49. Test IEBDGSEED custom data seed generator
    printf("[TEST] Testing IEBDGSEED sequential data generator seed...\n");
    bool dgseed_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebdgseed 953467954114363");
    assert(dgseed_ok == true);
    tsfi_xplos_run(&sched);

    // 50. Test TSO spool job scheduling priority updater (cbtsubpri)
    printf("[TEST] Testing cbtsubpri spool Priority modifier...\n");
    bool subpri_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsubpri JOB00021 15");
    assert(subpri_ok == true);
    tsfi_xplos_run(&sched);

    // 51. Test CICS Directory clean allocator (cbtcicsclean)
    printf("[TEST] Testing CICS TSQ directory storage cleaner...\n");
    bool clean_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicsclean");
    assert(clean_ok == true);
    tsfi_xplos_run(&sched);

    // 52. Test VTAM USS logon Solicit terminal monitor (vtamussstat)
    printf("[TEST] Testing VTAM USS Logon Solicit monitor...\n");
    bool ussstat_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamussstat");
    assert(ussstat_ok == true);
    tsfi_xplos_run(&sched);

    // 53. Test IEBCOMPRMASK character mask compare filter
    printf("[TEST] Testing IEBCOMPRMASK character mask comparison filter...\n");
    bool comprmask_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebcomprmask XXXXXX");
    assert(comprmask_ok == true);
    tsfi_xplos_run(&sched);

    // 54. Test TSO spool job purge cleanup action (cbtsubprg)
    printf("[TEST] Testing cbtsubprg spool purge...\n");
    bool subprg_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsubprg JOB00021");
    assert(subprg_ok == true);
    tsfi_xplos_run(&sched);

    // 55. Test CICS Queue compression toggle (cbtcicscmpflg)
    printf("[TEST] Testing CICS TSQ compaction auto toggle...\n");
    bool cmpflg_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicscmpflg on");
    assert(cmpflg_ok == true);
    tsfi_xplos_run(&sched);

    // 56. Test VTAM USS Solicit display format switch (vtamussfmt)
    printf("[TEST] Testing VTAM USS logo Solicit panel layout formatter...\n");
    bool ussfmt_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamussfmt DETAILED");
    assert(ussfmt_ok == true);
    tsfi_xplos_run(&sched);

    // 57. Test IEBIMAGESPC print vertical spacing (iebimagespc)
    printf("[TEST] Testing IEBIMAGESPC spacing formatter...\n");
    bool imagespc_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebimagespc 8");
    assert(imagespc_ok == true);
    tsfi_xplos_run(&sched);

    // 58. Test TSO spool job search (cbtsubfind)
    printf("[TEST] Testing cbtsubfind spool scanner...\n");
    bool subfind_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsubfind JOB00021 SUCCESS");
    assert(subfind_ok == true);
    tsfi_xplos_run(&sched);

    // 59. Test CICS wildcard Transient Storage Queue purger (cbtcicscleanmask)
    printf("[TEST] Testing CICS TSQ prefix purger...\n");
    bool cleanmask_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicscleanmask LOG*");
    assert(cleanmask_ok == true);
    tsfi_xplos_run(&sched);

    // 60. Test VTAM USS Solicit path delay simulator (vtamussdelay)
    printf("[TEST] Testing VTAM USS path transmission delay...\n");
    bool ussdelay_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamussdelay 120");
    assert(ussdelay_ok == true);
    tsfi_xplos_run(&sched);

    // 61. Test IEBDATRCHK record format verification checker
    printf("[TEST] Testing IEBDATRCHK record format checker...\n");
    bool datrchk_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebdatrchk OUTSEQ");
    assert(datrchk_ok == true);
    tsfi_xplos_run(&sched);

    // 62. Test TSO spool job notify alert toggle (cbtsubnot)
    printf("[TEST] Testing cbtsubnot spool Alert toggle...\n");
    bool subnot_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsubnot on");
    assert(subnot_ok == true);
    tsfi_xplos_run(&sched);

    // 63. Test CICS Queue details explorer (cbtcicsdet)
    printf("[TEST] Testing CICS TSQ detailed explorer...\n");
    bool cicsdet_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicsdet Q1");
    assert(cicsdet_ok == true);
    tsfi_xplos_run(&sched);

    // 64. Test VTAM Route stability control (vtamroutesol)
    printf("[TEST] Testing VTAM path routing stability controller...\n");
    bool routesol_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamroutesol 95");
    assert(routesol_ok == true);
    tsfi_xplos_run(&sched);

    // 65. Test IEBCOMPRDET detailed record comparison (iebcomprdet)
    printf("[TEST] Testing IEBCOMPRDET detailed comparison reporter...\n");
    bool comprdet_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebcomprdet GENSRC GENTGT");
    assert(comprdet_ok == true);
    tsfi_xplos_run(&sched);

    // 66. Test TSO spool job Class modifier (cbtsubmod)
    printf("[TEST] Testing cbtsubmod spool parameters modifier...\n");
    bool submod_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsubmod JOB00021 B");
    assert(submod_ok == true);
    tsfi_xplos_run(&sched);

    // 67. Test CICS clean operations log details (cbtcicscleandet)
    printf("[TEST] Testing CICS TSQ cleanup operations details...\n");
    bool cleandet_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicscleandet");
    assert(cleandet_ok == true);
    tsfi_xplos_run(&sched);

    // 68. Test VTAM Route failure simulator (vtamroutefail)
    printf("[TEST] Testing VTAM path gateway failure simulator...\n");
    bool routefail_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamroutefail LU_REMOTE");
    assert(routefail_ok == true);
    tsfi_xplos_run(&sched);

    // 69. Test IEBDG sequential patterns detail explorer (iebdgpatdet)
    printf("[TEST] Testing iebdgpatdet pattern details explorer...\n");
    bool patdet_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebdgpatdet SEQ1");
    assert(patdet_ok == true);
    tsfi_xplos_run(&sched);

    // 70. Test TSO spool output lines limiter (cbtsuboutlim)
    printf("[TEST] Testing cbtsuboutlim spool redirect line limiter...\n");
    bool outlim_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsuboutlim JOB00021 1000");
    assert(outlim_ok == true);
    tsfi_xplos_run(&sched);

    // 71. Test CICS clean operations filter details (cbtcicscleanflt)
    printf("[TEST] Testing CICS TSQ cleanup log prefix filter...\n");
    bool cleanflt_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicscleanflt Q");
    assert(cleanflt_ok == true);
    tsfi_xplos_run(&sched);

    // 72. Test VTAM USS terminal Greeting welcome customizer (vtamussmsg)
    printf("[TEST] Testing VTAM USS terminal welcome greeting message...\n");
    bool ussmsg_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamussmsg WELCOME");
    assert(ussmsg_ok == true);
    tsfi_xplos_run(&sched);

    // 73. Test IEBGENER Copy Buffer Capacity config (iebgenerbuf)
    printf("[TEST] Testing IEBGENER copy buffer sizing...\n");
    bool generbuf_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebgenerbuf 4096");
    assert(generbuf_ok == true);
    tsfi_xplos_run(&sched);

    // 74. Test TSO spool job wait timeout (cbtsubwait)
    printf("[TEST] Testing cbtsubwait spool dispatch timeout...\n");
    bool subwait_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsubwait 30");
    assert(subwait_ok == true);
    tsfi_xplos_run(&sched);

    // 75. Test CICS transient storage compaction details explorer (cbtcicscmpdet)
    printf("[TEST] Testing CICS TSQ compaction details query...\n");
    bool cmpdet_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicscmpdet");
    assert(cmpdet_ok == true);
    tsfi_xplos_run(&sched);

    // 76. Test VTAM Route recovery retry scheduler (vtamrouterec)
    printf("[TEST] Testing VTAM gateway route recovery scheduler...\n");
    bool routerec_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamrouterec 60");
    assert(routerec_ok == true);
    tsfi_xplos_run(&sched);

    // 77. Test IEBCOMPR output redirection helper (iebcomprredirect)
    printf("[TEST] Testing IEBCOMPR redirect output to PDS member...\n");
    bool comprredirect_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebcomprredirect REPORT1");
    assert(comprredirect_ok == true);
    tsfi_xplos_run(&sched);

    // 78. Test TSO spool job sync wait telemetry monitor (cbtsubwaitstat)
    printf("[TEST] Testing cbtsubwaitstat spool sync timer status...\n");
    bool waitstat_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsubwaitstat");
    assert(waitstat_ok == true);
    tsfi_xplos_run(&sched);

    // 79. Test CICS clean exceptions rule register (cbtcicscleanexcept)
    printf("[TEST] Testing CICS TSQ cleanup exclusion prefix filter...\n");
    bool cleanexcept_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicscleanexcept SYS*");
    assert(cleanexcept_ok == true);
    tsfi_xplos_run(&sched);

    // 80. Test VTAM Route failure scheduler monitor (vtamrouterecstat)
    printf("[TEST] Testing VTAM gateway route recovery telemetry status...\n");
    bool routerecstat_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamrouterecstat");
    assert(routerecstat_ok == true);
    tsfi_xplos_run(&sched);

    // 81. Test IEBCOMPR maximum mismatch exclusions limit config (iebcomprmaxexcl)
    printf("[TEST] Testing IEBCOMPR maximum exclusions limit config...\n");
    bool comprmaxexcl_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebcomprmaxexcl 100");
    assert(comprmaxexcl_ok == true);
    tsfi_xplos_run(&sched);

    // 82. Test TSO spool wait retry action configuration (cbtsubwaitretry)
    printf("[TEST] Testing cbtsubwaitretry spool sync retry toggle...\n");
    bool subwaitretry_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsubwaitretry on");
    assert(subwaitretry_ok == true);
    tsfi_xplos_run(&sched);

    // 83. Test CICS transient storage clean history log (cbtcicscleanlog)
    printf("[TEST] Testing CICS TSQ cleanup history logger...\n");
    bool cleanlog_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicscleanlog");
    assert(cleanlog_ok == true);
    tsfi_xplos_run(&sched);

    // 84. Test VTAM Route failure retry boundaries scheduler (vtamroutereccount)
    printf("[TEST] Testing VTAM gateway route recovery retry limits...\n");
    bool routereccount_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamroutereccount 5");
    assert(routereccount_ok == true);
    tsfi_xplos_run(&sched);

    // 85. Test IEBDG sequence generator iteration limit config (iebdgiterlim)
    printf("[TEST] Testing IEBDG maximum generator iterations limit...\n");
    bool iterlim_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebdgiterlim 5000");
    assert(iterlim_ok == true);
    tsfi_xplos_run(&sched);

    // 86. Test TSO job spool wait completion retry delay timer (cbtsubwaitretrydelay)
    printf("[TEST] Testing cbtsubwaitretrydelay retry schedule delay...\n");
    bool retrydelay_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsubwaitretrydelay 10");
    assert(retrydelay_ok == true);
    tsfi_xplos_run(&sched);

    // 87. Test CICS transient storage cleanup exclusion prefix filter list (cbtcicscleanexceptlist)
    printf("[TEST] Testing CICS TSQ cleanup exclusion rules list query...\n");
    bool exceptlist_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicscleanexceptlist");
    assert(exceptlist_ok == true);
    tsfi_xplos_run(&sched);

    // 88. Test VTAM Route failure scheduler counters reset (vtamrouterecreset)
    printf("[TEST] Testing VTAM gateway route recovery telemetry metrics reset...\n");
    bool routerecreset_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamrouterecreset");
    assert(routerecreset_ok == true);
    tsfi_xplos_run(&sched);

    // 89. Test IEBDG sequence generator iteration status query (iebdgiterstat)
    printf("[TEST] Testing IEBDG generator iteration telemetry status...\n");
    bool iterstat_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebdgiterstat");
    assert(iterstat_ok == true);
    tsfi_xplos_run(&sched);

    // 90. Test TSO spool job sync wait timeout recovery retry limits configuration (cbtsubwaitretrylimit)
    printf("[TEST] Testing cbtsubwaitretrylimit retry limits bounds...\n");
    bool retrylimit_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsubwaitretrylimit 3");
    assert(retrylimit_ok == true);
    tsfi_xplos_run(&sched);

    // 91. Test CICS transient storage clean exclusions rule match audit (cbtcicscleanexceptchk)
    printf("[TEST] Testing CICS TSQ cleanup exclusion matching check...\n");
    bool cleanexceptchk_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicscleanexceptchk SYS123");
    assert(cleanexceptchk_ok == true);
    tsfi_xplos_run(&sched);

    // 92. Test VTAM Route failure scheduler query (vtamrouterecquery)
    printf("[TEST] Testing VTAM gateway route recovery telemetry status query...\n");
    bool routerecquery_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamrouterecquery LU_REMOTE");
    assert(routerecquery_ok == true);
    tsfi_xplos_run(&sched);

    // 93. Test IEBDG dynamic sequence rule changer (iebdgiterchange)
    printf("[TEST] Testing IEBDG generator rule changer threshold...\n");
    bool iterchange_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebdgiterchange 2000 SHIFT1");
    assert(iterchange_ok == true);
    tsfi_xplos_run(&sched);

    // 94. Test TSO spool job sync wait timeout custom handler command (cbtsubwaitretryscript)
    printf("[TEST] Testing cbtsubwaitretryscript spool timeout action...\n");
    bool retryscript_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsubwaitretryscript SUBMIT2");
    assert(retryscript_ok == true);
    tsfi_xplos_run(&sched);

    // 95. Test CICS clean exclusions rule deletion (cbtcicscleanexceptdel)
    printf("[TEST] Testing CICS TSQ cleanup exclusion deletion...\n");
    bool cleanexceptdel_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicscleanexceptdel SYS*");
    assert(cleanexceptdel_ok == true);
    tsfi_xplos_run(&sched);

    // 96. Test VTAM Route failure recovery query list (vtamrouterecquerylist)
    printf("[TEST] Testing VTAM gateway route recovery list query...\n");
    bool routerecquerylist_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamrouterecquerylist");
    assert(routerecquerylist_ok == true);
    tsfi_xplos_run(&sched);

    // 97. Test CICS subsystem engine telemetry statistics (cbtcicsstats)
    printf("[TEST] Testing CICS engine statistics telemetry query...\n");
    bool cicsstats_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicsstats");
    assert(cicsstats_ok == true);
    tsfi_xplos_run(&sched);

    // 98. Test CICS screen map attribute viewer (cbtcicsmap)
    printf("[TEST] Testing CICS active map details query...\n");
    bool cicsmap_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicsmap");
    assert(cicsmap_ok == true);
    tsfi_xplos_run(&sched);

    // 99. Test CICS active/suspended task dispatcher lists (cbtcicstask)
    printf("[TEST] Testing CICS task queue console view...\n");
    bool cicstask_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicstask");
    assert(cicstask_ok == true);
    tsfi_xplos_run(&sched);

    // 100. Test CICS terminal session refresh reset (cbtcicsrefr)
    printf("[TEST] Testing CICS pseudo-conversational terminal refresh...\n");
    bool cicsrefr_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicsrefr");
    assert(cicsrefr_ok == true);
    tsfi_xplos_run(&sched);

    // 101. Test CICS PCT definitions query (cbtcicsprg)
    printf("[TEST] Testing CICS Program Control Table query...\n");
    bool cicsprg_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicsprg");
    assert(cicsprg_ok == true);
    tsfi_xplos_run(&sched);

    // 102. Test CICS TSQ write transaction (cbtcicswriteq)
    printf("[TEST] Testing CICS TSQ dynamic write...\n");
    bool cicswriteq_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicswriteq TSQ1 HELLO_CICS");
    assert(cicswriteq_ok == true);
    tsfi_xplos_run(&sched);

    // 103. Test CICS TSQ read/dump query (cbtcicsreadq)
    printf("[TEST] Testing CICS TSQ dynamic read dump...\n");
    bool cicsreadq_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicsreadq TSQ1");
    assert(cicsreadq_ok == true);
    tsfi_xplos_run(&sched);

    // 104. Test CICS Active Lock Manager query (cbtcicslock)
    printf("[TEST] Testing CICS Lock Manager query...\n");
    bool cicslock_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicslock");
    assert(cicslock_ok == true);
    tsfi_xplos_run(&sched);

    // 105. Test IEBGENER record length recalculation (iebgenerrecal)
    printf("[TEST] Testing IEBGENER dynamic LRECL recalculation...\n");
    bool generrecal_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebgenerrecal 120");
    assert(generrecal_ok == true);
    tsfi_xplos_run(&sched);

    // 106. Test TSO spool job timeout script directory query (cbtsubwaitretryscriptlist)
    printf("[TEST] Testing TSO spool timeout script list...\n");
    bool waitscriptlist_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsubwaitretryscriptlist");
    assert(waitscriptlist_ok == true);
    tsfi_xplos_run(&sched);

    // 107. Test CICS engine statistics reset (cbtcicsstatsreset)
    printf("[TEST] Testing CICS engine statistics reset...\n");
    bool statsreset_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicsstatsreset");
    assert(statsreset_ok == true);
    tsfi_xplos_run(&sched);

    // 108. Test VTAM gateway welcome greeting customizer (vtamroutesolmsg)
    printf("[TEST] Testing VTAM gateway greeting customizer...\n");
    bool routesolmsg_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamroutesolmsg HELLO");
    assert(routesolmsg_ok == true);
    tsfi_xplos_run(&sched);

    // 109. Test VTAM gateway route recovery telemetry logger (vtamroutereclog)
    printf("[TEST] Testing VTAM gateway logging destination customizer...\n");
    bool routereclog_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamroutereclog SYSLOG");
    assert(routereclog_ok == true);
    tsfi_xplos_run(&sched);

    // 110. Test TSO spool job timeout cumulative delay limiter (cbtsubwaitretrydelaylimit)
    printf("[TEST] Testing TSO spool timeout cumulative delay limiter...\n");
    bool retrydelaylimit_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsubwaitretrydelaylimit 180");
    assert(retrydelaylimit_ok == true);
    tsfi_xplos_run(&sched);

    // 111. Test IEBGENER sequential copier execution telemetry (iebgenerstat)
    printf("[TEST] Testing IEBGENER copier telemetry status...\n");
    bool generstat_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebgenerstat");
    assert(generstat_ok == true);
    tsfi_xplos_run(&sched);

    // 112. Test CICS TSQ cleanup exclusion prefixes bulk registry (cbtcicscleanexceptadd)
    printf("[TEST] Testing CICS TSQ cleanup exclusions bulk registry...\n");
    bool cleanexceptadd_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicscleanexceptadd SYS1 SYS2 SYS3");
    assert(cleanexceptadd_ok == true);
    tsfi_xplos_run(&sched);

    // 113. Test VTAM gateway route recovery telemetry error ignorer (vtamrouterecignore)
    printf("[TEST] Testing VTAM gateway telemetry error ignorer...\n");
    bool routerecignore_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamrouterecignore LU_REMOTE");
    assert(routerecignore_ok == true);
    tsfi_xplos_run(&sched);

    // 114. Test TSO spool job timeout script deletion (cbtsubwaitretryscriptdel)
    printf("[TEST] Testing TSO spool timeout script deletion...\n");
    bool retryscriptdel_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsubwaitretryscriptdel SUBMIT2");
    assert(retryscriptdel_ok == true);
    tsfi_xplos_run(&sched);

    // 115. Test IEBGENER sequential layout translation table refresher (iebgenerreftab)
    printf("[TEST] Testing IEBGENER translation table refresher...\n");
    bool generreftab_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebgenerreftab");
    assert(generreftab_ok == true);
    tsfi_xplos_run(&sched);

    // 116. Test CICS TSQ cleanup exclusions bulk validation (cbtcicscleanexceptchklist)
    printf("[TEST] Testing CICS TSQ exclusions bulk validation...\n");
    bool cleanexceptchklist_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicscleanexceptchklist SYS1 SYS2");
    assert(cleanexceptchklist_ok == true);
    tsfi_xplos_run(&sched);

    // 117. Test VTAM gateway route recovery telemetry statistics reset (vtamrouterecresetstat)
    printf("[TEST] Testing VTAM gateway telemetry stats reset...\n");
    bool routerecresetstat_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamrouterecresetstat");
    assert(routerecresetstat_ok == true);
    tsfi_xplos_run(&sched);

    // 118. Test TSO spool job timeout script status validator (cbtsubwaitretryscriptchk)
    printf("[TEST] Testing TSO spool timeout script status validator...\n");
    bool retryscriptchk_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsubwaitretryscriptchk SUBMIT1");
    assert(retryscriptchk_ok == true);
    tsfi_xplos_run(&sched);

    // 119. Test IEBGENER sequential layout translation table telemetry status (iebgenerreftabstat)
    printf("[TEST] Testing IEBGENER translation table telemetry status...\n");
    bool generreftabstat_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebgenerreftabstat");
    assert(generreftabstat_ok == true);
    tsfi_xplos_run(&sched);

    // 120. Test CICS TSQ cleanup exclusions bulk registry comma-separated (cbtcicscleanexceptaddlist)
    printf("[TEST] Testing CICS TSQ exclusions bulk registry comma-separated...\n");
    bool cleanexceptaddlist_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicscleanexceptaddlist SYS1,SYS2,SYS3");
    assert(cleanexceptaddlist_ok == true);
    tsfi_xplos_run(&sched);

    // 121. Test VTAM gateway route recovery telemetry statistics reset list (vtamrouterecresetlist)
    printf("[TEST] Testing VTAM gateway telemetry stats reset list...\n");
    bool routerecresetlist_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamrouterecresetlist LU_REMOTE,LU_LOCAL");
    assert(routerecresetlist_ok == true);
    tsfi_xplos_run(&sched);

    // 122. Test TSO spool job timeout script status bulk validator (cbtsubwaitretryscriptchklist)
    printf("[TEST] Testing TSO spool timeout script status bulk validator...\n");
    bool retryscriptchklist_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsubwaitretryscriptchklist SUBMIT1 SUBMIT2");
    assert(retryscriptchklist_ok == true);
    tsfi_xplos_run(&sched);

    // 123. Test IEBGENER sequential layout translation table statistics reset (iebgenerreftabstatreset)
    printf("[TEST] Testing IEBGENER translation table statistics reset...\n");
    bool generreftabstatreset_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebgenerreftabstatreset");
    assert(generreftabstatreset_ok == true);
    tsfi_xplos_run(&sched);

    // 124. Test CICS TSQ cleanup exclusions bulk validation stats (cbtcicscleanexceptchkliststat)
    printf("[TEST] Testing CICS TSQ exclusions bulk validation stats...\n");
    bool cleanexceptchkliststat_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicscleanexceptchkliststat");
    assert(cleanexceptchkliststat_ok == true);
    tsfi_xplos_run(&sched);

    // 125. Test VTAM gateway route recovery telemetry error ignorer list (vtamrouterecignorelist)
    printf("[TEST] Testing VTAM gateway telemetry error ignorer list...\n");
    bool routerecignorelist_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamrouterecignorelist LU_REMOTE,LU_LOCAL");
    assert(routerecignorelist_ok == true);
    tsfi_xplos_run(&sched);

    // 126. Test TSO spool job timeout script status bulk validation stats (cbtsubwaitretryscriptchkliststat)
    printf("[TEST] Testing TSO spool timeout script status bulk validation stats...\n");
    bool retryscriptchkliststat_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsubwaitretryscriptchkliststat");
    assert(retryscriptchkliststat_ok == true);
    tsfi_xplos_run(&sched);

    // 127. Test IEBGENER sequential layout translation table statistics selective reset (iebgenerreftabstatresetlist)
    printf("[TEST] Testing IEBGENER translation table statistics selective reset...\n");
    bool generreftabstatresetlist_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebgenerreftabstatresetlist EBCDIC,ASCII");
    assert(generreftabstatresetlist_ok == true);
    tsfi_xplos_run(&sched);

    // 128. Test CICS TSQ cleanup exclusions bulk registry stats (cbtcicscleanexceptaddliststat)
    printf("[TEST] Testing CICS TSQ exclusions bulk registry stats...\n");
    bool cleanexceptaddliststat_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicscleanexceptaddliststat");
    assert(cleanexceptaddliststat_ok == true);
    tsfi_xplos_run(&sched);

    // 129. Test VTAM gateway route recovery telemetry counts reset (vtamroutereccountreset)
    printf("[TEST] Testing VTAM gateway telemetry threshold count reset...\n");
    bool routereccountreset_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamroutereccountreset");
    assert(routereccountreset_ok == true);
    tsfi_xplos_run(&sched);

    // 130. Test TSO spool job timeout script status bulk validation reset (cbtsubwaitretryscriptchklistreset)
    printf("[TEST] Testing TSO spool timeout script status bulk validation reset...\n");
    bool retryscriptchklistreset_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsubwaitretryscriptchklistreset");
    assert(retryscriptchklistreset_ok == true);
    tsfi_xplos_run(&sched);

    // 131. Test IEBGENER sequential layout translation table statistics selective reset stats (iebgenerreftabstatresetliststat)
    printf("[TEST] Testing IEBGENER translation table statistics selective reset stats...\n");
    bool generreftabstatresetliststat_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebgenerreftabstatresetliststat");
    assert(generreftabstatresetliststat_ok == true);
    tsfi_xplos_run(&sched);

    // 132. Test CICS TSQ cleanup exclusions bulk registry integrity check (cbtcicscleanexceptaddlistchk)
    printf("[TEST] Testing CICS TSQ exclusions bulk registry integrity check...\n");
    bool cleanexceptaddlistchk_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicscleanexceptaddlistchk");
    assert(cleanexceptaddlistchk_ok == true);
    tsfi_xplos_run(&sched);

    // 133. Test VTAM gateway route recovery telemetry warning count breach status (vtamroutereccountstat)
    printf("[TEST] Testing VTAM gateway telemetry count breach status...\n");
    bool routereccountstat_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamroutereccountstat");
    assert(routereccountstat_ok == true);
    tsfi_xplos_run(&sched);

    // 134. Test TSO spool job timeout script status bulk validation reset stats (cbtsubwaitretryscriptchklistresetstat)
    printf("[TEST] Testing TSO spool timeout script status bulk validation reset stats...\n");
    bool retryscriptchklistresetstat_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsubwaitretryscriptchklistresetstat");
    assert(retryscriptchklistresetstat_ok == true);
    tsfi_xplos_run(&sched);

    // 135. Test IEBGENER sequential layout translation table statistics selective reset stats clear (iebgenerreftabstatresetliststatreset)
    printf("[TEST] Testing IEBGENER translation table statistics selective reset stats clear...\n");
    bool generreftabstatresetliststatreset_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebgenerreftabstatresetliststatreset");
    assert(generreftabstatresetliststatreset_ok == true);
    tsfi_xplos_run(&sched);

    // 136. Test CICS TSQ cleanup exclusions bulk registry clear (cbtcicscleanexceptaddlistreset)
    printf("[TEST] Testing CICS TSQ exclusions bulk registry clear...\n");
    bool cleanexceptaddlistreset_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicscleanexceptaddlistreset");
    assert(cleanexceptaddlistreset_ok == true);
    tsfi_xplos_run(&sched);

    // 137. Test VTAM gateway route recovery telemetry error limits (vtamroutereccountlimits)
    printf("[TEST] Testing VTAM gateway telemetry error limits...\n");
    bool routereccountlimits_ok1 = tsfi_xplos_shell_exec(&shell, &sched, "vtamroutereccountlimits");
    assert(routereccountlimits_ok1 == true);
    bool routereccountlimits_ok2 = tsfi_xplos_shell_exec(&shell, &sched, "vtamroutereccountlimits 15");
    assert(routereccountlimits_ok2 == true);
    tsfi_xplos_run(&sched);

    // 138. Test TSO spool job timeout script status bulk validation reset history logs (cbtsubwaitretryscriptchklistresetstats)
    printf("[TEST] Testing TSO spool timeout script status bulk validation reset logs...\n");
    bool retryscriptchklistresetstats_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsubwaitretryscriptchklistresetstats");
    assert(retryscriptchklistresetstats_ok == true);
    tsfi_xplos_run(&sched);

    // 139. Test IEBGENER sequential layout translation table statistics selective reset stats query (iebgenerreftabstatresetliststatresetstat)
    printf("[TEST] Testing IEBGENER translation table statistics selective reset stats query...\n");
    bool generreftabstatresetliststatresetstat_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebgenerreftabstatresetliststatresetstat");
    assert(generreftabstatresetliststatresetstat_ok == true);
    tsfi_xplos_run(&sched);

    // 140. Test CICS TSQ cleanup exclusions bulk registry clear stats (cbtcicscleanexceptaddlistresetstat)
    printf("[TEST] Testing CICS TSQ exclusions bulk registry clear stats...\n");
    bool cleanexceptaddlistresetstat_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicscleanexceptaddlistresetstat");
    assert(cleanexceptaddlistresetstat_ok == true);
    tsfi_xplos_run(&sched);

    // 141. Test VTAM gateway route recovery telemetry error limits reset (vtamroutereccountlimitsreset)
    printf("[TEST] Testing VTAM gateway telemetry error limits reset...\n");
    bool routereccountlimitsreset_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamroutereccountlimitsreset");
    assert(routereccountlimitsreset_ok == true);
    tsfi_xplos_run(&sched);

    // 142. Test TSO spool job timeout script status bulk validation reset history logs status (cbtsubwaitretryscriptchklistresetstatsstat)
    printf("[TEST] Testing TSO spool timeout script status bulk validation reset logs status...\n");
    bool retryscriptchklistresetstatsstat_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsubwaitretryscriptchklistresetstatsstat");
    assert(retryscriptchklistresetstatsstat_ok == true);
    tsfi_xplos_run(&sched);

    // 143. Test CICS TSQ cleanup exclusions bulk registry clear stats logs (cbtcicscleanexceptaddlistresetstats)
    printf("[TEST] Testing CICS TSQ exclusions bulk registry clear stats logs...\n");
    bool cleanexceptaddlistresetstats_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicscleanexceptaddlistresetstats");
    assert(cleanexceptaddlistresetstats_ok == true);
    tsfi_xplos_run(&sched);

    // 144. Test VTAM gateway route recovery telemetry error limits reset stats (vtamroutereccountlimitsresetstat)
    printf("[TEST] Testing VTAM gateway telemetry error limits reset stats...\n");
    bool routereccountlimitsresetstat_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamroutereccountlimitsresetstat");
    assert(routereccountlimitsresetstat_ok == true);
    tsfi_xplos_run(&sched);

    // 145. Test TSO spool job timeout script status bulk validation reset history logs status details (cbtsubwaitretryscriptchklistresetstatsstatus)
    printf("[TEST] Testing TSO spool timeout script status bulk validation reset logs status details...\n");
    bool retryscriptchklistresetstatsstatus_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsubwaitretryscriptchklistresetstatsstatus");
    assert(retryscriptchklistresetstatsstatus_ok == true);
    tsfi_xplos_run(&sched);

    // 146. Test CICS TSQ cleanup exclusions bulk registry clear stats logs status (cbtcicscleanexceptaddlistresetstatsstat)
    printf("[TEST] Testing CICS TSQ exclusions bulk registry clear stats logs status...\n");
    bool cleanexceptaddlistresetstatsstat_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicscleanexceptaddlistresetstatsstat");
    assert(cleanexceptaddlistresetstatsstat_ok == true);
    tsfi_xplos_run(&sched);

    // 147. Test VTAM gateway route recovery telemetry error limits reset stats logs (vtamroutereccountlimitsresetstats)
    printf("[TEST] Testing VTAM gateway telemetry error limits reset stats logs...\n");
    bool routereccountlimitsresetstats_ok = tsfi_xplos_shell_exec(&shell, &sched, "vtamroutereccountlimitsresetstats");
    assert(routereccountlimitsresetstats_ok == true);
    tsfi_xplos_run(&sched);

    // 148. Test TSO spool job timeout script status bulk validation reset history logs status text details (cbtsubwaitretryscriptchklistresetstatsstatusdetails)
    printf("[TEST] Testing TSO spool timeout script status bulk validation reset logs status text details...\n");
    bool retryscriptchklistresetstatsstatusdetails_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtsubwaitretryscriptchklistresetstatsstatusdetails");
    assert(retryscriptchklistresetstatsstatusdetails_ok == true);
    tsfi_xplos_run(&sched);

    // 149. Test CICS TSQ cleanup exclusions bulk registry clear stats logs status progress (cbtcicscleanexceptaddlistresetstatsstatus)
    printf("[TEST] Testing CICS TSQ exclusions bulk registry clear stats logs status progress...\n");
    bool cleanexceptaddlistresetstatsstatus_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtcicscleanexceptaddlistresetstatsstatus");
    assert(cleanexceptaddlistresetstatsstatus_ok == true);
    tsfi_xplos_run(&sched);

    // 150. Test JCL syntax validation audit checks (cbtjclvalidate)
    printf("[TEST] Testing JCL syntax validation audit...\n");
    bool jclvalidate_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtjclvalidate MY_JOB_Q");
    assert(jclvalidate_ok == true);
    tsfi_xplos_run(&sched);

    // 151. Test JCL registered partitions list query (cbtjcllist)
    printf("[TEST] Testing JCL registered partitions member list query...\n");
    bool jcllist_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtjcllist");
    assert(jcllist_ok == true);
    tsfi_xplos_run(&sched);

    // 152. Test JCL job execution telemetry resolution status (cbtjclresstat)
    printf("[TEST] Testing JCL job execution telemetry status query...\n");
    bool jclresstat_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtjclresstat");
    assert(jclresstat_ok == true);
    tsfi_xplos_run(&sched);

    // 153. Test JCL syntax validation audit history logs query (cbtjclvalidatelog)
    printf("[TEST] Testing JCL syntax validation audit history logs...\n");
    bool jclvalidatelog_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtjclvalidatelog");
    assert(jclvalidatelog_ok == true);
    tsfi_xplos_run(&sched);

    // 154. Test JCL active partition member count statistics query (cbtjclliststat)
    printf("[TEST] Testing JCL member partition statistics query...\n");
    bool jclliststat_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtjclliststat");
    assert(jclliststat_ok == true);
    tsfi_xplos_run(&sched);

    // 155. Test JCL job execution telemetry stats reset (cbtjclresstatreset)
    printf("[TEST] Testing JCL execution telemetry reset...\n");
    bool jclresstatreset_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtjclresstatreset");
    assert(jclresstatreset_ok == true);
    tsfi_xplos_run(&sched);

    // 156. Test JCL telemetry reset event statistics (cbtjclresstatresetstat)
    printf("[TEST] Testing JCL telemetry reset event stats query...\n");
    bool jclresstatresetstat_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtjclresstatresetstat");
    assert(jclresstatresetstat_ok == true);
    tsfi_xplos_run(&sched);

    // 157. Test JCL telemetry reset event history logs (cbtjclresstatresetstats)
    printf("[TEST] Testing JCL telemetry reset event history logs query...\n");
    bool jclresstatresetstats_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtjclresstatresetstats");
    assert(jclresstatresetstats_ok == true);
    tsfi_xplos_run(&sched);

    // 158. Test JCL telemetry reset integrity verification status (cbtjclresstatresetstatsstatus)
    printf("[TEST] Testing JCL telemetry reset logs integrity status query...\n");
    bool jclresstatresetstatsstatus_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtjclresstatresetstatsstatus");
    assert(jclresstatresetstatsstatus_ok == true);
    tsfi_xplos_run(&sched);

    // 159. Test JCL telemetry reset integrity verification status details (cbtjclresstatresetstatsstatusdetails)
    printf("[TEST] Testing JCL telemetry reset logs integrity status details query...\n");
    bool jclresstatresetstatsstatusdetails_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtjclresstatresetstatsstatusdetails");
    assert(jclresstatresetstatsstatusdetails_ok == true);
    tsfi_xplos_run(&sched);

    // 160. Test JCL job execution limits config (cbtjcljoblimit)
    printf("[TEST] Testing JCL job resource limit configuration...\n");
    bool jcljoblimit_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtjcljoblimit 50");
    assert(jcljoblimit_ok == true);
    tsfi_xplos_run(&sched);

    // 161. Test JCL job execution limits config stats (cbtjcljoblimitstat)
    printf("[TEST] Testing JCL job resource limit statistics query...\n");
    bool jcljoblimitstat_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtjcljoblimitstat");
    assert(jcljoblimitstat_ok == true);
    tsfi_xplos_run(&sched);

    printf("\n=== ALL CBT TAPE EXTRA FEATURE TESTS PASSED ===\n");
    return 0;
}
