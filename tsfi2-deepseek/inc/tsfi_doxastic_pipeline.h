#ifndef TSFI_DOXASTIC_PIPELINE_H
#define TSFI_DOXASTIC_PIPELINE_H

#include "tsfi_unified_classifier.h"
#include "tsfi_doxastic_projector.h"

// Unified Doxastic Render Pipeline Context
typedef struct {
    tsfi_dat *dat;
    TSFiDoxasticVertex vertex_buffer[1024];
    uint32_t active_vertex_count;
    int is_dirty;
} TSFiDoxasticPipeline;

// Create Render Pipeline Context
TSFiDoxasticPipeline* tsfi_doxastic_pipeline_create(tsfi_dat *dat);

// Run pipeline update step: scans SVDAG coordinates and writes attributes to Vulkan buffer
int tsfi_doxastic_pipeline_update(TSFiDoxasticPipeline *pipeline, int x_start, int x_end, int y, int z,
                                  const char *hyp_path, const char *ev_path);

// Destroy Render Pipeline Context
void tsfi_doxastic_pipeline_destroy(TSFiDoxasticPipeline *pipeline);

#endif // TSFI_DOXASTIC_PIPELINE_H
