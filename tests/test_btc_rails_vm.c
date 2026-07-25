#include "inc/btc_rails_vm.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

/* Helper helper to set stack pointer externally in tests */
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
    
    vm->cia1.control_b = 0x03;
    vm->cia1.cnt_pin = 0;
    
    uint8_t payload[] = { 0x99 };
    assert(btc_rails_vm_push_ds(vm, payload, 1) == 0);
    
    vm->cia1.cnt_pin = 1;
    assert(btc_rails_vm_push_ds(vm, payload, 1) == 1);
    
    uint8_t popped = 0;
    assert(btc_rails_vm_pop_ds(vm, &popped, 1) == 1);
    assert(popped == 0x99);
    
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
    assert((vm->cia1.icr & 0x08) != 0);
    
    printf("[Test] SDR and ICR event matrix verification passed.\n");
    fflush(stdout);
}

void run_pbon_clock_tests(BtcRailsVm *vm) {
    printf("[Test] Running PBON Port B Pin Modulation verification...\n");
    fflush(stdout);
    
    /* Save current stack pointer to avoid clock cycle test side-effects */
    uint16_t save_a = vm->cia1.timer_a_count;
    uint16_t save_b = vm->cia1.timer_b_count;
    
    /* Configure Timer A to start at 1000 and enable PBON output enable (bit 1) */
    vm->cia1.timer_a_count = 1000;
    vm->cia1.timer_a_latch = 1000;
    vm->cia1.control_a = 0x03; /* bit 0 = start, bit 1 = PBON */
    vm->cia1.port_b_data = 0x00;
    
    /* Step clock less than 1000 cycles, should not toggle PBON */
    btc_rails_vm_step_clock(vm, 500);
    assert(vm->cia1.timer_a_count == 500);
    assert((vm->cia1.port_b_data & 0x40) == 0);
    
    /* Step remaining cycles to force underflow, toggling Pin 6 (0x40) */
    btc_rails_vm_step_clock(vm, 505);
    assert((vm->cia1.port_b_data & 0x40) != 0); /* Pin 6 toggled to high */
    assert((vm->cia1.icr & 0x01) != 0);        /* Timer A underflow set in ICR */
    
    /* Restore stack pointer */
    vm->cia1.timer_a_count = save_a;
    vm->cia1.timer_b_count = save_b;
    vm->cia1.control_a = 0x01;
    
    printf("[Test] PBON Port B Pin Modulation verification passed.\n");
    fflush(stdout);
}

void run_keycode_scan_tests(BtcRailsVm *vm) {
    printf("[Test] Running Keypad Matrix keycode verification...\n");
    fflush(stdout);
    
    /* Scan D keycode */
    assert(btc_rails_vm_scan_keycode(vm, KEYCODE_D) == 1);
    assert(vm->cia1.port_a_data == 0xF7); /* Column 3 low */
    assert(vm->cia1.port_b_data == 0xFB); /* Row 2 low */
    
    /* Scan A keycode */
    assert(btc_rails_vm_scan_keycode(vm, KEYCODE_A) == 1);
    assert(vm->cia1.port_a_data == 0xFD); /* Column 1 low */
    assert(vm->cia1.port_b_data == 0xFD); /* Row 1 low */
    
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
    
    /* Restore pointer in case of any clock drift */
    set_cia_pointer(&(vm->cia1), 0x00018000);
    vm->cia1.control_b = 0x02;
    
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
    
    btc_rails_vm_free(vm);
    printf("=== ALL VM TESTS PASSED ===\n");
    fflush(stdout);
    return 0;
}
