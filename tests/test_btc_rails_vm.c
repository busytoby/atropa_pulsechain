#include "inc/btc_rails_vm.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

void run_stack_tests(BtcRailsVm *vm) {
    printf("[Test] Running 32-bit cascaded stack verification...\n");
    fflush(stdout);
    
    uint8_t payload1[] = { 0xAA, 0xBB, 0xCC, 0xDD };
    uint8_t payload2[] = { 0x11, 0x22, 0x33 };
    
    /* Push onto 32-bit Data Stack (DS) */
    assert(btc_rails_vm_push_ds(vm, payload1, sizeof(payload1)) == 1);
    assert(btc_rails_vm_push_ds(vm, payload2, sizeof(payload2)) == 1);
    
    /* Pop from Data Stack (DS) */
    uint8_t out[16];
    int len = btc_rails_vm_pop_ds(vm, out, sizeof(out));
    assert(len == sizeof(payload2));
    assert(memcmp(out, payload2, len) == 0);
    
    len = btc_rails_vm_pop_ds(vm, out, sizeof(out));
    assert(len == sizeof(payload1));
    assert(memcmp(out, payload1, len) == 0);
    
    printf("[Test] 32-bit stack verification passed.\n");
    fflush(stdout);
}

void run_gated_counter_tests(BtcRailsVm *vm) {
    printf("[Test] Running Gated Counter Control verification...\n");
    fflush(stdout);
    
    /* Configure CIA 1 Timer B to gated mode (0x03) and pull CNT pin low (0) */
    vm->cia1.control_b = 0x03;
    vm->cia1.cnt_pin = 0;
    
    uint8_t payload[] = { 0x99 };
    /* Push should fail because counter modification is gated to CNT pin state */
    assert(btc_rails_vm_push_ds(vm, payload, 1) == 0);
    
    /* Pull CNT pin high (1) */
    vm->cia1.cnt_pin = 1;
    /* Push should now succeed */
    assert(btc_rails_vm_push_ds(vm, payload, 1) == 1);
    
    /* Pop should succeed when CNT is high */
    uint8_t popped = 0;
    assert(btc_rails_vm_pop_ds(vm, &popped, 1) == 1);
    assert(popped == 0x99);
    
    /* Restore control state */
    vm->cia1.control_b = 0x02;
    
    printf("[Test] Gated Counter Control verification passed.\n");
    fflush(stdout);
}

void run_sdr_and_icr_tests(BtcRailsVm *vm) {
    printf("[Test] Running SDR Serial Shift and ICR Event Matrix verification...\n");
    fflush(stdout);
    
    uint8_t val = 0x55;
    assert(btc_rails_vm_push_ds(vm, &val, 1) == 1);
    
    uint8_t shifted_byte = 0;
    assert(btc_rails_vm_shift_sdr(vm, &shifted_byte) == 1);
    assert(shifted_byte == 0x55);
    assert(vm->cia1.sdr == 0x55);
    
    /* Verify that SDR completion bit (bit 3 / 0x08) is set in the ICR */
    assert((vm->cia1.icr & 0x08) != 0);
    
    printf("[Test] SDR and ICR event matrix verification passed.\n");
    fflush(stdout);
}

void run_tree_and_dat_tests(BtcRailsVm *vm) {
    printf("[Test] Running 2-3 tree and .dat.bin database verification...\n");
    fflush(stdout);
    
    const char *filepath = "tests/mock_db.dat.bin";
    FILE *f = fopen(filepath, "wb");
    assert(f != NULL);
    
    uint32_t key1 = 0x42;
    uint32_t size1 = 6;
    uint8_t data1[] = "Active";
    
    fwrite(&key1, 4, 1, f);
    fwrite(&size1, 4, 1, f);
    fwrite(data1, 1, size1, f);
    
    fclose(f);
    
    assert(btc_rails_vm_load_dat(vm, filepath) == 1);
    
    size_t out_size = 0;
    uint8_t *res = two_three_tree_search(vm->root_index, 0x42, &out_size);
    assert(res != NULL);
    assert(out_size == 6);
    assert(memcmp(res, "Active", 6) == 0);
    
    remove(filepath);
    printf("[Test] 2-3 tree and .dat.bin database verification passed.\n");
    fflush(stdout);
}

void run_yul_and_alarm_tests(BtcRailsVm *vm) {
    printf("[Test] Running Yul interpreter and TOD Alarm Match verification...\n");
    fflush(stdout);
    
    /* Test standard CLTV pass:
       Current TOD: 12:00:00.0 (432000 tenths)
       Target locktime: 11:00:00.0 (396000 tenths)
    */
    uint32_t valid_locktime = 396000;
    uint8_t bytecode_pass[] = {
        0x01, 0x04, (uint8_t)(valid_locktime & 0xFF), (uint8_t)((valid_locktime >> 8) & 0xFF), (uint8_t)((valid_locktime >> 16) & 0xFF), (uint8_t)((valid_locktime >> 24) & 0xFF),
        0x04
    };
    assert(btc_rails_vm_deploy_yul(vm, bytecode_pass, sizeof(bytecode_pass)) == 1);
    
    /* Test TOD Alarm Match:
       Set alarm to match current clock time: 12:00:00.0
       This will set the alarm flag (0x04) in the ICR, allowing CLTV to pass regardless of locktime target.
    */
    vm->cia1.alarm_hours = 12;
    vm->cia1.alarm_mins = 0;
    vm->cia1.alarm_secs = 0;
    vm->cia1.alarm_tenths = 0;
    
    uint32_t future_locktime = 500000; /* Target is in future (500000 > 432000) */
    uint8_t bytecode_alarm[] = {
        0x01, 0x04, (uint8_t)(future_locktime & 0xFF), (uint8_t)((future_locktime >> 8) & 0xFF), (uint8_t)((future_locktime >> 16) & 0xFF), (uint8_t)((future_locktime >> 24) & 0xFF),
        0x04
    };
    
    /* Should pass because the Alarm matched, setting the ICR flag which overrides locktime limits */
    assert(btc_rails_vm_deploy_yul(vm, bytecode_alarm, sizeof(bytecode_alarm)) == 1);
    assert((vm->cia1.icr & 0x04) != 0); /* Verify Alarm matched bit is set in ICR */
    
    printf("[Test] Yul interpreter and TOD Alarm Match verification passed.\n");
    fflush(stdout);
}

int main() {
    printf("=== AUNCIENT BTC RAILS VM TESTING ===\n");
    fflush(stdout);
    BtcRailsVm *vm = btc_rails_vm_init(262144);
    assert(vm != NULL);
    
    run_stack_tests(vm);
    run_gated_counter_tests(vm);
    run_sdr_and_icr_tests(vm);
    run_tree_and_dat_tests(vm);
    run_yul_and_alarm_tests(vm);
    
    btc_rails_vm_free(vm);
    printf("=== ALL VM TESTS PASSED ===\n");
    fflush(stdout);
    return 0;
}
