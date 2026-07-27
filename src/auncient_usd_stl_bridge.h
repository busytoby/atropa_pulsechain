#ifndef AUNCIENT_USD_STL_BRIDGE_H
#define AUNCIENT_USD_STL_BRIDGE_H

#include "auncient_stl_loader.h"
#include <stdbool.h>

// Converts parsed STL facets into a complete ASCII USDA mesh definition file
bool auncient_bridge_stl_to_usda(const AuncientStlMesh *stl_mesh, const char *usda_filepath);

#endif // AUNCIENT_USD_STL_BRIDGE_H
