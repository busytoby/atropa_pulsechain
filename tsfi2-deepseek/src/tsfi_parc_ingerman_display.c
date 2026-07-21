#include "tsfi_parc_ingerman_display.h"
#include <stdio.h>
#include <string.h>

int tsfi_ingerman_display_env_init(tsfi_ingerman_display_env_t *env) {
    if (!env) return -1;
    memset(env, 0, sizeof(tsfi_ingerman_display_env_t));
    env->is_initialized = true;
    env->current_depth = 0;
    snprintf(env->dynamic_contract_router, sizeof(env->dynamic_contract_router),
             "dynamic_0x446973706c6179566563746f7231393631564d");
    return 0;
}

int tsfi_ingerman_display_push_block(tsfi_ingerman_display_env_t *env, uint32_t block_id, tsfi_ingerman_stack_frame_t *frame) {
    if (!env || !frame) return -1;
    if (env->current_depth >= TSFI_INGERMAN_MAX_LEXICAL_DEPTH) return -2;

    uint32_t level = env->current_depth;
    frame->block_id = block_id;
    frame->lexical_level = level;
    snprintf(frame->contract_address, sizeof(frame->contract_address),
             "dynamic_0x4672616d654c6576656c%02x", level);

    // Update Display Vector for O(1) lexical access
    env->display_vector[level] = frame;
    env->current_depth++;

    return 0;
}

int tsfi_ingerman_display_pop_block(tsfi_ingerman_display_env_t *env) {
    if (!env || env->current_depth == 0) return -1;

    env->current_depth--;
    env->display_vector[env->current_depth] = NULL;
    return 0;
}

uint64_t tsfi_ingerman_display_lookup(const tsfi_ingerman_display_env_t *env, uint32_t lexical_level, uint32_t slot_index) {
    if (!env || lexical_level >= env->current_depth) return 0;

    const tsfi_ingerman_stack_frame_t *frame = env->display_vector[lexical_level];
    if (!frame || slot_index >= frame->slot_count) return 0;

    return frame->frame_slots[slot_index];
}

int tsfi_ingerman_display_store(tsfi_ingerman_display_env_t *env, uint32_t lexical_level, uint32_t slot_index, uint64_t value) {
    if (!env || lexical_level >= env->current_depth) return -1;

    tsfi_ingerman_stack_frame_t *frame = env->display_vector[lexical_level];
    if (!frame || slot_index >= TSFI_INGERMAN_MAX_FRAME_SLOTS) return -2;

    frame->frame_slots[slot_index] = value;
    if (slot_index >= frame->slot_count) {
        frame->slot_count = slot_index + 1;
    }

    return 0;
}
