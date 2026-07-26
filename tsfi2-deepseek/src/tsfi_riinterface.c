#include "tsfi_riinterface.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

void tsfi_riinterface_init(TSFiRiInterface *ri) {
    if (!ri) return;
    memset(ri->hudson_vce_color_reg, 0, sizeof(ri->hudson_vce_color_reg));
    ri->active_sprite_id = 0;
    ri->clip_min_x = 0.0;
    ri->clip_min_y = 0.0;
    ri->clip_max_x = 256.0; // Align coordinate clipping limits with simulated VDC resolution
    ri->clip_max_y = 256.0;
    ri->is_world_active = false;
    memset(ri->psg_channel_freq, 0, sizeof(ri->psg_channel_freq));
    memset(ri->psg_channel_vol, 0, sizeof(ri->psg_channel_vol));
    memset(ri->frame_buffer, 0, sizeof(ri->frame_buffer));
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

void tsfi_riinterface_modulate_psg(TSFiRiInterface *ri, double velocity) {
    if (!ri) return;
    
    // Translate rendering velocity parameters directly to audio frequency and volume parameters
    uint16_t base_freq = (uint16_t)(350 + (int)(velocity * 50.0));
    for (int i = 0; i < 6; i++) {
        ri->psg_channel_freq[i] = base_freq + (i * 20);
        ri->psg_channel_vol[i] = (uint8_t)(10 + (int)(velocity * 2.0));
        if (ri->psg_channel_vol[i] > 31) ri->psg_channel_vol[i] = 31;
    }
}

void tsfi_riinterface_vdc_dma_copy(TSFiRiInterface *ri, uint16_t src_idx, uint16_t dest_idx, uint16_t length) {
    if (!ri || src_idx >= 16 || dest_idx >= 16) return;
    
    // Perform block copies of color palette registers to emulate hardware DMA loops
    for (uint16_t i = 0; i < length && (src_idx + i < 16) && (dest_idx + i < 16); i++) {
        ri->hudson_vce_color_reg[dest_idx + i] = ri->hudson_vce_color_reg[src_idx + i];
    }
}

void tsfi_riinterface_discharge_verlet(TSFiRiInterface *ri, double *pos_x, double *prev_pos_x, int count, double dt, double decay) {
    if (!ri || !pos_x || !prev_pos_x || count <= 0) return;
    
    // Soft body physics (Verlet integration) applied strictly to FET discharge cycles
    for (int i = 0; i < count; i++) {
        double current = pos_x[i];
        double prev = prev_pos_x[i];
        
        // Verlet equation: x_new = x_curr + (x_curr - x_prev) * decay - (discharge_rate * dt^2)
        double next = current + (current - prev) * decay - (0.05 * dt * dt);
        
        prev_pos_x[i] = current;
        pos_x[i] = next;
        
        // Mirror the active discharge levels to simulated VDC frame buffer registers
        int idx = i % 256;
        ri->frame_buffer[idx] = (uint8_t)(fabs(next) * 10.0);
    }
}
