#ifndef TSFI_DESIGN_PIPELINE_H
#define TSFI_DESIGN_PIPELINE_H

#include <stddef.h>
#include <stdbool.h>
#include "tsfi_vision_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

float tsfi_eval_teddy_fitness(const TSFiResonanceAnalysis *vision, float mass_density);
bool tsfi_pipeline_archive_candidate(const char *id, void *pixels, size_t pix_size, void *latents, size_t lat_size, float score);

// Autonomy: Fault-to-Evolution Bridge
#include "window_inc/vulkan_struct.h"
void tsfi_resolve_fault_autonomous(void *manifold_shm, VulkanContext *vk);

#ifdef __cplusplus
}
#endif

#endif // TSFI_DESIGN_PIPELINE_H
