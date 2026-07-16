#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_strategy_lang.h"
#include "tsfi_anvil_vm.h"

int main(void) {
    printf("[Strategy Lang] Starting strategy script parser tests...\n");
    fflush(stdout);

    TSFiStrategyVM vm;
    tsfi_strategy_vm_init(&vm);

    TSFiPriorityQueue pq;
    tsfi_priority_queue_init(&pq);

    tsfi_priority_queue_push(&pq, 10, 30, "LOW_PRIO");
    tsfi_priority_queue_push(&pq, 2, 31, "HIGH_PRIO");

    // Execute script: set depth scale to 2, abductive scale to 5, and execute EVAL re-prioritization
    const char *script = "SET depth 2; SET abductive 5; EVAL;";
    int res = tsfi_strategy_vm_execute(&vm, &pq, script, NULL);
    assert(res == 0);
    printf("  [Strategy Script] Executed eval: %d (Expected 1), Depth Scale: %d (Expected 2)\n", 
           vm.executed_evals, vm.depth_priority_scale);
    fflush(stdout);
    assert(vm.executed_evals == 1);
    assert(vm.depth_priority_scale == 2);
    assert(vm.abductive_priority_scale == 5);

    // Pop and verify new priorities:
    // Item 1: (2 * 2) + 5 = 9
    // Item 2: (10 * 2) + 5 = 25
    TSFiQueueItem item;
    res = tsfi_priority_queue_pop(&pq, &item);
    assert(res == 0);
    printf("  [PQ Pop 1] Keycode: %d, Priority: %d (Expected Priority: 9)\n", item.keycode, item.priority);
    fflush(stdout);
    assert(item.priority == 9);

    res = tsfi_priority_queue_pop(&pq, &item);
    assert(res == 0);
    printf("  [PQ Pop 2] Keycode: %d, Priority: %d (Expected Priority: 25)\n", item.keycode, item.priority);
    fflush(stdout);
    assert(item.priority == 25);

    // Push new subgoals to verify PRUNE and WEIGHT
    tsfi_priority_queue_push(&pq, 15, 33, "SUBGOAL_C");
    tsfi_priority_queue_push(&pq, 50, 34, "SUBGOAL_D");

    // Execute script: set priority of 34 to 3, then prune anything exceeding 20
    const char *prune_script = "WEIGHT 34 3; PRUNE 20;";
    res = tsfi_strategy_vm_execute(&vm, &pq, prune_script, NULL);
    assert(res == 0);

    // Verify item 33 (prio 15 <= 20) and item 34 (prio 3 <= 20) are kept
    assert(pq.size == 2);
    res = tsfi_priority_queue_pop(&pq, &item);
    assert(res == 0 && item.keycode == 34 && item.priority == 3);
    printf("  [Strategy Prune/Weight] Popped priority 3 keycode: %d (Expected: 34)\n", item.keycode);
    fflush(stdout);

    // 3. Verify Bytecode execution pathway
    tsfi_priority_queue_push(&pq, 10, 40, "BC_LOW");
    tsfi_priority_queue_push(&pq, 2, 41, "BC_HIGH");

    // Bytecode stream:
    // 0x01, 2 (SET_DEPTH = 2)
    // 0x02, 5 (SET_ABDUCTIVE = 5)
    // 0x03    (OP_EVAL)
    uint8_t bc[5] = { 0x01, 2, 0x02, 5, 0x03 };
    res = tsfi_strategy_vm_execute_bytecode(&vm, &pq, bc, 5, NULL);
    assert(res == 0);

    // Verify popped priorities:
    // Item 41: (2 * 2) + 5 = 9
    // Item 40: (10 * 2) + 5 = 25
    res = tsfi_priority_queue_pop(&pq, &item);
    assert(res == 0 && item.keycode == 41 && item.priority == 9);
    printf("  [Strategy Bytecode] Popped bytecode priority 9 keycode: %d (Expected: 41)\n", item.keycode);
    fflush(stdout);

    // 4. Verify script compiling tool
    uint8_t compiled_bc[32];
    int bc_len = 0;
    res = tsfi_strategy_compile_script("SET depth 3; SET abductive 10; EVAL;", compiled_bc, 32, &bc_len);
    assert(res == 0);
    assert(bc_len == 5);
    assert(compiled_bc[0] == 0x01 && compiled_bc[1] == 3);
    assert(compiled_bc[2] == 0x02 && compiled_bc[3] == 10);
    assert(compiled_bc[4] == 0x03);
    printf("  [Strategy Compiler] Compiled script successfully to 5 bytecode instructions.\n");
    fflush(stdout);

    // 5. Verify Turing Complete register additions and jumps
    // Script: SET_REG R0 5; SET_REG R1 10; ADD R0 R1; (R0 becomes 15)
    uint8_t turing_bc[32];
    int turing_len = 0;
    res = tsfi_strategy_compile_script("SET_REG R0 5; SET_REG R1 10; ADD R0 R1;", turing_bc, 32, &turing_len);
    assert(res == 0);
    
    TSFiStrategyVM turing_vm;
    tsfi_strategy_vm_init(&turing_vm);
    res = tsfi_strategy_vm_execute_bytecode(&turing_vm, NULL, turing_bc, turing_len, NULL);
    assert(res == 0);
    printf("  [Strategy Turing Registers] R0=%d (Expected 15)\n", turing_vm.registers[0]);
    fflush(stdout);
    assert(turing_vm.registers[0] == 15);

    // 6. Verify State queries: GET_PRIO and GET_SIZE
    tsfi_priority_queue_push(&pq, 45, 99, "QUERY_TARGET");
    
    uint8_t query_bc[32];
    int query_len = 0;
    res = tsfi_strategy_compile_script("GET_SIZE R2; GET_PRIO R3 99;", query_bc, 32, &query_len);
    assert(res == 0);
    
    TSFiStrategyVM query_vm;
    tsfi_strategy_vm_init(&query_vm);
    res = tsfi_strategy_vm_execute_bytecode(&query_vm, &pq, query_bc, query_len, NULL);
    assert(res == 0);
    printf("  [Strategy Queries] PQ size: %d, Keycode 99 priority: %d\n", query_vm.registers[2], query_vm.registers[3]);
    fflush(stdout);
    assert(query_vm.registers[2] > 0);
    assert(query_vm.registers[3] == 45);

    // 7. Verify loop modifier: LOOP_UNTIL_EMPTY
    // Bytecode checks if queue is non-empty, Jumps to index 0.
    // We will compile a script: "LOOP_UNTIL_EMPTY 0;"
    uint8_t loop_bc[32];
    int loop_len = 0;
    res = tsfi_strategy_compile_script("LOOP_UNTIL_EMPTY 0;", loop_bc, 32, &loop_len);
    assert(res == 0);
    assert(loop_bc[0] == 0x17 && loop_bc[1] == 0);

    // 8. Verify Telemetry Broadcast
    TSFiStrategyTelemetry telemetry;
    memset(&telemetry, 0, sizeof(telemetry));
    tsfi_strategy_vm_broadcast(&query_vm, &pq, &telemetry);
    assert(telemetry.queue_size == pq.size);
    assert(telemetry.registers[2] == query_vm.registers[2]);
    printf("  [Strategy Telemetry] Broadcast Verified. Queue Size: %d\n", telemetry.queue_size);
    fflush(stdout);

    // 9. Verify C-like syntax compiler translation
    uint8_t c_like_bc[32];
    int c_like_len = 0;
    res = tsfi_strategy_compile_script("depth = 4; R0 = 15; R0 = R0 + R1; if (R0 == R1) jump(24); eval();", c_like_bc, 32, &c_like_len);
    assert(res == 0);
    assert(c_like_bc[0] == 0x01 && c_like_bc[1] == 4); // depth = 4
    assert(c_like_bc[2] == 0x14 && c_like_bc[3] == 0 && c_like_bc[4] == 15); // R0 = 15
    assert(c_like_bc[5] == 0x10 && c_like_bc[6] == 0 && c_like_bc[7] == 1); // ADD R0 R1
    assert(c_like_bc[8] == 0x12 && c_like_bc[9] == 24 && c_like_bc[10] == 0 && c_like_bc[11] == 1); // JEQ 24 R0 R1
    assert(c_like_bc[12] == 0x03); // EVAL
    printf("  [Strategy C-like Parser] Compiled and verified statement tree successfully.\n");
    fflush(stdout);

    // 10. Verify semantic logical subgoal lookups
    TSFiAnvilVM logic_vm;
    tsfi_anvil_vm_init(&logic_vm);
    tsfi_anvil_vm_bind(&logic_vm, "svdag/relation/0", "TRUE");

    uint8_t logic_bc[32];
    int logic_len = 0;
    res = tsfi_strategy_compile_script("get_logic(\"svdag/relation/0\", R1);", logic_bc, 32, &logic_len);
    assert(res == 0);

    TSFiStrategyVM strategy_vm;
    tsfi_strategy_vm_init(&strategy_vm);
    res = tsfi_strategy_vm_execute_bytecode(&strategy_vm, NULL, logic_bc, logic_len, &logic_vm);
    assert(res == 0);
    printf("  [Strategy Logic Bindings] R1=%d (Expected 1/TRUE)\n", strategy_vm.registers[1]);
    fflush(stdout);
    assert(strategy_vm.registers[1] == 1);

    // 11. Verify COBOL-style syntax compiler translation
    uint8_t cobol_bc[64];
    int cobol_len = 0;
    res = tsfi_strategy_compile_script("SET_REG R0 10; SET_REG R1 5; SUBTRACT R1 FROM R0; ADD R1 TO R0;", cobol_bc, 64, &cobol_len);
    assert(res == 0);
    
    TSFiStrategyVM cobol_vm;
    tsfi_strategy_vm_init(&cobol_vm);
    res = tsfi_strategy_vm_execute_bytecode(&cobol_vm, NULL, cobol_bc, cobol_len, NULL);
    assert(res == 0);
    // R0 = 10 -> R0 = R0 - R1 (5) -> R0 = R0 + R1 (10)
    assert(cobol_vm.registers[0] == 10);
    printf("  [Strategy COBOL Parser] Compiled and verified COBOL ADD/SUBTRACT successfully.\n");
    fflush(stdout);

    printf("[PASS] Strategy script execution verified successfully!\n");
    fflush(stdout);
    return 0;
}
