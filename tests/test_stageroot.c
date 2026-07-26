#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_stageroot.h"

int main(void) {
    printf("=== RUNNING AUNCIENT STAGEROOT METADATA TESTS ===\n");

    TSFiStageRoot stage;
    tsfi_stageroot_init(&stage, "/auncient/stage/main_layout");

    assert(strcmp(stage.stage_path, "/auncient/stage/main_layout") == 0);
    assert(strcmp(stage.up_axis, "Z") == 0);
    assert(fabs(stage.global_scale - 1.0f) < 1e-5f);

    // Update metadata to Y up axis and 0.01 scale
    tsfi_stageroot_set_metadata(&stage, "Y", 0.01f);
    assert(strcmp(stage.up_axis, "Y") == 0);
    assert(fabs(stage.global_scale - 0.01f) < 1e-5f);

    // Define a new camera and mesh primitive on the stage
    assert(tsfi_stageroot_define_prim(&stage, "/World/Camera", TSFI_PRIM_CAMERA));
    assert(tsfi_stageroot_define_prim(&stage, "/World/Torus", TSFI_PRIM_MESH));
    assert(stage.prim_count == 2);

    // Check duplicate path blocking
    assert(!tsfi_stageroot_define_prim(&stage, "/World/Camera", TSFI_PRIM_CAMERA));
    assert(stage.prim_count == 2);

    printf("   ✓ StageRoot global metadata updates verified.\n");
    printf("   ✓ Stage Prim registry definitions and duplicates blocking verified.\n");
    printf("=== AUNCIENT STAGEROOT METADATA TESTS COMPLETE (PASS) ===\n");
    return 0;
}
