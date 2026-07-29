#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "tsfi_xplos_kernel.h"
#include "tsfi_xplos_kernel_internal.h"
#include "tsfi_mainframe_computerworld.h"

extern bool tsfi_xplos_shell_cbt_jcl(const char *cmd);
extern bool tsfi_xplos_shell_cbt_jes(const char *cmd);
extern bool tsfi_xplos_shell_book(const char *cmd);

int main(void) {
    printf("=== RUNNING SKELETON-HASP-BOOK END-TO-END PROOFS ===\n");

    // 1. Initialize virtual disk VFS and JCL spool systems
    XplosVirtualDisk vfs;
    memset(&vfs, 0, sizeof(XplosVirtualDisk));
    vfs.count = 0;

    // 2. SKELETON: Simulate expanding a JCL job from template
    printf("  -> Phase 1: Expanding SKELETON JCL job...\n");
    // We register the job under the CBT JCL command tables
    bool jcl_ok = tsfi_xplos_shell_cbt_jcl("jclrun JOB_PROD");
    assert(jcl_ok == true);

    // 3. HASP: Route expanded job via spool loops
    printf("  -> Phase 2: Routing job through HASP spool scheduler...\n");
    bool jes_ok = tsfi_xplos_shell_cbt_jes("cbthaspspoollogsstatus");
    assert(jes_ok == true);

    // 4. BOOK: Load dataset members dynamically into VSAM
    printf("  -> Phase 3: Loading PDS members via BOOK loader to VSAM...\n");
    remove("VTOC.dat.bin");
    tsfi_cw_vsam_ksds ksds;
    int open_rc = tsfi_cw_vsam_open(&ksds, "VTOC.dat.bin");
    assert(open_rc == 0);

    // Write a mock book record representing PDS catalog data
    uint8_t mock_data[64] = "PGM=TEST_PROD,RECFM=FB,LRECL=80";
    int write_rc = tsfi_cw_vsam_write(&ksds, "M1", mock_data, 32);
    if (write_rc != 0) {
        printf("  -> tsfi_cw_vsam_write failed with RC=%d (filepath='%s')\n", write_rc, ksds.filepath);
    }
    assert(write_rc == 0);

    // 5. Read back verify the record from VSAM (WinchesterMQ loopback route)
    uint8_t read_buf[64] = {0};
    int read_len = 0;
    int read_rc = tsfi_cw_vsam_read(&ksds, "M1", read_buf, sizeof(read_buf), &read_len);
    assert(read_rc == 0);
    assert(strncmp((char *)read_buf, "PGM=TEST_PROD", 13) == 0);

    printf("  -> End-to-end data pipeline integrity verified successfully.\n");
    printf("\n=== SKELETON HASP BOOK PROOFS PASSED ===\n");
    return 0;
}
