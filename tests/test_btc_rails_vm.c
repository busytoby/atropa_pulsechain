#include "inc/btc_rails_vm.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

static void set_cia_pointer(M6526Cia *cia, uint32_t val) {
    cia->timer_a_count = val & 0xFFFF;
    cia->timer_b_count = (val >> 16) & 0xFFFF;
}

void run_stack_tests(BtcRailsVm *vm) {
    printf("[Test] Running 32-bit cascaded stack verification...\n");
    fflush(stdout);
    
    uint8_t payload1[] = { 0xAA, 0xBB, 0xCC, 0xDD };
    uint8_t payload2[] = { 0x11, 0x22, 0x33 };
    
    assert(btc_rails_vm_push_ds(vm, payload1, sizeof(payload1)) == 1);
    assert(btc_rails_vm_push_ds(vm, payload2, sizeof(payload2)) == 1);
    
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
    
    /* Gated pointer mode: bit 6 set (0x40) + bits 1-0 set to 0x03 for gated mode */
    vm->cia1.control_b = 0x43;
    vm->cia1.cnt_pin = 0;
    
    uint8_t payload[] = { 0x99 };
    assert(btc_rails_vm_push_ds(vm, payload, 1) == 0);
    
    vm->cia1.cnt_pin = 1;
    assert(btc_rails_vm_push_ds(vm, payload, 1) == 1);
    
    uint8_t popped = 0;
    assert(btc_rails_vm_pop_ds(vm, &popped, 1) == 1);
    assert(popped == 0x99);
    
    vm->cia1.control_b = 0x40; /* Restore default cascaded mode */
    
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
    assert((vm->cia1.icr & 0x08) != 0);
    
    printf("[Test] SDR and ICR event matrix verification passed.\n");
    fflush(stdout);
}

void run_pbon_clock_tests(BtcRailsVm *vm) {
    printf("[Test] Running PBON Port B Pin Modulation verification...\n");
    fflush(stdout);
    
    uint16_t save_a = vm->cia1.timer_a_count;
    uint16_t save_b = vm->cia1.timer_b_count;
    uint8_t save_ctrl_a = vm->cia1.control_a;
    uint8_t save_ctrl_b = vm->cia1.control_b;
    
    /* Disable cascaded mode temporarily for timer B clock test */
    vm->cia1.control_b = 0x00; /* System clock mode for Timer B */
    vm->cia1.timer_a_count = 1000;
    vm->cia1.timer_a_latch = 1000;
    vm->cia1.control_a = 0x03; /* bit 0 = start, bit 1 = PBON */
    vm->cia1.port_b_data = 0x00;
    
    btc_rails_vm_step_clock(vm, 500);
    assert(vm->cia1.timer_a_count == 500);
    assert((vm->cia1.port_b_data & 0x40) == 0);
    
    btc_rails_vm_step_clock(vm, 505);
    assert((vm->cia1.port_b_data & 0x40) != 0);
    assert((vm->cia1.icr & 0x01) != 0);
    
    /* Restore stack pointers and Control configurations */
    vm->cia1.timer_a_count = save_a;
    vm->cia1.timer_b_count = save_b;
    vm->cia1.control_a = save_ctrl_a;
    vm->cia1.control_b = save_ctrl_b;
    
    printf("[Test] PBON Port B Pin Modulation verification passed.\n");
    fflush(stdout);
}

