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

void test_svm_mmu_registers() {
    printf("[TEST] Running SVM MMU register ($DF10-$DF13) checks...\n");
    tsfi_zmm_svm_init();

    // 1. Write telemetry to Card 1, Page 0x70, offset 5 ($7005)
    uint16_t telemetry_addr = 0x7005;
    tsfi_zmm_svm_write_byte(1, telemetry_addr, 0xEE);

    // 2. Perform Mount of Card 1's Page 0x70 to Card 2 via MMU registers
    // Write Source Card (1) to Card 2's MMU_SRC_CARD ($DF11)
    tsfi_zmm_svm_write_byte(2, 0xDF11, 1);
    // Write Page Index (0x70) to Card 2's MMU_PAGE ($DF12)
    tsfi_zmm_svm_write_byte(2, 0xDF12, 0x70);
    // Trigger Mount command (0x01) on Card 2's MMU_CMD ($DF10)
    tsfi_zmm_svm_write_byte(2, 0xDF10, 0x01);

    // Verify status register MMU_STATUS ($DF13) is 0 (success)
    uint8_t status = tsfi_zmm_svm_read_byte(2, 0xDF13);
    assert(status == 0);

    // Card 2 must now see telemetry data from Card 1
    uint8_t read_val = tsfi_zmm_svm_read_byte(2, telemetry_addr);
    assert(read_val == 0xEE);

    // 3. Lock Page 0x70 on Card 2 via MMU registers
    tsfi_zmm_svm_write_byte(2, 0xDF12, 0x70);
    tsfi_zmm_svm_write_byte(2, 0xDF10, 0x02); // Trigger Lock

    status = tsfi_zmm_svm_read_byte(2, 0xDF13);
    assert(status == 0);

    // Verify writing is ignored because page is locked
    tsfi_zmm_svm_write_byte(2, telemetry_addr, 0x99);
    read_val = tsfi_zmm_svm_read_byte(2, telemetry_addr);
    assert(read_val == 0xEE);

    // 4. Try mounting Card 0 page over locked Page 0x70 (should fail with -2 status)
    tsfi_zmm_svm_write_byte(2, 0xDF11, 0);
    tsfi_zmm_svm_write_byte(2, 0xDF12, 0x70);
    tsfi_zmm_svm_write_byte(2, 0xDF10, 0x01); // Trigger Mount

    status = tsfi_zmm_svm_read_byte(2, 0xDF13);
    assert(status == (uint8_t)-2); // -2 cast to uint8_t is 254

    // 5. Unlock page 0x70 on Card 2 via MMU registers
    tsfi_zmm_svm_write_byte(2, 0xDF12, 0x70);
    tsfi_zmm_svm_write_byte(2, 0xDF10, 0x03); // Trigger Unlock

    status = tsfi_zmm_svm_read_byte(2, 0xDF13);
    assert(status == 0);

    // Modify and check that writing works again
    tsfi_zmm_svm_write_byte(2, telemetry_addr, 0x55);
    read_val = tsfi_zmm_svm_read_byte(2, telemetry_addr);
    assert(read_val == 0x55);

    printf("[SUCCESS] SVM MMU registers passed.\n");
    tsfi_zmm_svm_destroy();
}

int main() {
    printf("=== RUNNING TSFI ZMM SVM INTEGRATION TESTS ===\n");
    
    test_svm_basic_mapping();
    test_svm_page_locking();
    test_svm_mmu_registers();

    printf("=== ALL SVM INTEGRATION TESTS PASSED ===\n");
    return 0;
}
