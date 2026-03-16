#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "tsfi_io.h"
#include "tsfi_helmholtz.h"

int main() {
    printf("=== TSFi Disk-to-SHM Sync Test ===\n");
    tsfi_helmholtz_init();
    size_t manifold_size = 512 * 1024;
    const char *disk_path = "sync_test.pos";
    const char *shm_name = "tsfi_sync_shm";

    // 1. Create a manifold with specific data
    void *manifold_orig = calloc(1, manifold_size);
    uint8_t *l0 = (uint8_t*)manifold_orig;
    l0[0] = 0xDE; l0[1] = 0xAD; l0[2] = 0xBE; l0[3] = 0xEF;
    
    uint8_t root_orig[32], receipt_orig[32];
    tsfi_helmholtz_reduce_11(root_orig, receipt_orig, NULL, NULL, manifold_orig, 1, 2026, NULL);

    // 2. Persist to Disk
    if (tsfi_persist_manifold(disk_path, manifold_orig, manifold_size) != 0) {
        printf("[FAIL] Failed to persist to disk\n");
        return 1;
    }

    // 3. Sync Disk to SHM
    tsfi_shm_unlink(shm_name); // Ensure clean start
    if (tsfi_sync_manifold_to_shm(disk_path, shm_name, manifold_size) != 0) {
        printf("[FAIL] Sync failed\n");
        return 1;
    }

    // 4. Verify SHM Content
    int shm_fd = tsfi_shm_open(shm_name, O_RDONLY, 0644);
    void *shm_ptr = mmap(NULL, manifold_size, PROT_READ, MAP_SHARED, shm_fd, 0);
    
    uint8_t root_shm[32], receipt_shm[32];
    tsfi_helmholtz_reduce_11(root_shm, receipt_shm, NULL, NULL, shm_ptr, 1, 2026, NULL);

    if (memcmp(root_orig, root_shm, 32) == 0) {
        printf("[PASS] Disk-to-SHM Sync Verified with Perfect Parity.\n");
    } else {
        printf("[FAIL] Manifold Divergence in SHM!\n");
        munmap(shm_ptr, manifold_size);
        close(shm_fd);
        return 1;
    }

    munmap(shm_ptr, manifold_size);
    close(shm_fd);
    tsfi_shm_unlink(shm_name);
    unlink(disk_path);
    free(manifold_orig);
    tsfi_helmholtz_cleanup();
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}
