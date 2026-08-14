#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_deepseek_cordis_tools.h"

int main(void) {
    printf("=== TESTING DEEPSEEK CORDIS DIFF, HEALING & GIT TOOLS ===\n");

    // 1. Test Surgical Line-Level Diff & Patch Engine
    tsfi_cordis_diff_patch_state_t diff;
    assert(tsfi_cordis_diff_init(&diff));

    assert(tsfi_cordis_diff_add_hunk(
        &diff,
        10,
        1,
        "uint32_t old_reg = 0;",
        "uint32_t old_reg = ZMM_INITIAL_VECTOR;"
    ));

    const char *orig_src = "int main() {\n    uint32_t old_reg = 0;\n    return 0;\n}\n";
    char patched_src[512] = {0};
    assert(tsfi_cordis_apply_patch_buffer(&diff, orig_src, patched_src, sizeof(patched_src)));
    assert(strstr(patched_src, "ZMM_INITIAL_VECTOR") != NULL);
    printf("[1/3] Surgical diff patch applied successfully\n");

    // 2. Test Autonomous Self-Healing Loop
    tsfi_cordis_self_healing_state_t heal;
    assert(tsfi_cordis_self_healing_init(&heal, 3));
    assert(tsfi_cordis_self_healing_record_failure(
        &heal,
        1,
        "error: undefined reference to 'ZMM_INITIAL_VECTOR'",
        "uint32_t old_reg = ZMM_INITIAL_VECTOR;"
    ));
    assert(heal.current_attempt == 1);
    assert(strstr(heal.synthesized_repair_prompt, "Self-Healing Pass 1/3") != NULL);
    printf("[2/3] Self-healing failure captured and repair prompt constructed\n");

    // 3. Test Git Rule 2 Safety Guard
    tsfi_cordis_git_status_t git;
    assert(tsfi_cordis_git_status_init(&git));

    // Valid source file
    assert(tsfi_cordis_git_verify_file_safety(&git, "tsfi2-deepseek/src/tsfi_deepseek_cordis_tools.c"));

    // Reject compiled objects (.o)
    assert(!tsfi_cordis_git_verify_file_safety(&git, "tsfi2-deepseek/obj/tsfi_core.o"));
    assert(git.rule2_binary_detected);

    // Reject shared libraries (.so)
    assert(!tsfi_cordis_git_verify_file_safety(&git, "tsfi2-deepseek/lib/libcore.so"));

    // Reject prohibited executables
    assert(!tsfi_cordis_git_verify_file_safety(&git, "tests/rooted_frame_presenter"));
    printf("[3/3] Git Rule 2 binary commit guard verified successfully\n");

    printf("=== ALL DEEPSEEK CORDIS TOOL SUITE TESTS PASSED ===\n");
    return 0;
}
