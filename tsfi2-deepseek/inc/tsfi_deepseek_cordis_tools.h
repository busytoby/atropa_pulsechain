#ifndef TSFI_DEEPSEEK_CORDIS_TOOLS_H
#define TSFI_DEEPSEEK_CORDIS_TOOLS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define TSFI_CORDIS_MAX_HUNKS       32
#define TSFI_CORDIS_DIFF_LINE_LEN   512

// ============================================================================
// 1. Surgical Line-Level Diff & Patch Engine
// ============================================================================

typedef struct {
    uint32_t start_line;
    uint32_t delete_count;
    uint32_t insert_count;
    char target_match[TSFI_CORDIS_DIFF_LINE_LEN];
    char replacement_text[TSFI_CORDIS_DIFF_LINE_LEN];
} tsfi_cordis_diff_hunk_t;

typedef struct {
    uint32_t hunk_count;
    tsfi_cordis_diff_hunk_t hunks[TSFI_CORDIS_MAX_HUNKS];
    uint32_t lines_modified;
    bool patch_cleanly_applied;
} tsfi_cordis_diff_patch_state_t;

bool tsfi_cordis_diff_init(tsfi_cordis_diff_patch_state_t *patch);
bool tsfi_cordis_diff_add_hunk(
    tsfi_cordis_diff_patch_state_t *patch,
    uint32_t start_line,
    uint32_t delete_count,
    const char *match_text,
    const char *replace_text
);
bool tsfi_cordis_apply_patch_buffer(
    const tsfi_cordis_diff_patch_state_t *patch,
    const char *src_buffer,
    char *dst_buffer,
    size_t max_dst_len
);

// ============================================================================
// 2. Autonomous Test-Driven Self-Healing Loop
// ============================================================================

typedef struct {
    uint32_t max_healing_attempts;
    uint32_t current_attempt;
    int last_exit_code;
    char captured_error_log[2048];
    char synthesized_repair_prompt[4096];
    bool healed_successfully;
    float loop_elapsed_ms;
} tsfi_cordis_self_healing_state_t;

bool tsfi_cordis_self_healing_init(
    tsfi_cordis_self_healing_state_t *heal,
    uint32_t max_attempts
);
bool tsfi_cordis_self_healing_record_failure(
    tsfi_cordis_self_healing_state_t *heal,
    int exit_code,
    const char *compiler_error_output,
    const char *failing_source_snippet
);

// ============================================================================
// 3. Git Tree State Tracker & Rule 2 Binary Staging Guard
// ============================================================================

typedef struct {
    char current_branch[64];
    uint32_t staged_files_count;
    uint32_t untracked_files_count;
    bool rule2_binary_detected;
    char violating_file_path[256];
} tsfi_cordis_git_status_t;

bool tsfi_cordis_git_status_init(tsfi_cordis_git_status_t *git);
bool tsfi_cordis_git_verify_file_safety(
    tsfi_cordis_git_status_t *git,
    const char *file_path
);

#endif // TSFI_DEEPSEEK_CORDIS_TOOLS_H
