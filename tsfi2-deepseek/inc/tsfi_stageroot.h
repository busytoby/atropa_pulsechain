#ifndef TSFI_STAGEROOT_H
#define TSFI_STAGEROOT_H

#include <stdint.h>
#include <stdbool.h>

#define TSFI_MAX_STAGE_PRIMS 16

typedef enum {
    TSFI_PRIM_MESH,
    TSFI_PRIM_CAMERA,
    TSFI_PRIM_CURVES,
    TSFI_PRIM_POINTS,
    TSFI_PRIM_LIGHT,
    TSFI_PRIM_XFORM
} TSFiPrimType;

typedef struct {
    char prim_path[64];
    TSFiPrimType type;
    bool is_active;
} TSFiStagePrim;

typedef struct {
    char stage_path[64];
    char up_axis[8]; // "Y" or "Z"
    float global_scale;
    float start_time_code;
    float end_time_code;
    
    TSFiStagePrim prims[TSFI_MAX_STAGE_PRIMS];
    int prim_count;
} TSFiStageRoot;

// Initialize the StageRoot context
void tsfi_stageroot_init(TSFiStageRoot *stage, const char *path);

// Defines a new prim reference in the stage container
bool tsfi_stageroot_define_prim(TSFiStageRoot *stage, const char *path, TSFiPrimType type);

// Sets the global up-axis and metadata values
void tsfi_stageroot_set_metadata(TSFiStageRoot *stage, const char *up_axis, float global_scale);

#endif // TSFI_STAGEROOT_H
