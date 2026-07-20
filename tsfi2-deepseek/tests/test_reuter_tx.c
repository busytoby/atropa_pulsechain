#include "tsfi_reuter_tx.h"
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
    printf("[INFO] Starting Andreas Reuter Transaction Compliance Test...\n");
    
    tsfi_reuter_init();
    assert(tsfi_reuter_get_global_lsn() == 0);
    
    // Test 1: Write-Ahead Logging (WAL)
    const char *temp_log = "tmp/reuter_tx.log";
    int fd = open(temp_log, O_RDWR | O_CREAT | O_TRUNC, 0644);
    assert(fd >= 0);
    
    uint8_t before[10] = {1, 2, 3};
    uint8_t after[10] = {1, 2, 9};
    uint64_t lsn1 = 0;
    
    int rc = tsfi_reuter_write_wal(fd, 1001, 12, 3, before, after, &lsn1);
    assert(rc == 0);
    assert(lsn1 == 1);
    assert(tsfi_reuter_get_global_lsn() == 1);
    
    // Test 2: Compensation Log Record (CLR)
    uint64_t lsn2 = 0;
    rc = tsfi_reuter_write_clr(fd, 1001, lsn1, 12, 3, before, &lsn2);
    assert(rc == 0);
    assert(lsn2 == 2);
    assert(tsfi_reuter_get_global_lsn() == 2);
    
    close(fd);
    unlink(temp_log);
    
    // Test 3: LU6.2 Two-Phase Commit (2PC)
    tsfi_reuter_2pc_coordinator coord;
    tsfi_reuter_2pc_init(&coord, 1002);
    
    tsfi_reuter_2pc_add_participant(&coord, 101);
    tsfi_reuter_2pc_add_participant(&coord, 102);
    
    // Vote commit for both nodes
    tsfi_reuter_2pc_vote(&coord, 101, true);
    tsfi_reuter_2pc_vote(&coord, 102, true);
    
    bool is_committed = false;
    rc = tsfi_reuter_2pc_finalize(&coord, &is_committed);
    assert(rc == 0);
    assert(is_committed == true);
    
    // Test 4: 2PC Abort Veto
    tsfi_reuter_2pc_init(&coord, 1003);
    tsfi_reuter_2pc_add_participant(&coord, 101);
    tsfi_reuter_2pc_add_participant(&coord, 102);
    
    tsfi_reuter_2pc_vote(&coord, 101, true);
    tsfi_reuter_2pc_vote(&coord, 102, false); // Veto
    
    rc = tsfi_reuter_2pc_finalize(&coord, &is_committed);
    assert(rc == 0);
    assert(is_committed == false);
    
    printf("[SUCCESS] Andreas Reuter Transaction Compliance Test Passed!\n");
    return 0;
}
