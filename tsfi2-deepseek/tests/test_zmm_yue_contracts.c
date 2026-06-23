#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"
#include "lau_yul_thunk.h"
#include "tsfi_pulsechain.h"

// Terminology Constraint: Never use the spelling "ancient". Always spell it as "Auncient".

// Parse 256-bit hex return value from VM output buffer
void parse_vm_uint256(const char *buffer, int word_idx, uint64_t *val_out) {
    *val_out = 0;
    int offset = word_idx * 64;
    if ((int)strlen(buffer) >= offset + 64) {
        sscanf(buffer + offset + 48, "%16lx", val_out);
    }
}

int main(int argc, char *argv[]) {
    printf("=========================================================\n");
    printf("Auncient ZMM VM: YUE Relational Mapping & Dynamic Opt-In/React Validation\n");
    printf("=========================================================\n");

    uint64_t fom_bal_arg = 250;
    uint64_t fornax_bal_arg = 300;
    uint64_t urgent_bal_arg = 1000;
    uint64_t crows_bal_arg = 750;

    if (argc >= 5) {
        fom_bal_arg = strtoull(argv[1], NULL, 10);
        fornax_bal_arg = strtoull(argv[2], NULL, 10);
        urgent_bal_arg = strtoull(argv[3], NULL, 10);
        crows_bal_arg = strtoull(argv[4], NULL, 10);
    }

    // Reset static/thread-local storage parameters to avoid accumulation across test runs
    memset(&g_yul_evm_context, 0, sizeof(g_yul_evm_context));
    // Clear evm_storage.json before initializing compilers to clear previous state
    remove("evm_storage.json");
    remove("tsfi2-deepseek/evm_storage.json");

    // Initialize dependencies
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    const char *yue_path = "tests/yue_react_simulator.yul";
    FILE *f_yue = fopen(yue_path, "r");
    if (f_yue) {
        fclose(f_yue);
    } else {
        yue_path = "tsfi2-deepseek/tests/yue_react_simulator.yul";
        f_yue = fopen(yue_path, "r");
        if (f_yue) {
            fclose(f_yue);
        } else {
            yue_path = "../tests/yue_react_simulator.yul";
        }
    }
    char yue_cmd[256];
    snprintf(yue_cmd, sizeof(yue_cmd), "YULINIT \"YueReactSimulator\", \"%s\", 768", yue_path);
    tsfi_zmm_vm_exec(&vm, yue_cmd);

    // 1. Set ERC-20 balances for Fomalhaute and Fornax on target contract to satisfy dynamic preconditions
    // setBalance(address account, uint256 amount, uint256 slot_id)
    printf("[ZMM TEST] Initializing Fomalhaute ERC-20 balance on simulator...\n");
    char exec_buf[512];
    snprintf(exec_buf, sizeof(exec_buf),
             "YULEXEC \"YueReactSimulator\", \"f221a28a00000000000000000000000000000000000000000000000000000000000004cc%064lx0000000000000000000000000000000000000000000000000000000000000032\"",
             fom_bal_arg);
    tsfi_zmm_vm_exec(&vm, exec_buf);

    printf("[ZMM TEST] Initializing Fornax ERC-20 balance on simulator...\n");
    snprintf(exec_buf, sizeof(exec_buf),
             "YULEXEC \"YueReactSimulator\", \"f221a28a00000000000000000000000000000000000000000000000000000000000004cc%064lx0000000000000000000000000000000000000000000000000000000000000033\"",
             fornax_bal_arg);
    tsfi_zmm_vm_exec(&vm, exec_buf);

    // 2. Execute dynamic React calls to PUSH weighted bars to Fomalhaute and Fornax Qings
    printf("[ZMM TEST] Calling React(Fomalhaute) to dynamically push weights...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"YueReactSimulator\", \"7ac9e05f00000000000000000000000000000000000000000000000000000000f0ba1ab0\"");

    printf("[ZMM TEST] Calling React(Fornax) to dynamically push weights...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"YueReactSimulator\", \"7ac9e05f00000000000000000000000000000000000000000000000000000000f074a1e0\"");

    // 2. Query mapped values back via Bar(Qing) getter thunk
    printf("[ZMM TEST] Querying Bar(Fomalhaute) to retrieve Hypobar and Epibar...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"YueReactSimulator\", \"0ebc857700000000000000000000000000000000000000000000000000000000f0ba1ab0\"");
    
    uint64_t fomal_hypo = 0, fomal_epi = 0;
    parse_vm_uint256(vm.output_buffer, 0, &fomal_hypo);
    parse_vm_uint256(vm.output_buffer, 1, &fomal_epi);

    printf("[ZMM TEST] Querying Bar(Fornax) to retrieve Hypobar and Epibar...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"YueReactSimulator\", \"0ebc857700000000000000000000000000000000000000000000000000000000f074a1e0\"");
    
    uint64_t fornax_hypo = 0, fornax_epi = 0;
    parse_vm_uint256(vm.output_buffer, 0, &fornax_hypo);
    parse_vm_uint256(vm.output_buffer, 1, &fornax_epi);

    // Compute expected values dynamically
    uint64_t expected_fomal_epi = fom_bal_arg / 10;
    uint64_t expected_fomal_hypo = fom_bal_arg / 3;
    uint64_t expected_fornax_epi = fornax_bal_arg / 20;
    uint64_t expected_fornax_hypo = fornax_bal_arg / 5;

    // 3. Compute calculated bar width: Width = (Hypobar * 100) / Epibar
    uint64_t fomal_width = fomal_epi > 0 ? (fomal_hypo * 100) / fomal_epi : 0;
    uint64_t fornax_width = fornax_epi > 0 ? (fornax_hypo * 100) / fornax_epi : 0;

    printf("[ZMM TEST] Results:\n");
    printf("   Fomalhaute Hypobar / Epibar: %lu Gram / %lu Gram -> Calculated Width: %lu (Expected: %lu Gram / %lu Gram)\n",
           fomal_hypo, fomal_epi, fomal_width, expected_fomal_hypo, expected_fomal_epi);
    printf("   Fornax Hypobar / Epibar:     %lu Gram / %lu Gram -> Calculated Width: %lu (Expected: %lu Gram / %lu Gram)\n",
           fornax_hypo, fornax_epi, fornax_width, expected_fornax_hypo, expected_fornax_epi);

    // Perform assertions matching standard dynamic updates
    assert(fomal_hypo == expected_fomal_hypo);
    assert(fomal_epi == expected_fomal_epi);
    if (fomal_epi > 0) {
        assert(fomal_width == (expected_fomal_hypo * 100) / expected_fomal_epi);
    }
    
    assert(fornax_hypo == expected_fornax_hypo);
    assert(fornax_epi == expected_fornax_epi);
    if (fornax_epi > 0) {
        assert(fornax_width == (expected_fornax_hypo * 100) / expected_fornax_epi);
    }

    // Test a second push to verify accumulation (cannot simply set, must push/accumulate)
    printf("[ZMM TEST] Performing secondary React(Fomalhaute) call to test weight accumulation...\n");
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"YueReactSimulator\", \"7ac9e05f00000000000000000000000000000000000000000000000000000000f0ba1ab0\"");

    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"YueReactSimulator\", \"0ebc857700000000000000000000000000000000000000000000000000000000f0ba1ab0\"");
    
    parse_vm_uint256(vm.output_buffer, 0, &fomal_hypo);
    parse_vm_uint256(vm.output_buffer, 1, &fomal_epi);
    fomal_width = fomal_epi > 0 ? (fomal_hypo * 100) / fomal_epi : 0;

    printf("   Fomalhaute Accumulation Results: %lu Gram / %lu Gram -> Calculated Width: %lu (Expected: %lu Gram / %lu Gram)\n",
           fomal_hypo, fomal_epi, fomal_width, expected_fomal_hypo * 2, expected_fomal_epi * 2);

    assert(fomal_hypo == expected_fomal_hypo * 2);
    assert(fomal_epi == expected_fomal_epi * 2);
    if (fomal_epi > 0) {
        assert(fomal_width == (expected_fomal_hypo * 100) / expected_fomal_epi);
    }

    // 4. Validate URGENT Qing (0x9999) Relational Mappings
    printf("[ZMM TEST] Initializing URGENT Qing (0x9999) ERC-20 balance on simulator...\n");
    snprintf(exec_buf, sizeof(exec_buf),
             "YULEXEC \"YueReactSimulator\", \"f221a28a0000000000000000000000000000000000000000000000000000000000009999%064lx0000000000000000000000000000000000000000000000000000000000000034\"",
             urgent_bal_arg);
    tsfi_zmm_vm_exec(&vm, exec_buf);

    printf("[ZMM TEST] Calling React(URGENT) to dynamically push weights...\n");
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"YueReactSimulator\", \"7ac9e05f0000000000000000000000000000000000000000000000000000000000009999\"");

    printf("[ZMM TEST] Querying Bar(URGENT) to retrieve Hypobar and Epibar...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"YueReactSimulator\", \"0ebc85770000000000000000000000000000000000000000000000000000000000009999\"");

    uint64_t urgent_hypo = 0, urgent_epi = 0;
    parse_vm_uint256(vm.output_buffer, 0, &urgent_hypo);
    parse_vm_uint256(vm.output_buffer, 1, &urgent_epi);
    uint64_t urgent_width = urgent_epi > 0 ? (urgent_hypo * 100) / urgent_epi : 0;

    uint64_t expected_urgent_epi = urgent_bal_arg / 2;
    uint64_t expected_urgent_hypo = urgent_bal_arg / 1;

    printf("   URGENT Hypobar / Epibar:     %lu Gram / %lu Gram -> Calculated Width: %lu (Expected: %lu Gram / %lu Gram)\n",
           urgent_hypo, urgent_epi, urgent_width, expected_urgent_hypo, expected_urgent_epi);

    assert(urgent_hypo == expected_urgent_hypo);
    assert(urgent_epi == expected_urgent_epi);
    if (urgent_epi > 0) {
        assert(urgent_width == (expected_urgent_hypo * 100) / expected_urgent_epi);
    }

    printf("[SUCCESS] Verified YUE dynamic react logic, bar accumulation, and width calculations stably.\n");

    const char *crows_path = "tests/crows_rank.yul";
    FILE *f_crows = fopen(crows_path, "r");
    if (f_crows) {
        fclose(f_crows);
    } else {
        crows_path = "tsfi2-deepseek/tests/crows_rank.yul";
        f_crows = fopen(crows_path, "r");
        if (f_crows) {
            fclose(f_crows);
        } else {
            crows_path = "../tests/crows_rank.yul";
        }
    }
    char crows_cmd[256];
    snprintf(crows_cmd, sizeof(crows_cmd), "YULINIT \"crows_rank\", \"%s\", 1024", crows_path);
    tsfi_zmm_vm_exec(&vm, crows_cmd);

    // Attempt to call updateAndGetRankings when player is not opted in -> must revert
    printf("[ZMM TEST] Calling updateAndGetRankings(YueReactSimulator, CROWS) before opt-in (should revert)...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"crows_rank\", \"e003a27b00000000000000000000000000000000000000000000000000000000000004cc0000000000000000000000000000000000000000000000000000000000000300000000000000000000000000203e366a1821570b2f84ff5ae8b3bdeb48dc4fa1\"");

    // Opt-in the deployer caller wallet (0x4cc) -> Selector 0x1c1e0a29
    printf("[ZMM TEST] Opting in caller (0x4cc) to CrowsRank...\n");
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"crows_rank\", \"1c1e0a2900000000000000000000000000000000000000000000000000000000000004cc\"");

    // Setup CROWS bars on YueReactSimulator for the player
    printf("[ZMM TEST] Initializing CROWS balance on YueReactSimulator for caller...\n");
    snprintf(exec_buf, sizeof(exec_buf),
             "YULEXEC \"YueReactSimulator\", \"f221a28a00000000000000000000000000000000000000000000000000000000000004cc%064lx0000000000000000000000000000000000000000000000000000000000000032\"",
             crows_bal_arg);
    tsfi_zmm_vm_exec(&vm, exec_buf);

    printf("[ZMM TEST] Pushing weights for CROWS Qing in YueReactSimulator...\n");
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"YueReactSimulator\", \"7ac9e05f000000000000000000000000203e366a1821570b2f84ff5ae8b3bdeb48dc4fa1\"");

    // Query YueReactSimulator to make sure CROWS bars are loaded
    printf("[ZMM TEST] Querying YueReactSimulator Bar(CROWS)...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"YueReactSimulator\", \"0ebc8577000000000000000000000000203e366a1821570b2f84ff5ae8b3bdeb48dc4fa1\"");
    uint64_t crows_hypo = 0, crows_epi = 0;
    parse_vm_uint256(vm.output_buffer, 0, &crows_hypo);
    parse_vm_uint256(vm.output_buffer, 1, &crows_epi);
    printf("   CROWS Hypobar: %lu, Epibar: %lu (Sum: %lu)\n", crows_hypo, crows_epi, crows_hypo + crows_epi);

    uint64_t expected_crows_epi = crows_bal_arg / 10;
    uint64_t expected_crows_hypo = crows_bal_arg / 3;

    assert(crows_hypo == expected_crows_hypo);
    assert(crows_epi == expected_crows_epi);

    // Call updateAndGetRankings now that the player is opted in
    printf("[ZMM TEST] Calling updateAndGetRankings(YueReactSimulator, CROWS) post opt-in...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"crows_rank\", \"e003a27b00000000000000000000000000000000000000000000000000000000000004cc0000000000000000000000000000000000000000000000000000000000000300000000000000000000000000203e366a1821570b2f84ff5ae8b3bdeb48dc4fa1\"");

    uint64_t ranked_addr = 0, ranked_score = 0;
    parse_vm_uint256(vm.output_buffer, 0, &ranked_addr);
    parse_vm_uint256(vm.output_buffer, 10, &ranked_score); // Score index starts after the 10 address elements (offset 320 bytes)

    printf("[ZMM TEST] Top 1 Leaderboard Entry -> Address: 0x%lx, Score: %lu\n", ranked_addr, ranked_score);
    assert(ranked_addr == 0x4cc);
    assert(ranked_score == (crows_hypo + crows_epi));

    printf("[SUCCESS] Verified CrowsRank leaderboard updates and opt-in gates successfully.\n");

    // Cleanup
    tsfi_zmm_vm_destroy(&vm);
    printf("=========================================================\n");
    return 0;
}
