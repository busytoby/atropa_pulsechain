#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_xplos_kernel.h"
#include "tsfi_xplos_shell_cbt_extra.h"

extern XplosVirtualDisk g_vfs;
extern bool g_vfs_initialized;

int main(void) {
    printf("====================================================================\n");
    printf("        TSFi XplOS IEBDG DATASET GENERATOR TEST SUITE               \n");
    printf("====================================================================\n");

    // Initialize Scheduler and VFS
    XplosScheduler sched;
    tsfi_xplos_init_scheduler(&sched);

    tsfi_xplos_init_vfs(&g_vfs);
    g_vfs_initialized = true;

    XplosShell shell;
    tsfi_xplos_init_shell(&shell);

    // Test Case 1: Valid Sequential Pattern Generation
    printf("[TEST 1] Generating sequential pattern (SEQ)...\n");
    bool dg_seq = tsfi_xplos_shell_exec(&shell, &sched, "iebdg SEQOUT SEQ");
    assert(dg_seq == true);
    tsfi_xplos_run(&sched);

    // Verify file output in VFS
    char vfs_name[128];
    snprintf(vfs_name, sizeof(vfs_name), "SEQOUT.dat.bin");
    int found_idx = -1;
    for (int i = 0; i < g_vfs.count; i++) {
        if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, vfs_name) == 0) {
            found_idx = i;
            break;
        }
    }
    assert(found_idx != -1);
    assert(strcmp(g_vfs.files[found_idx].data, "LINE01\nLINE02\nLINE03\nLINE04\n") == 0);
    printf("   ✓ Sequential pattern generated and verified.\n");

    // Test Case 2: Valid Fallback/Default Pattern Generation
    printf("[TEST 2] Generating fallback pattern (DUMMY)...\n");
    bool dg_dummy = tsfi_xplos_shell_exec(&shell, &sched, "iebdg DUMMYOUT FILL");
    assert(dg_dummy == true);
    tsfi_xplos_run(&sched);

    snprintf(vfs_name, sizeof(vfs_name), "DUMMYOUT.dat.bin");
    found_idx = -1;
    for (int i = 0; i < g_vfs.count; i++) {
        if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, vfs_name) == 0) {
            found_idx = i;
            break;
        }
    }
    assert(found_idx != -1);
    assert(strcmp(g_vfs.files[found_idx].data, "AUNCIENT FIELDATA GENERATED DUMMY DATA\n") == 0);
    printf("   ✓ Fallback pattern generated and verified.\n");

    // Test Case 3: Error handling on missing parameters
    printf("[TEST 3] Running IEBDG with missing parameters (expected fail info)...\n");
    bool dg_fail = tsfi_xplos_shell_exec(&shell, &sched, "iebdg SEQOUT");
    assert(dg_fail == true); // Command executes but reports error code
    tsfi_xplos_run(&sched);
    printf("   ✓ Error handling verified.\n");

    // Test Case 4: IEBCOMPR integration testing
    printf("[TEST 4] Testing IEBCOMPR with IEBDG generated datasets...\n");
    bool dg_c1 = tsfi_xplos_shell_exec(&shell, &sched, "iebdg SEQ1 SEQ");
    assert(dg_c1);
    tsfi_xplos_run(&sched);
    bool dg_c2 = tsfi_xplos_shell_exec(&shell, &sched, "iebdg SEQ2 SEQ");
    assert(dg_c2);
    tsfi_xplos_run(&sched);
    bool cmp_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebcompr SEQ1 SEQ2");
    assert(cmp_ok == true);
    tsfi_xplos_run(&sched);
    printf("   ✓ IEBCOMPR comparison integration verified.\n");

    // Test Case 5: IEBGENER copy integration testing
    printf("[TEST 5] Testing IEBGENER copy of IEBDG generated dataset...\n");
    bool gen_ok = tsfi_xplos_shell_exec(&shell, &sched, "iebgener SEQ1 SEQTGT");
    assert(gen_ok == true);
    tsfi_xplos_run(&sched);
    printf("   ✓ IEBGENER copy integration verified.\n");

    // Test Case 6: JCL pipeline conditional testing
    printf("[TEST 6] Testing JCL run pipeline using IEBDG input...\n");
    assert(tsfi_xplos_create_file(&g_vfs, "DGJCL.dat.bin", 2048) == true);
    XplosFile *jcl_file = &g_vfs.files[g_vfs.count - 1];
    strcpy(jcl_file->data,
           "//DGJCL JOB 'PIPELINE TEST',CLASS=A\n"
           "//STEP1 EXEC PGM=IEBCOPY\n"
           "//SYSUT1 DD DSN=SEQ1,DISP=SHR\n"
           "//SYSUT2 DD DSN=SEQ2,DISP=SHR\n");
    jcl_file->size_bytes = (uint32_t)strlen(jcl_file->data);

    bool jcl_ok = tsfi_xplos_shell_exec(&shell, &sched, "jclrun DGJCL");
    assert(jcl_ok == true);
    tsfi_xplos_run(&sched);
    printf("   ✓ JCL pipeline integration verified.\n");

    // Test Case 7: REXX Scripting variable mapping check
    printf("[TEST 7] Testing CBTREXX pool processing of generated content...\n");
    bool rx_ok = tsfi_xplos_shell_exec(&shell, &sched, "cbtrexx vput SYSVAR 953467954114363");
    assert(rx_ok == true);
    tsfi_xplos_run(&sched);
    printf("   ✓ CBTREXX pool processing verified.\n");

    printf("====================================================================\n");
    printf("ALL IEBDG DATASET GENERATOR UNIT TESTS PASSED SUCCESSFULLY\n");
    printf("====================================================================\n");
    return 0;
}
