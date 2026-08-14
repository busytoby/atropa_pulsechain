#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_deepseek_cordis_advanced.h"

int main(void) {
    printf("=== TESTING ADVANCED DEEPSEEK CORDIS EXTENSIONS ===\n");

    // 1. Test Spatiotemporal Session Tree (Resume, Fork, Replay)
    tsfi_cordis_session_tree_t tree_main;
    assert(tsfi_cordis_session_tree_init(&tree_main, 1001));

    assert(tsfi_cordis_session_record_turn(&tree_main, "init zmm registers", "registers initialized", 0, 4));
    assert(tsfi_cordis_session_record_turn(&tree_main, "emit scsi test c source", "void test_scsi() {}", 4, 8));

    uint32_t verified = 0;
    assert(tsfi_cordis_session_replay_verify(&tree_main, &verified) && verified == 2);
    printf("[1/4] Session history replay verified: %u turns recorded\n", verified);

    // Fork session at turn 1 into branch session 1002
    tsfi_cordis_session_tree_t tree_fork;
    assert(tsfi_cordis_session_fork_branch(&tree_main, 1, 1002, &tree_fork));
    assert(tree_fork.is_forked && tree_fork.total_recorded_turns == 1);
    assert(strcmp(tree_fork.turns[0].prompt, "init zmm registers") == 0);
    printf("[1/4] Branch forking verified (Session 1001 -> Forked Session 1002 @ Turn 1)\n");

    // 2. Test Sandboxing Path Validation
    tsfi_cordis_sandbox_config_t sandbox;
    assert(tsfi_cordis_sandbox_init(&sandbox, "/home/mariarahel/src/tsfi2", "/home/mariarahel/src/tsfi2/wal"));

    assert(tsfi_cordis_sandbox_validate_path(&sandbox, "/home/mariarahel/src/tsfi2/tsfi2-deepseek/src/file.c", true));
    assert(tsfi_cordis_sandbox_validate_path(&sandbox, "/home/mariarahel/src/tsfi2/wal/test.dat.bin", true));
    assert(tsfi_cordis_sandbox_validate_path(&sandbox, "/tmp/scratch.bin", true));
    assert(!tsfi_cordis_sandbox_validate_path(&sandbox, "/etc/shadow", true));
    printf("[2/4] Sandbox path isolation verified (protected /etc/shadow rejected)\n");

    // 3. Test Presets Loading
    tsfi_cordis_preset_profile_t preset_code, preset_hw;
    assert(tsfi_cordis_preset_load(CORDIS_PRESET_CODE, &preset_code));
    assert(preset_code.enable_ast_secondary_pass && preset_code.max_token_budget == 4096);

    assert(tsfi_cordis_preset_load(CORDIS_PRESET_HARDWARE_TRACE, &preset_hw));
    assert(preset_hw.enable_scsi_hardware_tracing && strcmp(preset_hw.default_model_alias, "DeepSeek-Coder-Hardware-ZMM") == 0);
    printf("[3/4] Operational presets loaded (Code & Hardware-Trace verified)\n");

    // 4. Test In-Process Adapter Hot-Swap
    tsfi_cordis_adapter_state_t adapter;
    assert(tsfi_cordis_adapter_init(&adapter, "/models/DeepSeek-Coder-6.7B-Q4.gguf", "Q4_K_M"));
    assert(tsfi_cordis_adapter_hot_swap(&adapter, "/models/DeepSeek-Coder-6.7B-Q8.gguf", "Q8_0", true));
    assert(adapter.total_adapter_hot_swaps == 1 && adapter.session_context_preserved);
    printf("[4/4] In-process model adapter hot-swap verified (latency: %.2f ms)\n", adapter.swap_latency_ms);

    printf("=== ALL ADVANCED DEEPSEEK CORDIS TESTS PASSED ===\n");
    return 0;
}
