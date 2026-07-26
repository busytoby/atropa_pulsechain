#include "tsfi_riinterface.h"
#include <string.h>

void tsfi_riinterface_init(TSFiRiInterface *ri) {
    if (!ri) return;
    memset(ri->hudson_vce_color_reg, 0, sizeof(ri->hudson_vce_color_reg));
    ri->active_sprite_id = 0;
    ri->clip_min_x = 0.0;
    ri->clip_min_y = 0.0;
    ri->clip_max_x = 640.0;
    ri->clip_max_y = 480.0;
    ri->is_world_active = false;
}

void tsfi_riinterface_world_begin(TSFiRiInterface *ri) {
    if (!ri) return;
    ri->is_world_active = true;
    
    // Cycle/resets the mirrored Hudson VCE color palette registers (RGB565 format: Red)
    for (int i = 0; i < 16; i++) {
        ri->hudson_vce_color_reg[i] = 0xF800; 
    }
}

void tsfi_riinterface_world_end(TSFiRiInterface *ri) {
    if (!ri) return;
    ri->is_world_active = false;
}

bool tsfi_riinterface_sphere(TSFiRiInterface *ri, int sprite_id, double radius) {
    if (!ri || !ri->is_world_active || radius <= 0.0) return false;
    
    // Mirror sphere coordinate boundary details to the active Hudson VDC sprite layer index
    ri->active_sprite_id = sprite_id;
    
    // Update Hudson VCE palette value based on sprite diameter (Green color scale)
    uint16_t color_intensity = (uint16_t)(radius * 2.0);
    if (color_intensity > 31) color_intensity = 31;
    ri->hudson_vce_color_reg[sprite_id % 16] = (color_intensity << 5);
    return true;
}

bool tsfi_riinterface_clip_check(const TSFiRiInterface *ri, double x, double y) {
    if (!ri) return false;
    
    // Emulates Tom Hudson's clipLine routine limits
    if (x >= ri->clip_min_x && x <= ri->clip_max_x &&
        y >= ri->clip_min_y && y <= ri->clip_max_y) {
        return true; // Inside boundaries, do not clip
    }
    return false; // Clipped
}
