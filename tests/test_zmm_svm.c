#include <stdio.h>
#include <assert.h>
#include "tsfi_zmm_svm.h"

void test_svm_basic_mapping() {
    printf("[TEST] Running basic SVM mapping checks...\n");
    tsfi_zmm_svm_init();

    // 1. Write telemetry data to Card 1, Page 0x70, offset 5 ($7005)
    uint16_t addr = 0x7005;
    tsfi_zmm_svm_write_byte(1, addr, 0xAA);
    
    // Verify Card 1 read
    uint8_t val1 = tsfi_zmm_svm_read_byte(1, addr);
    assert(val1 == 0xAA);

    // Verify Card 2 initially reads 0 (unmapped page)
    uint8_t val2 = tsfi_zmm_svm_read_byte(2, addr);
    assert(val2 == 0x00);

    // 2. Perform zero-copy SVM mount: Map Card 1 page 0x70 to Card 2
    int res = tsfi_zmm_svm_mount(1, 2, 0x70);
    assert(res == 0);

    // 3. Verify zero-copy write propagation
    // Card 2 must immediately see Card 1's value at $7005
    val2 = tsfi_zmm_svm_read_byte(2, addr);
    assert(val2 == 0xAA);

    // Modify value from Card 2
    tsfi_zmm_svm_write_byte(2, addr, 0xBB);

    // Card 1 must immediately see the change (since they share the pointer)
    val1 = tsfi_zmm_svm_read_byte(1, addr);
    assert(val1 == 0xBB);

    printf("[SUCCESS] SVM basic mapping passed.\n");
    tsfi_zmm_svm_destroy();
}

void test_svm_page_locking() {
    printf("[TEST] Running SVM page lock checks...\n");
    tsfi_zmm_svm_init();

    uint16_t addr = 0x8520; // Page 0x85
    tsfi_zmm_svm_write_byte(3, addr, 0x11);
    
    // Lock Page 0x85 on Card 3
    tsfi_zmm_svm_lock_page(3, 0x85);

    // Attempt write while page is locked (should be ignored)
    tsfi_zmm_svm_write_byte(3, addr, 0x99);
    
    uint8_t val = tsfi_zmm_svm_read_byte(3, addr);
    assert(val == 0x11); // Value must remain unchanged

    // Attempt to mount a page over a locked page (should fail with -2)
    int res = tsfi_zmm_svm_mount(1, 3, 0x85);
    assert(res == -2);

    // Unlock and write again
    tsfi_zmm_svm_unlock_page(3, 0x85);
    tsfi_zmm_svm_write_byte(3, addr, 0x99);
    val = tsfi_zmm_svm_read_byte(3, addr);
    assert(val == 0x99);

    printf("[SUCCESS] SVM page locking passed.\n");
    tsfi_zmm_svm_destroy();
}

int main() {
    printf("=== RUNNING TSFI ZMM SVM INTEGRATION TESTS ===\n");
    
    test_svm_basic_mapping();
    test_svm_page_locking();

    printf("=== ALL SVM INTEGRATION TESTS PASSED ===\n");
    return 0;
}
