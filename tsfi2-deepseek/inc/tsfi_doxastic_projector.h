#ifndef TSFI_DOXASTIC_PROJECTOR_H
#define TSFI_DOXASTIC_PROJECTOR_H

#include "tsfi_doxastic_classifier.h"

// Vulkan vertex representation of a doxastic classification coordinate
typedef struct {
    float x, y, z;      // Position coordinates (3D voxel space)
    float r, g, b, a;   // Visual color representation based on belief class
    float strain;       // Strain boundary gradient
} TSFiDoxasticVertex;

// Project SVDAG classifications in a region to a Vulkan vertex buffer
// Returns the number of vertices written, up to max_vertices
int tsfi_doxastic_project_region(tsfi_dat *dat, int x_start, int x_end, int y, int z, 
                                 TSFiDoxasticVertex *vertices_out, int max_vertices);

#endif // TSFI_DOXASTIC_PROJECTOR_H
