#ifndef TSFI_SHADOWMAP_H
#define TSFI_SHADOWMAP_H

#include <stdint.h>
#include <stdbool.h>

#define TSFI_SHADOW_SIZE 16

typedef struct {
    double depth_grid[TSFI_SHADOW_SIZE][TSFI_SHADOW_SIZE];
} TSFiShadowMap;

// Initialize the ShadowMap context with max depth values
void tsfi_shadowmap_init(TSFiShadowMap *sm);

// Sets the depth value at specific grid cell coordinates
bool tsfi_shadowmap_set_depth(TSFiShadowMap *sm, int x, int y, double depth);

// Performs O(1) constant-time shadow lookup comparing point depth to shadow map depth
bool tsfi_shadowmap_is_in_shadow(const TSFiShadowMap *sm, int x, int y, double point_depth);

// Packs the depth grid array into a compressed 8-bit XPLSM shadow map byte layout
void tsfi_shadowmap_pack_xplsm(const TSFiShadowMap *sm, uint8_t *dest_xpl_buffer);

#endif // TSFI_SHADOWMAP_H
