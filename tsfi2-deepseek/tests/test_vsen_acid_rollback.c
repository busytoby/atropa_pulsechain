#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include "tsfi_zorse_eval.h"
#include "tsfi_xplos_shell_cbt_vtam.h"

int main(void) {
    printf("[TEST VSEN RDBMS ACID] Starting z/VSEn Enterprise RDBMS Unit Test Suite...\n");

    // Clean initial test state
    unlink("vaesen_registry.dat.bin");
    unlink("vaesen_registry.wal.dat.bin");

    // 1. Test MVCC Transaction & Record Registration
    assert(tsfi_vsen_vaesen_register("HoganBank", "Teddy Bear", 5, "Active") == 0);
    assert(tsfi_vsen_vaesen_register("Tomte", "Nisse", 3, "Friendly") == 0);

    char type_buf[64];
    char status_buf[32];
    int risk = 0;

    // 2. Test Record Lookup
    assert(tsfi_vsen_vaesen_lookup("HoganBank", type_buf, &risk, status_buf, sizeof(type_buf)) == 0);
    assert(strcmp(type_buf, "Teddy Bear") == 0);
    assert(risk == 5);

    // Record timestamp of initial version
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    uint64_t initial_timestamp = (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)(ts.tv_nsec / 1000);
    usleep(10000); // 10ms wait

    // 3. Test MVCC Record Update
    assert(tsfi_vsen_vaesen_register("HoganBank", "Teddy Bear Sovereign", 9, "Elevated") == 0);
    assert(tsfi_vsen_vaesen_lookup("HoganBank", type_buf, &risk, status_buf, sizeof(type_buf)) == 0);
    assert(strcmp(type_buf, "Teddy Bear Sovereign") == 0);
    assert(risk == 9);

    // 4. Test Point-In-Time Restore (PITR) / Historical Time-Travel Lookup
    assert(tsfi_vsen_vaesen_lookup_as_of("HoganBank", initial_timestamp, type_buf, &risk, status_buf, sizeof(type_buf)) == 0);
    assert(strcmp(type_buf, "Teddy Bear") == 0);
    assert(risk == 5);

    // 5. Test Cryptographic DNA Hash Chain Audit
    assert(tsfi_vsen_audit_chain_verify("vaesen_registry.dat.bin") == 0);

    // 6. Test WAL Log Recovery API
    assert(tsfi_vsen_wal_recover("vaesen_registry.dat.bin") == 0);

    // 7. Test Savepoint & Sub-Transaction APIs
    uint64_t tx_id = 0;
    assert(tsfi_vsen_tx_begin(&tx_id) == 0);
    assert(tsfi_vsen_tx_savepoint(tx_id, "SP1") == 0);
    assert(tsfi_vsen_tx_rollback_to_savepoint(tx_id, "SP1") == 0);
    assert(tsfi_vsen_tx_commit(tx_id) == 0);

    // 8. Test VTAM SNA LU6.2 2PC SyncPoint Engine
    assert(tsfi_vsen_vtam_2pc_prepare(tx_id, 101) == true);
    assert(tsfi_vsen_vtam_2pc_commit(tx_id, 101) == true);
    assert(tsfi_vsen_vtam_2pc_rollback(tx_id, 101) == true);

    // 9. Test VTAM Deadlock WFG Engine
    assert(tsfi_vsen_vtam_deadlock_check_and_resolve(101, tx_id) == true);

    printf("[TEST VSEN RDBMS ACID] All 9 z/VSEn Enterprise RDBMS tests passed successfully!\n");
    return 0;
}
