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

    printf("\n=== ALL CBT TAPE EXTRA FEATURE TESTS PASSED ===\n");
    return 0;
}
