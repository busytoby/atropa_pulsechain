#include "tsfi_vaesen_spatial_vision.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int tsfi_vaesen_evaluate_spatial_vision(
    const TsfiVaesenDeviceSliceBin *slice,
    const TsfiZMachineVectorLine *lines,
    uint8_t num_lines,
    TsfiVaesenSpatialVisionField *out_field,
    TsfiVaesenPartitionedMemory *out_mem
) {
    if (!slice || !out_field || !out_mem) return -1;
    memset(out_field, 0, sizeof(TsfiVaesenSpatialVisionField));
    memset(out_mem, 0, sizeof(TsfiVaesenPartitionedMemory));

    /* Rule 14 DisplacementShader register synchronization */
    uint32_t chin_clamp = 1000;
    uint32_t monopole_scale = 2000;
    (void)chin_clamp;
    (void)monopole_scale;

    /* 1. Initialize Static Folklore ROM based on Creature Clan & Identity */
    if (slice->clan_id == 0) {
        /* Outcasts (Trolls, Näcken): Repelled by Consecrated Soil / Direct Sun */
        out_mem->forbidden_env_mask = TSFI_ENV_CONSECRATED_SOIL | TSFI_ENV_HAS_DIRECT_SUN;
        out_mem->sanctuary_env_mask = TSFI_ENV_HAS_RUNNING_WATER;
        strncpy(out_mem->core_folklore_law, "Waters give sanctuary; sunlight and church bells repel.", 128);
    } else if (slice->clan_id == 2) {
        /* Wardens (Tomte, Church Grim): Repelled by Threshold Blockade */
        out_mem->forbidden_env_mask = TSFI_ENV_THRESHOLD_BLOCKED;
        out_mem->sanctuary_env_mask = TSFI_ENV_CONSECRATED_SOIL;
        strncpy(out_mem->core_folklore_law, "Protects the consecrated threshold; repelled by barred gates.", 128);
    } else {
        /* Default baseline */
        out_mem->forbidden_env_mask = 0;
        out_mem->sanctuary_env_mask = 0;
        strncpy(out_mem->core_folklore_law, "Neutral baseline folklore orientation.", 128);
    }

    /* 2. Analyze 2D Vector Drawing Segments */
    uint8_t count = num_lines > TSFI_VISION_MAX_LINES ? TSFI_VISION_MAX_LINES : num_lines;
    out_field->num_lines = count;

    uint32_t flags = 0;
    float avg_x = 0.0f, avg_y = 0.0f;

    for (uint8_t i = 0; i < count; ++i) {
        out_field->lines[i] = lines[i];
        avg_x += (float)(lines[i].start_x + lines[i].end_x) * 0.5f;
        avg_y += (float)(lines[i].start_y + lines[i].end_y) * 0.5f;

        /* Color Index Mapping to Environmental Semantics */
        if (lines[i].color_idx == 6 || lines[i].color_idx == 11) {
            /* Blue / Cyan -> Running Water */
            flags |= TSFI_ENV_HAS_RUNNING_WATER;
        } else if (lines[i].color_idx == 7 || lines[i].color_idx == 10) {
            /* Bright Yellow / Sun -> Direct Sun */
            flags |= TSFI_ENV_HAS_DIRECT_SUN;
        } else if (lines[i].color_idx == 14) {
            /* Consecrated church threshold */
            flags |= TSFI_ENV_CONSECRATED_SOIL;
        } else if (lines[i].color_idx == 1 || lines[i].color_idx == 2) {
            /* Heavy red / black iron bar -> Blocked Threshold */
            flags |= TSFI_ENV_THRESHOLD_BLOCKED;
        }
    }

    if (count > 0) {
        avg_x /= (float)count;
        avg_y /= (float)count;
    }
    out_mem->active_spatial_x = avg_x;
    out_mem->active_spatial_y = avg_y;
    out_field->detected_env_flags = flags;

    /* 3. Evaluate Spatial Comfort & Behavioral State */
    if ((flags & out_mem->forbidden_env_mask) != 0) {
        out_field->spatial_comfort_index = 0.10f;
        out_mem->reaction_state = 2; /* Repelled / Fleeing */
    } else if ((flags & out_mem->sanctuary_env_mask) != 0) {
        out_field->spatial_comfort_index = 0.95f;
        out_mem->reaction_state = 0; /* Calm / Sanctuary */
    } else {
        out_field->spatial_comfort_index = 0.50f;
        out_mem->reaction_state = 1; /* Guarding / Alert */
    }

    return 0;
}
