#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_doxastic_pipeline.h"

TSFiDoxasticPipeline* tsfi_doxastic_pipeline_create(tsfi_dat *dat) {
    TSFiDoxasticPipeline *pipeline = calloc(1, sizeof(TSFiDoxasticPipeline));
    if (!pipeline) return NULL;
    pipeline->dat = dat;
    pipeline->active_vertex_count = 0;
    pipeline->is_dirty = 0;
    return pipeline;
}

int tsfi_doxastic_pipeline_update(TSFiDoxasticPipeline *pipeline, int x_start, int x_end, int y, int z,
                                  const char *hyp_path, const char *ev_path) {
    if (!pipeline || !pipeline->dat) return -1;

    // 1. Run direct Vulkan projection of doxastic coordinate states
    int count = tsfi_doxastic_project_region(pipeline->dat, x_start, x_end, y, z, 
                                             pipeline->vertex_buffer, 1024);
    if (count < 0) return -2;

    pipeline->active_vertex_count = (uint32_t)count;
    pipeline->is_dirty = 1;

    // 2. Perform conditional check
    if (hyp_path && ev_path) {
        (void)tsfi_unified_classify_region(pipeline->dat, x_start, x_end, y, z, hyp_path, ev_path);
    }

    return 0;
}

void tsfi_doxastic_pipeline_destroy(TSFiDoxasticPipeline *pipeline) {
    if (!pipeline) return;
    free(pipeline);
}
