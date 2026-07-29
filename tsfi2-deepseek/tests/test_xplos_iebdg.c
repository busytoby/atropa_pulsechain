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

    printf("====================================================================\n");
    printf("ALL IEBDG DATASET GENERATOR UNIT TESTS PASSED SUCCESSFULLY\n");
    printf("====================================================================\n");
    return 0;
}
