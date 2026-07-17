#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_anvil_vm.h"
#include "tsfi_strategy_lang.h"

// Load file contents into a dynamically allocated buffer
static char* load_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        printf("Error: Could not open strategy script file at %s\n", path);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = malloc(size + 1);
    if (buf) {
        size_t read_bytes = fread(buf, 1, size, f);
        buf[read_bytes] = '\0';
    }
    fclose(f);
    return buf;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT DYSNOMIA DOMAIN STRATEGY VERIFIER\n");
    printf("=============================================================\n");

    TSFiStrategyVM vm;
    TSFiPriorityQueue pq;

    // 1. Test abductive_weight.strategy compilation and execution
    char *strat1 = load_file("../solidity/dysnomia/domain/strategies/abductive_weight.strategy");
    assert(strat1 != NULL);

    printf("1. Compiling abductive_weight.strategy...\n");
    uint8_t bc1[512];
    int len1 = 0;
    int res = tsfi_strategy_compile_script(strat1, bc1, 512, &len1);
    assert(res == 0);
    printf("   ✓ Compiled successfully. Bytecode length: %d\n", len1);

    // Initialize Priority Queue with coordinates for testing
    tsfi_priority_queue_init(&pq);
    tsfi_priority_queue_push(&pq, 10, 32, "key_32"); // Keycode 32, initial priority 10
    tsfi_priority_queue_push(&pq, 15, 30, "key_30"); // Keycode 30, initial priority 15

    tsfi_strategy_vm_init(&vm);
    printf("2. Executing abductive_weight.strategy bytecode...\n");
    res = tsfi_strategy_vm_execute_bytecode(&vm, &pq, bc1, len1, NULL);
    assert(res == 0);

    // After abductive_weight.strategy runs:
    // depth = 1 (vm.depth_priority_scale = 1)
    // R0 = 10, R1 = 10. (R0 == R1) triggers JEQ to target_pc 16.
    // Let's verify R0 and R1 registers
    printf("   ✓ Depth scale: %d (Expected: 1)\n", vm.depth_priority_scale);
    printf("   ✓ Register R0: %d, R1: %d\n", vm.registers[0], vm.registers[1]);
    assert(vm.depth_priority_scale == 1);
    assert(vm.registers[0] == 10);
    assert(vm.registers[1] == 10);

    // 2. Test depth_bias.strategy compilation and execution
    char *strat2 = load_file("../solidity/dysnomia/domain/strategies/depth_bias.strategy");
    assert(strat2 != NULL);

    printf("3. Compiling depth_bias.strategy...\n");
    uint8_t bc2[512];
    int len2 = 0;
    res = tsfi_strategy_compile_script(strat2, bc2, 512, &len2);
    assert(res == 0);
    printf("   ✓ Compiled successfully. Bytecode length: %d\n", len2);

    tsfi_priority_queue_init(&pq);
    tsfi_priority_queue_push(&pq, 10, 100, "node_100");
    tsfi_priority_queue_push(&pq, 20, 200, "node_200");

    tsfi_strategy_vm_init(&vm);
    printf("4. Executing depth_bias.strategy bytecode...\n");
    res = tsfi_strategy_vm_execute_bytecode(&vm, &pq, bc2, len2, NULL);
    assert(res == 0);

    // depth_bias.strategy:
    // depth = 3
    // R0 = 5, R1 = 2
    // R0 = R0 + R1 (7)
    // eval() -> adjusts priority to: (priority * 3) + 1 (since abductive scale defaults to 1)
    // prune(15) -> leaves items with priority <= 15
    // node_100: (10 * 3) + 1 = 31 > 15 -> pruned
    // node_200: (20 * 3) + 1 = 61 > 15 -> pruned
    printf("   ✓ Depth scale: %d (Expected: 3)\n", vm.depth_priority_scale);
    printf("   ✓ Register R0: %d (Expected: 7)\n", vm.registers[0]);
    printf("   ✓ Queue Size: %d (Expected: 0 after prune)\n", pq.size);
    assert(vm.depth_priority_scale == 3);
    assert(vm.registers[0] == 7);
    assert(pq.size == 0);

    free(strat1);
    free(strat2);

    // 3. Test sort_generator.strategy
    char *strat3 = load_file("../solidity/dysnomia/domain/strategies/sort_generator.strategy");
    assert(strat3 != NULL);
    printf("5. Compiling sort_generator.strategy...\n");
    uint8_t bc3[512];
    int len3 = 0;
    res = tsfi_strategy_compile_script(strat3, bc3, 512, &len3);
    assert(res == 0);
    printf("   ✓ Compiled successfully. Bytecode length: %d\n", len3);

    tsfi_priority_queue_init(&pq);
    tsfi_priority_queue_push(&pq, 10, 30, "key_30");
    tsfi_priority_queue_push(&pq, 15, 32, "key_32");
    tsfi_strategy_vm_init(&vm);
    res = tsfi_strategy_vm_execute_bytecode(&vm, &pq, bc3, len3, NULL);
    assert(res == 0);
    // after sort_generator:
    // weight(30, 10) -> key_30 priority becomes 10
    // weight(32, 12) -> key_32 priority becomes 12
    // eval() -> key_30: (10 * 2) + 3 = 23, key_32: (12 * 2) + 3 = 27
    // prune(25) -> key_32 is pruned (27 > 25), key_30 remains (23 <= 25)
    assert(pq.size == 1);
    assert(pq.items[0].keycode == 30);
    free(strat3);

    // 4. Test report_writer.strategy
    char *strat4 = load_file("../solidity/dysnomia/domain/strategies/report_writer.strategy");
    assert(strat4 != NULL);
    printf("6. Compiling report_writer.strategy...\n");
    uint8_t bc4[512];
    int len4 = 0;
    res = tsfi_strategy_compile_script(strat4, bc4, 512, &len4);
    assert(res == 0);
    printf("   ✓ Compiled successfully. Bytecode length: %d\n", len4);

    tsfi_priority_queue_init(&pq);
    tsfi_priority_queue_push(&pq, 2, 40, "key_40");
    tsfi_strategy_vm_init(&vm);
    res = tsfi_strategy_vm_execute_bytecode(&vm, &pq, bc4, len4, NULL);
    assert(res == 0);
    // after report_writer:
    // depth = 4, abductive = 1
    // R0 = 15 - 12 = 3
    // eval() -> key_40: (2 * 4) + 1 = 9
    // prune(10) -> key_40 remains (9 <= 10)
    assert(pq.size == 1);
    assert(vm.registers[0] == 3);
    free(strat4);

    // 5. Test two_phase_commit.strategy (Scenario 140)
    char *strat5 = load_file("../solidity/dysnomia/domain/strategies/two_phase_commit.strategy");
    assert(strat5 != NULL);
    printf("7. Compiling two_phase_commit.strategy...\n");
    uint8_t bc5[512];
    int len5 = 0;
    res = tsfi_strategy_compile_script(strat5, bc5, 512, &len5);
    assert(res == 0);
    printf("   ✓ Compiled successfully. Bytecode length: %d\n", len5);

    tsfi_priority_queue_init(&pq);
    tsfi_priority_queue_push(&pq, 10, 10, "node_10");
    tsfi_priority_queue_push(&pq, 20, 20, "node_20");
    tsfi_strategy_vm_init(&vm);
    res = tsfi_strategy_vm_execute_bytecode(&vm, &pq, bc5, len5, NULL);
    assert(res == 0);
    // after two_phase_commit:
    // R0 = 1 (TXSTATE), R1 = 2 (PARTCNT)
    // weight(10, 1) -> node_10 priority becomes 1
    // weight(20, 1) -> node_20 priority becomes 1
    // eval() -> node_10: (1 * 2) + 1 = 3, node_20: (1 * 2) + 1 = 3
    // prune(10) -> both remain (3 <= 10)
    assert(pq.size == 2);
    assert(vm.registers[0] == 1);
    assert(vm.registers[1] == 2);
    free(strat5);

    // 6. Test crypto_subsystem.strategy (Scenario 142)
    char *strat6 = load_file("../solidity/dysnomia/domain/strategies/crypto_subsystem.strategy");
    assert(strat6 != NULL);
    printf("8. Compiling crypto_subsystem.strategy...\n");
    uint8_t bc6[512];
    int len6 = 0;
    res = tsfi_strategy_compile_script(strat6, bc6, 512, &len6);
    assert(res == 0);
    printf("   ✓ Compiled successfully. Bytecode length: %d\n", len6);

    tsfi_priority_queue_init(&pq);
    tsfi_priority_queue_push(&pq, 10, 40, "key_40");
    tsfi_strategy_vm_init(&vm);
    res = tsfi_strategy_vm_execute_bytecode(&vm, &pq, bc6, len6, NULL);
    assert(res == 0);
    // after crypto_subsystem:
    // R0 = 1 (SECURE), R1 = 1 (CRYPTO)
    // R0 == R1 triggers jump to 16. Jump target bypasses WEIGHT 40 1.
    // eval() -> key_40: (10 * 4) + 1 = 41
    // prune(10) -> key_40 is pruned (41 > 10)
    assert(pq.size == 0);
    assert(vm.registers[0] == 1);
    assert(vm.registers[1] == 1);
    free(strat6);

    // 7. Test latency_monitor.strategy (Scenario 171)
    char *strat7 = load_file("../solidity/dysnomia/domain/strategies/latency_monitor.strategy");
    assert(strat7 != NULL);
    printf("9. Compiling latency_monitor.strategy...\n");
    uint8_t bc7[512];
    int len7 = 0;
    res = tsfi_strategy_compile_script(strat7, bc7, 512, &len7);
    assert(res == 0);
    printf("   ✓ Compiled successfully. Bytecode length: %d\n", len7);

    tsfi_priority_queue_init(&pq);
    tsfi_priority_queue_push(&pq, 5, 32, "key_32");
    tsfi_strategy_vm_init(&vm);
    res = tsfi_strategy_vm_execute_bytecode(&vm, &pq, bc7, len7, NULL);
    assert(res == 0);
    // after latency_monitor:
    // R0 = 20 (R0 = 15, R1 = 5, ADD R1 TO R0)
    // eval() -> key_32: (5 * 2) + 1 = 11
    // prune(20) -> key_32 remains (11 <= 20)
    assert(pq.size == 1);
    assert(vm.registers[0] == 20);
    free(strat7);

    printf("\n=============================================================\n");
    printf("ALL AUNCIENT DOMAIN STRATEGY SCRIPT VERIFICATIONS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
