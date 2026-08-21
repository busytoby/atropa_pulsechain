#include "tsfi_zmachine_vaesen_bridge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_CACHED_SLICES 16
static struct {
    char path[64];
    TsfiVaesenDeviceSliceBin slice;
} g_slice_cache[MAX_CACHED_SLICES];
static int g_slice_cache_count = 0;

static int get_or_load_slice(const char *path, TsfiVaesenDeviceSliceBin *out_slice) {
    for (int i = 0; i < g_slice_cache_count; ++i) {
        if (strcmp(g_slice_cache[i].path, path) == 0) {
            *out_slice = g_slice_cache[i].slice;
            return 0;
        }
    }
    if (tsfi_vaesen_slice_load(path, out_slice) != 0) return -1;
    if (g_slice_cache_count < MAX_CACHED_SLICES) {
        strncpy(g_slice_cache[g_slice_cache_count].path, path, 64);
        g_slice_cache[g_slice_cache_count].slice = *out_slice;
        g_slice_cache_count++;
    }
    return 0;
}

int tsfi_zmachine_execute_wmq_interact(
    const TsfiZMachinePlayerState *player,
    const char *target_vaesen_slice_path,
    TsfiZMachineInteractionResult *out_result
) {
    if (!player || !target_vaesen_slice_path || !out_result) return -1;
    memset(out_result, 0, sizeof(TsfiZMachineInteractionResult));

    /* 1. Load the Target Vaesen .dat.bin Slice (Cached for sub-microsecond latency) */
    TsfiVaesenDeviceSliceBin entity_slice;
    if (get_or_load_slice(target_vaesen_slice_path, &entity_slice) != 0) {
        snprintf(out_result->dialogue_output, sizeof(out_result->dialogue_output),
                 "[Z-MACHINE ERROR] Failed to memory-map target WinchesterMQ slice: %s", target_vaesen_slice_path);
        return -1;
    }

    /* Rule 14 DisplacementShader register synchronization */
    uint32_t chin_clamp = 500 + (uint32_t)player->phys_trauma * 100;
    uint32_t monopole_scale = 1000 + (uint32_t)player->ment_trauma * 250;
    (void)chin_clamp;
    (void)monopole_scale;

    /* 2. Check for Inventory Banishment Ritual Match */
    if (strlen(player->held_item) > 0 &&
        strstr(entity_slice.banishment_ritual_key, player->held_item) != NULL) {
        out_result->ritual_banishment_success = true;
        snprintf(out_result->dialogue_output, sizeof(out_result->dialogue_output),
                 "[Z-MACHINE] You present '%s'. %s recoils as the banishment rite seals the encounter!",
                 player->held_item, entity_slice.name);

        /* Generate calm, open sanctuary vector lines */
        out_result->num_lines = 4;
        out_result->lines[0] = (TsfiZMachineVectorLine){ .start_x = 20, .start_y = 120, .end_x = 100, .end_y = 40, .color_idx = 7 };  /* Sunlit roof */
        out_result->lines[1] = (TsfiZMachineVectorLine){ .start_x = 100, .start_y = 40, .end_x = 180, .end_y = 120, .color_idx = 7 };
        out_result->lines[2] = (TsfiZMachineVectorLine){ .start_x = 40, .start_y = 120, .end_x = 160, .end_y = 120, .color_idx = 14 }; /* Ground threshold */
        out_result->lines[3] = (TsfiZMachineVectorLine){ .start_x = 100, .start_y = 120, .end_x = 100, .end_y = 60, .color_idx = 10 }; /* Peaceful spire */
        return 0;
    }

    /* 3. Construct Temporary Player Slice Reflecting Active Physical & Mental Trauma */
    TsfiVaesenDeviceSliceBin player_slice;
    memset(&player_slice, 0, sizeof(player_slice));
    strncpy(player_slice.magic, "VAESEN01", 12);
    strncpy(player_slice.wmq_address, player->player_wmq, 48);
    strncpy(player_slice.name, "Investigator", 64);
    player_slice.clan_id = 1; /* Society Clan */
    player_slice.caste = 4;   /* Scholar / Officer */
    player_slice.physique = (player->phys_trauma >= 3) ? 1 : (5 - player->phys_trauma);
    player_slice.precision = 3;
    player_slice.logic = (player->ment_trauma >= 3) ? 1 : (4 - player->ment_trauma);
    player_slice.fervour = 3 + player->ment_trauma;
    player_slice.dogma = 3;
    player_slice.fear_rating = player->ment_trauma + 1;
    player_slice.edo22_carrier_freq = 8;
    player_slice.mathieu_q0_milli = 800 + (uint16_t)player->ment_trauma * 1000;
    player_slice.rest_length_L0 = 10.0f;
    player_slice.spring_stiffness_k = 1.5f;

    /* 4. Execute Dual-Slice [WMQ:WMQ] Interaction */
    float dist = 0.0f;
    uint16_t phase = 0;
    bool is_locked = false;
    char dialogue_buf[256];

    tsfi_vaesen_slice_interact(&player_slice, &entity_slice, &dist, &phase, &is_locked, dialogue_buf, sizeof(dialogue_buf));
    strncpy(out_result->dialogue_output, dialogue_buf, sizeof(out_result->dialogue_output));

    /* Attach Mistral Entropy, Guardrails, and Certainty Telemetry */
    out_result->entropy = tsfi_vaesen_compute_acoustic_entropy(&entity_slice);
    out_result->guardrails = tsfi_vaesen_get_lore_guardrail_mask(&entity_slice);
    out_result->certainty = tsfi_vaesen_compute_acoustic_certainty(phase);

    /* 5. Synthesize 5-Byte Vector Line Drawing Segments (getVectorScene) */
    if (is_locked) {
        /* Balanced, harmonic perspective room */
        out_result->num_lines = 5;
        out_result->lines[0] = (TsfiZMachineVectorLine){ .start_x = 30, .start_y = 180, .end_x = 70, .end_y = 100, .color_idx = 6 };
        out_result->lines[1] = (TsfiZMachineVectorLine){ .start_x = 70, .start_y = 100, .end_x = 130, .end_y = 100, .color_idx = 6 };
        out_result->lines[2] = (TsfiZMachineVectorLine){ .start_x = 130, .start_y = 100, .end_x = 170, .end_y = 180, .color_idx = 6 };
        out_result->lines[3] = (TsfiZMachineVectorLine){ .start_x = 70, .start_y = 100, .end_x = 70, .end_y = 180, .color_idx = 11 };
        out_result->lines[4] = (TsfiZMachineVectorLine){ .start_x = 130, .start_y = 100, .end_x = 130, .end_y = 180, .color_idx = 11 };
    } else {
        /* Jagged, chaotic fear spikes */
        out_result->num_lines = 6;
        out_result->lines[0] = (TsfiZMachineVectorLine){ .start_x = 10, .start_y = 190, .end_x = 50, .end_y = 30, .color_idx = 2 };
        out_result->lines[1] = (TsfiZMachineVectorLine){ .start_x = 50, .start_y = 30, .end_x = 90, .end_y = 160, .color_idx = 2 };
        out_result->lines[2] = (TsfiZMachineVectorLine){ .start_x = 90, .start_y = 160, .end_x = 130, .end_y = 20, .color_idx = 4 };
        out_result->lines[3] = (TsfiZMachineVectorLine){ .start_x = 130, .start_y = 20, .end_x = 170, .end_y = 170, .color_idx = 4 };
        out_result->lines[4] = (TsfiZMachineVectorLine){ .start_x = 170, .start_y = 170, .end_x = 190, .end_y = 190, .color_idx = 1 };
        out_result->lines[5] = (TsfiZMachineVectorLine){ .start_x = 10, .start_y = 190, .end_x = 190, .end_y = 190, .color_idx = 1 };
    }

    return 0;
}
