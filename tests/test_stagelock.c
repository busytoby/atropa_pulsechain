#define _GNU_SOURCE
#include <stdio.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/tsfi_stagelock.h"

int main(void) {
    printf("=== RUNNING AUNCIENT USD STAGE LOCK (STAGELOCK) TESTS ===\n");

    TSFiStageLock lock;
    tsfi_stagelock_init(&lock);

    assert(lock.reader_count == 0);
    assert(lock.is_write_locked == false);

    // Test shared read lock
    bool ok = tsfi_stagelock_lock_read(&lock);
    assert(ok == true);
    assert(lock.reader_count == 1);
    assert(lock.is_write_locked == false);

    tsfi_stagelock_unlock_read(&lock);
    assert(lock.reader_count == 0);

    // Test exclusive write lock
    ok = tsfi_stagelock_lock_write(&lock);
    assert(ok == true);
    assert(lock.reader_count == 0);
    assert(lock.is_write_locked == true);

    tsfi_stagelock_unlock_write(&lock);
    assert(lock.is_write_locked == false);

    tsfi_stagelock_destroy(&lock);

    printf("   ✓ Shared read and exclusive write stage locks verified successfully.\n");
    printf("=== AUNCIENT USD STAGE LOCK (STAGELOCK) TESTS COMPLETE (PASS) ===\n");
    return 0;
}
