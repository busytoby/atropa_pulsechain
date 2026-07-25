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
    
    /* Push/Pop Return Stack (RS) */
    assert(btc_rails_vm_push_rs(vm, payload1, sizeof(payload1)) == 1);
    len = btc_rails_vm_pop_rs(vm, out, sizeof(out));
    assert(len == sizeof(payload1));
    assert(memcmp(out, payload1, len) == 0);
    
    printf("[Test] 32-bit stack verification passed.\n");
    fflush(stdout);
}

void run_sdr_tests(BtcRailsVm *vm) {
    printf("[Test] Running SDR Serial Data Register shift verification...\n");
    fflush(stdout);
    
    uint8_t val = 0x55;
    assert(btc_rails_vm_push_ds(vm, &val, 1) == 1);
    
    uint8_t shifted_byte = 0;
    assert(btc_rails_vm_shift_sdr(vm, &shifted_byte) == 1);
    assert(shifted_byte == 0x55);
    assert(vm->cia1.sdr == 0x55);
    
    printf("[Test] SDR shift verification passed.\n");
    fflush(stdout);
}

void run_tree_and_dat_tests(BtcRailsVm *vm) {
    printf("[Test] Running 2-3 tree and .dat.bin database verification...\n");
    fflush(stdout);
    
    /* Create a mock .dat.bin database */
    const char *filepath = "tests/mock_db.dat.bin";
    FILE *f = fopen(filepath, "wb");
    assert(f != NULL);
    
    uint32_t key1 = 0x42;
    uint32_t size1 = 6;
    uint8_t data1[] = "Active";
    
    uint32_t key2 = 0x88;
    uint32_t size2 = 8;
    uint8_t data2[] = "Launcher";
    
    fwrite(&key1, 4, 1, f);
    fwrite(&size1, 4, 1, f);
    fwrite(data1, 1, size1, f);
    
    fwrite(&key2, 4, 1, f);
    fwrite(&size2, 4, 1, f);
    fwrite(data2, 1, size2, f);
    
    fclose(f);
    
    /* Load .dat.bin database into VM index */
    assert(btc_rails_vm_load_dat(vm, filepath) == 1);
    
    /* Search index */
    size_t out_size = 0;
    uint8_t *res = two_three_tree_search(vm->root_index, 0x42, &out_size);
    assert(res != NULL);
    assert(out_size == 6);
    assert(memcmp(res, "Active", 6) == 0);
    
    res = two_three_tree_search(vm->root_index, 0x88, &out_size);
    assert(res != NULL);
    assert(out_size == 8);
    assert(memcmp(res, "Launcher", 8) == 0);
    
    /* Clean up mock file */
    remove(filepath);
    printf("[Test] 2-3 tree and .dat.bin database verification passed.\n");
    fflush(stdout);
}

void run_yul_interpreter_tests(BtcRailsVm *vm) {
    printf("[Test] Running Yul interpreter and locktime verification...\n");
    fflush(stdout);
    
    /* Current TOD: 12:00:00.0 -> total tenths = 12 * 36000 = 432000 tenths */
    /* Target locktime: 11 hours -> 11 * 36000 = 396000 tenths. Locktime satisfied! */
    uint32_t valid_locktime = 396000;
    
    /* Bytecode:
       1. Push 0x42 (key for "Active") -> Opcode 0x01, size 4, value 0x42, 0x00, 0x00, 0x00
       2. Delegate to lookup -> Opcode 0x03
       3. Push locktime value -> Opcode 0x01, size 4, value valid_locktime
       4. OP_CHECKLOCKTIMEVERIFY -> Opcode 0x04
    */
    uint8_t bytecode_pass[] = {
        0x01, 0x04, 0x42, 0x00, 0x00, 0x00,
        0x03,
        0x01, 0x04, (uint8_t)(valid_locktime & 0xFF), (uint8_t)((valid_locktime >> 8) & 0xFF), (uint8_t)((valid_locktime >> 16) & 0xFF), (uint8_t)((valid_locktime >> 24) & 0xFF),
        0x04
    };
    
    assert(btc_rails_vm_deploy_yul(vm, bytecode_pass, sizeof(bytecode_pass)) == 1);
    
    /* The result "Active" should remain at the top of the Data Stack since CLTV succeeded */
    uint8_t out[16];
    int len = btc_rails_vm_pop_ds(vm, out, sizeof(out));
    assert(len == 6);
    assert(memcmp(out, "Active", 6) == 0);
    
    /* Test failing locktime: target is 13 hours -> 13 * 36000 = 468000. CLTV should fail. */
    uint32_t invalid_locktime = 468000;
    uint8_t bytecode_fail[] = {
        0x01, 0x04, (uint8_t)(invalid_locktime & 0xFF), (uint8_t)((invalid_locktime >> 8) & 0xFF), (uint8_t)((invalid_locktime >> 16) & 0xFF), (uint8_t)((invalid_locktime >> 24) & 0xFF),
        0x04
    };
    assert(btc_rails_vm_deploy_yul(vm, bytecode_fail, sizeof(bytecode_fail)) == 0);
    
    printf("[Test] Yul interpreter and locktime verification passed.\n");
    fflush(stdout);
}

int main() {
    printf("=== AUNCIENT BTC RAILS VM TESTING ===\n");
    fflush(stdout);
    BtcRailsVm *vm = btc_rails_vm_init(262144);
    assert(vm != NULL);
    
    run_stack_tests(vm);
    run_sdr_tests(vm);
    run_tree_and_dat_tests(vm);
    run_yul_interpreter_tests(vm);
    
    btc_rails_vm_free(vm);
    printf("=== ALL VM TESTS PASSED ===\n");
    fflush(stdout);
    return 0;
}
