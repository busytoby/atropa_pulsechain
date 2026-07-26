#include "tsfi_shadowmap.h"
#include <stdio.h>

void tsfi_shadowmap_init(TSFiShadowMap *sm) {
    if (!sm) return;
    for (int y = 0; y < TSFI_SHADOW_SIZE; y++) {
        for (int x = 0; x < TSFI_SHADOW_SIZE; x++) {
            sm->depth_grid[y][x] = 999.0; // Set to large background depth
        }
    }
}

bool tsfi_shadowmap_set_depth(TSFiShadowMap *sm, int x, int y, double depth) {
    if (!sm || x < 0 || x >= TSFI_SHADOW_SIZE || y < 0 || y >= TSFI_SHADOW_SIZE) return false;
    sm->depth_grid[y][x] = depth;
    return true;
}

bool tsfi_shadowmap_is_in_shadow(const TSFiShadowMap *sm, int x, int y, double point_depth) {
    if (!sm || x < 0 || x >= TSFI_SHADOW_SIZE || y < 0 || y >= TSFI_SHADOW_SIZE) return false;
    
    // Constant-time O(1) depth comparison (epsilon factor applied to prevent surface self-shadowing)
    double threshold_epsilon = 0.01;
    if (point_depth > sm->depth_grid[y][x] + threshold_epsilon) {
        return true; // Occluded by closer geometry
    }
    return false; // Illuminated
}

void tsfi_shadowmap_pack_xplsm(const TSFiShadowMap *sm, uint8_t *dest_xpl_buffer) {
    if (!sm || !dest_xpl_buffer) return;
    
    for (int y = 0; y < TSFI_SHADOW_SIZE; y++) {
        for (int x = 0; x < TSFI_SHADOW_SIZE; x++) {
            double depth = sm->depth_grid[y][x];
            // Clamp and scale to fit into an 8-bit quantized byte representation (0-255)
            if (depth < 0.0) depth = 0.0;
            if (depth > 255.0) depth = 255.0;
            dest_xpl_buffer[y * TSFI_SHADOW_SIZE + x] = (uint8_t)depth;
        }
    }
}

void tsfi_shadowmap_print_grid(const TSFiShadowMap *sm) {
    if (!sm) return;
    for (int y = 0; y < TSFI_SHADOW_SIZE; y++) {
        for (int x = 0; x < TSFI_SHADOW_SIZE; x++) {
            double depth = sm->depth_grid[y][x];
            if (depth > 900.0) {
                printf(". "); // Background/unwritten depth
            } else if (depth > 10.0) {
                printf("# "); // Medium depth
            } else {
                printf("@ "); // Close depth
            }
        }
        printf("\n");
    }
}

void tsfi_shadowmap_reflect_substrate(TSFiShadowMap *sm, const uint8_t *frame_buffer, int width, int height) {
    if (!sm || !frame_buffer || width <= 0 || height <= 0) return;
    
    // Map and downsample the frame buffer coordinates into the 16x16 shadow map grid
    for (int y = 0; y < TSFI_SHADOW_SIZE; y++) {
        int src_y = (y * height) / TSFI_SHADOW_SIZE;
        for (int x = 0; x < TSFI_SHADOW_SIZE; x++) {
            int src_x = (x * width) / TSFI_SHADOW_SIZE;
            uint8_t pixel_val = frame_buffer[src_y * width + src_x];
            // Normalize pixel values (0-255) as reflected depth points
            sm->depth_grid[y][x] = (double)pixel_val;
        }
    }
}
