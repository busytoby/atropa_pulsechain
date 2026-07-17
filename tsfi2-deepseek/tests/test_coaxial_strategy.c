#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_coaxial_strategy.h"

int main(void) {
    printf("[Coaxial Strategy] Starting coaxial strategy agent dispatch tests...\n");
    fflush(stdout);

    // Context Setup
    TSFiSHMBridge bridge;
    tsfi_shm_bridge_init(&bridge);

    TSFiAnvilVM vm;
    tsfi_anvil_vm_init(&vm);

    TSFiOTAccumulator acc;
    tsfi_ot_accumulator_init(&acc);

    TSFiCoaxialObserver obs;
    tsfi_coaxial_observer_init(&obs, &bridge, &vm, &acc);

    TsfiZmmVmState zmm;
    tsfi_zmm_vm_init(&zmm);

    TSFiCoaxialStrategyAgent agent;
    tsfi_coaxial_strategy_agent_init(&agent, &obs, &zmm);

    // 1. Trigger & Collect Logic Event (Default priority 15)
    tsfi_anvil_vm_bind(&vm, "svdag/relation/0", "TRUE");
    int collected = tsfi_coaxial_strategy_agent_collect(&agent);
    assert(collected == 1);

    // 2. Trigger & Collect Hardware Event (Default priority 5)
    tsfi_shm_bridge_write(&bridge, 30);
    collected = tsfi_coaxial_strategy_agent_collect(&agent);
    assert(collected == 1);
    assert(agent.scheduler_queue.size == 2);

    // 3. Dispatch using Strategy script: "SET abductive 10; EVAL;"
    // path_B (Hardware): 5 * 1 + 10 = 15
    // path_A (Logic): 15 * 1 + 10 = 25
    // Hardware keycode 30 (priority 15) should be dispatched first!
    int status = tsfi_coaxial_strategy_agent_dispatch(&agent, "SET abductive 10; EVAL;");
    printf("  [Strategy Dispatch 1] Status: %d (Expected 1), Payload: %s\n", status, agent.last_payload);
    fflush(stdout);
    assert(status == 1);
    assert(strstr(agent.last_payload, "30 15") != NULL);

    // 4. Compile and verify Decision Table strategy
    tsfi_decision_table table;
    memset(&table, 0, sizeof(table));
    strcpy(table.condition_op[0], "==");
    table.condition_reg_a[0] = 0;
    table.condition_reg_b[0] = 1;
    strcpy(table.action_op[0], "MOVE");
    table.action_reg[0] = 2;
    table.action_val[0] = 88;
    table.rule_count = 1;
    
    uint8_t agent_bytecode[64];
    int compiled_len = tsfi_coaxial_strategy_agent_compile_decision_table(&agent, &table, agent_bytecode, 64);
    assert(compiled_len > 0);
    
    TSFiStrategyVM strategy_vm;
    tsfi_strategy_vm_init(&strategy_vm);
    strategy_vm.registers[0] = 100;
    strategy_vm.registers[1] = 100;
    int exec_res = tsfi_strategy_vm_execute_bytecode(&strategy_vm, NULL, agent_bytecode, compiled_len, NULL);
    assert(exec_res == 0);
    assert(strategy_vm.registers[2] == 88);
    printf("  [PASS] Coaxial agent compiled decision table verified.\n");

    // Clean up
    tsfi_zmm_vm_destroy(&zmm);

    printf("[PASS] Coaxial strategy agent dispatch verified successfully!\n");
    fflush(stdout);
    return 0;
}
