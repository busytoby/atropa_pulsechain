#ifndef AUNCIENT_USD_STL_BRIDGE_H
#define AUNCIENT_USD_STL_BRIDGE_H

#include "auncient_stl_loader.h"
#include <stdbool.h>

// Converts parsed STL facets into a complete ASCII USDA mesh definition file
bool auncient_bridge_stl_to_usda(const AuncientStlMesh *stl_mesh, const char *usda_filepath);

// Converts a unified USDA mesh topology back into an ASCII STL file
bool auncient_bridge_usda_to_stl(const float *points, uint32_t point_count, const uint32_t *indices, uint32_t index_count, const char *stl_filepath);

// Assembles multiple named STL segments into a single USDA stage structure
bool auncient_bridge_multi_stl_to_usda(const AuncientStlMesh *meshes, const char **segment_names, uint32_t count, const char *usda_filepath);

#endif // AUNCIENT_USD_STL_BRIDGE_H
