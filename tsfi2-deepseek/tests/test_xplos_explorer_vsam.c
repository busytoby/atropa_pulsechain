#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "tsfi_xplos_kernel.h"
#include "tsfi_xplos_kernel_internal.h"
#include "tsfi_mainframe_computerworld.h"

extern bool tsfi_xplos_shell_explorer(const char *cmd);

int main(void) {
    printf("=== RUNNING EXPLORER AND MAINFRAME VSAM COMPLIANCE TESTS ===\n");

    // 1. Test explorer command "cbtcpu"
    printf("  -> Testing cbtcpu command...\n");
    bool cpu_ok = tsfi_xplos_shell_explorer("cbtcpu");
    assert(cpu_ok == true);

    // 2. Test explorer command "cbtstat"
    printf("  -> Testing cbtstat command...\n");
    bool stat_ok = tsfi_xplos_shell_explorer("cbtstat");
    assert(stat_ok == true);

    // 3. Test explorer command "cbthelp"
    printf("  -> Testing cbthelp command...\n");
    bool help_ok = tsfi_xplos_shell_explorer("cbthelp");
    assert(help_ok == true);

    // 4. Test explorer command "cbtexplorertvtocstatsstatusdetailsprogress" (leveraging mainframe VSAM)
    printf("  -> Testing cbtexplorertvtocstatsstatusdetailsprogress command (mainframe VSAM link)...\n");
    bool tvtoc_ok = tsfi_xplos_shell_explorer("cbtexplorertvtocstatsstatusdetailsprogress");
    assert(tvtoc_ok == true);

    // 5. Test explorer command "cbtexplorerignorelistreset"
    printf("  -> Testing cbtexplorerignorelistreset command...\n");
    bool reset_ok = tsfi_xplos_shell_explorer("cbtexplorerignorelistreset");
    assert(reset_ok == true);

    // 6. Test explorer command "cbtexplorerignorelistresetstat"
    printf("  -> Testing cbtexplorerignorelistresetstat command...\n");
    bool resetstat_ok = tsfi_xplos_shell_explorer("cbtexplorerignorelistresetstat");
    assert(resetstat_ok == true);

    // 7. Test explorer command "cbtdict" for browsing PDS layout dynamically
    printf("  -> Testing cbtdict command for PDS directory layout browsing...\n");
    bool cbtdict_ok = tsfi_xplos_shell_explorer("cbtdict test_vsam.dat.bin");
    assert(cbtdict_ok == true);

    printf("\n=== EXPLORER MAINFRAME VSAM TESTS PASSED ===\n");
    return 0;
}
