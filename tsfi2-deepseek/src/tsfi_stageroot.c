#include "tsfi_stageroot.h"
#include <string.h>

void tsfi_stageroot_init(TSFiStageRoot *stage, const char *path) {
    if (!stage) return;
    
    if (path) {
        strncpy(stage->stage_path, path, sizeof(stage->stage_path) - 1);
    } else {
        strcpy(stage->stage_path, "/auncient/stage/root");
    }
    
    strcpy(stage->up_axis, "Z");
    stage->global_scale = 1.0f;
    stage->start_time_code = 0.0f;
    stage->end_time_code = 100.0f;
    stage->prim_count = 0;
    
    memset(stage->prims, 0, sizeof(stage->prims));
}

bool tsfi_stageroot_define_prim(TSFiStageRoot *stage, const char *path, TSFiPrimType type) {
    if (!stage || !path || stage->prim_count >= TSFI_MAX_STAGE_PRIMS) return false;
    
    // Check for duplicate path definitions
    for (int i = 0; i < stage->prim_count; i++) {
        if (strcmp(stage->prims[i].prim_path, path) == 0) {
            return false;
        }
    }
    
    TSFiStagePrim *prim = &stage->prims[stage->prim_count];
    strncpy(prim->prim_path, path, sizeof(prim->prim_path) - 1);
    prim->type = type;
    prim->is_active = true;
    
    stage->prim_count++;
    return true;
}

void tsfi_stageroot_set_metadata(TSFiStageRoot *stage, const char *up_axis, float global_scale) {
    if (!stage) return;
    
    if (up_axis) {
        strncpy(stage->up_axis, up_axis, sizeof(stage->up_axis) - 1);
    }
    
    if (global_scale > 0.0f) {
        stage->global_scale = global_scale;
    }
}
