#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tsfi_deepseek_cordis_tools.h"

// ----------------------------------------------------------------------------
// 1. Surgical Line-Level Diff & Patch Engine Implementation
// ----------------------------------------------------------------------------

bool tsfi_cordis_diff_init(tsfi_cordis_diff_patch_state_t *patch) {
    if (!patch) return false;
    memset(patch, 0, sizeof(*patch));
    patch->patch_cleanly_applied = true;
    return true;
}

bool tsfi_cordis_diff_add_hunk(
    tsfi_cordis_diff_patch_state_t *patch,
    uint32_t start_line,
    uint32_t delete_count,
    const char *match_text,
    const char *replace_text
) {
    if (!patch || !match_text || !replace_text || patch->hunk_count >= TSFI_CORDIS_MAX_HUNKS) {
        return false;
    }
    uint32_t idx = patch->hunk_count++;
    tsfi_cordis_diff_hunk_t *hunk = &patch->hunks[idx];
    hunk->start_line = start_line;
    hunk->delete_count = delete_count;
    hunk->insert_count = 1;
    strncpy(hunk->target_match, match_text, sizeof(hunk->target_match) - 1);
    strncpy(hunk->replacement_text, replace_text, sizeof(hunk->replacement_text) - 1);
    return true;
}

bool tsfi_cordis_apply_patch_buffer(
    const tsfi_cordis_diff_patch_state_t *patch,
    const char *src_buffer,
    char *dst_buffer,
    size_t max_dst_len
) {
    if (!patch || !src_buffer || !dst_buffer || max_dst_len == 0) return false;
    
    // Copy source buffer, replacing target matched strings directly
    size_t src_len = strlen(src_buffer);
    if (src_len >= max_dst_len) return false;

    char temp_work[8192];
    strncpy(temp_work, src_buffer, sizeof(temp_work) - 1);
    temp_work[sizeof(temp_work) - 1] = '\0';

    for (uint32_t h = 0; h < patch->hunk_count; h++) {
        const tsfi_cordis_diff_hunk_t *hk = &patch->hunks[h];
        char *loc = strstr(temp_work, hk->target_match);
        if (loc) {
            char after[4096] = {0};
            strncpy(after, loc + strlen(hk->target_match), sizeof(after) - 1);
            *loc = '\0';
            snprintf(dst_buffer, max_dst_len, "%s%s%s", temp_work, hk->replacement_text, after);
            strncpy(temp_work, dst_buffer, sizeof(temp_work) - 1);
        } else {
            return false;
        }
    }
    strncpy(dst_buffer, temp_work, max_dst_len - 1);
    return true;
}

// ----------------------------------------------------------------------------
// 2. Autonomous Test-Driven Self-Healing Loop Implementation
// ----------------------------------------------------------------------------

bool tsfi_cordis_self_healing_init(
    tsfi_cordis_self_healing_state_t *heal,
    uint32_t max_attempts
) {
    if (!heal) return false;
    memset(heal, 0, sizeof(*heal));
    heal->max_healing_attempts = (max_attempts > 0) ? max_attempts : 3;
    return true;
}

bool tsfi_cordis_self_healing_record_failure(
    tsfi_cordis_self_healing_state_t *heal,
    int exit_code,
    const char *compiler_error_output,
    const char *failing_source_snippet
) {
    if (!heal || !compiler_error_output || !failing_source_snippet) return false;
    heal->current_attempt++;
    heal->last_exit_code = exit_code;
    strncpy(heal->captured_error_log, compiler_error_output, sizeof(heal->captured_error_log) - 1);

    snprintf(heal->synthesized_repair_prompt, sizeof(heal->synthesized_repair_prompt),
             "Self-Healing Pass %u/%u:\n"
             "Compiler exited with code %d.\n"
             "Diagnostics: %.512s\n"
             "Failing Block:\n%.1024s\n"
             "Synthesize minimal AST patch.",
             heal->current_attempt, heal->max_healing_attempts, exit_code,
             compiler_error_output, failing_source_snippet);
    return true;
}

// ----------------------------------------------------------------------------
// 3. Git Tree State Tracker & Rule 2 Binary Staging Guard Implementation
// ----------------------------------------------------------------------------

bool tsfi_cordis_git_status_init(tsfi_cordis_git_status_t *git) {
    if (!git) return false;
    memset(git, 0, sizeof(*git));
    strncpy(git->current_branch, "master", sizeof(git->current_branch) - 1);
    return true;
}

bool tsfi_cordis_git_verify_file_safety(
    tsfi_cordis_git_status_t *git,
    const char *file_path
) {
    if (!git || !file_path) return false;
    size_t len = strlen(file_path);

    // Rule 2 Enforcement: Reject compiled binaries, .o, .so, executables
    if ((len >= 2 && strcmp(file_path + len - 2, ".o") == 0) ||
        (len >= 3 && strcmp(file_path + len - 3, ".so") == 0) ||
        (len >= 4 && strcmp(file_path + len - 4, ".bin") != 0 && strstr(file_path, "bin/")) ||
        strstr(file_path, "tests/rooted_frame_presenter")) {
        git->rule2_binary_detected = true;
        strncpy(git->violating_file_path, file_path, sizeof(git->violating_file_path) - 1);
        return false; // Violates Rule 2
    }
    return true;
}