void run_keycode_scan_tests(BtcRailsVm *vm) {
    printf("[Test] Running Keypad Matrix keycode verification...\n");
    fflush(stdout);
    
    assert(btc_rails_vm_scan_keycode(vm, KEYCODE_D) == 1);
    assert(vm->cia1.port_a_data == 0xF7);
    assert(vm->cia1.port_b_data == 0xFB);
    
    assert(btc_rails_vm_scan_keycode(vm, KEYCODE_A) == 1);
    assert(vm->cia1.port_a_data == 0xFD);
    assert(vm->cia1.port_b_data == 0xFD);
    
    printf("[Test] Keypad Matrix keycode verification passed.\n");
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
    
    set_cia_pointer(&(vm->cia1), 0x00018000);
    vm->cia1.control_b = 0x40; /* Restore correct cascaded configuration */
    
    uint32_t valid_locktime = 396000;
    uint8_t bytecode_pass[] = {
        0x01, 0x04, (uint8_t)(valid_locktime & 0xFF), (uint8_t)((valid_locktime >> 8) & 0xFF), (uint8_t)((valid_locktime >> 16) & 0xFF), (uint8_t)((valid_locktime >> 24) & 0xFF),
        0x04
    };
    assert(btc_rails_vm_deploy_yul(vm, bytecode_pass, sizeof(bytecode_pass)) == 1);
    
    vm->cia1.alarm_hours = 12;
    vm->cia1.alarm_mins = 0;
    vm->cia1.alarm_secs = 0;
    vm->cia1.alarm_tenths = 0;
    
    uint32_t future_locktime = 500000;
    uint8_t bytecode_alarm[] = {
        0x01, 0x04, (uint8_t)(future_locktime & 0xFF), (uint8_t)((future_locktime >> 8) & 0xFF), (uint8_t)((future_locktime >> 16) & 0xFF), (uint8_t)((future_locktime >> 24) & 0xFF),
        0x04
    };
    
    assert(btc_rails_vm_deploy_yul(vm, bytecode_alarm, sizeof(bytecode_alarm)) == 1);
    assert((vm->cia1.icr & 0x04) != 0);
    
    printf("[Test] Yul interpreter and TOD Alarm Match verification passed.\n");
    fflush(stdout);
}

void run_diyat_tax_tests(BtcRailsVm *vm) {
    printf("[Test] Running Diyat taxation mechanisms verification...\n");
    fflush(stdout);
    
    set_cia_pointer(&(vm->cia1), 0x00018000);
    vm->cia1.control_b = 0x40; /* Chained/Cascaded mode */
    
    uint32_t balance = 100000;
    
    /* 1. Performance Latency Tax: Measure counts and apply fee */
    vm->cia1.timer_a_count = 1000;
    btc_rails_vm_step_clock(vm, 150); /* Simulate execution cycles spent */
    
    uint32_t cycles_spent = 1000 - vm->cia1.timer_a_count;
    uint32_t performance_tax = cycles_spent * 10;
    assert(performance_tax == 1500);
    balance -= performance_tax;
    assert(balance == 98500);
    
    /* 2. Bandwidth SDR Tax: Deduct fee for serial transmission */
    vm->cia1.icr = 0;
    uint8_t val = 0xAA;
    assert(btc_rails_vm_push_ds(vm, &val, 1) == 1);
    
    uint8_t shifted_byte = 0;
    assert(btc_rails_vm_shift_sdr(vm, &shifted_byte) == 1);
    
    if (vm->cia1.icr & 0x08) {
        uint32_t bandwidth_tax = 500;
        balance -= bandwidth_tax;
    }
    assert(balance == 98000);
    
    /* 3. Gated Counter Penalty: Deduct fine if stack pointer is locked */
    vm->cia1.control_b = 0x43; /* Gated cascaded mode */
    vm->cia1.cnt_pin = 0;
    
    uint8_t val2 = 0xBB;
    int push_result = btc_rails_vm_push_ds(vm, &val2, 1);
    assert(push_result == 0);
    
    if (push_result == 0) {
        uint32_t lock_penalty = 5000;
        balance -= lock_penalty;
    }
    assert(balance == 93000);
    
    /* Restore control state */
    vm->cia1.control_b = 0x40;
    vm->cia1.cnt_pin = 1;
    
    printf("[Test] Diyat taxation verification passed. Remaining balance: %u satoshis.\n", balance);
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
    run_pbon_clock_tests(vm);
    run_keycode_scan_tests(vm);
    run_tree_and_dat_tests(vm);
    run_yul_and_alarm_tests(vm);
    run_diyat_tax_tests(vm);
    
    btc_rails_vm_free(vm);
    printf("=== ALL VM TESTS PASSED ===\n");
    fflush(stdout);
    return 0;
}
