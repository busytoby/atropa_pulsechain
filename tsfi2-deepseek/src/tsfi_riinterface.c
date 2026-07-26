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
    memset(ri->psg_channel_pan, 0, sizeof(ri->psg_channel_pan));
    memset(ri->psg_channel_wavetable, 0, sizeof(ri->psg_channel_wavetable));
    ri->psg_lfo_ctrl = 0;
    ri->psg_lfo_freq = 0;
    ri->psg_lfo_phase = 0;
    memset(ri->psg_noise_ctrl, 0, sizeof(ri->psg_noise_ctrl));
    memset(ri->psg_channel_dda, 0, sizeof(ri->psg_channel_dda));
    ri->vdc_scroll_x = 0;
    ri->vdc_scroll_y = 0;
    ri->vdc_raster_interrupt_line = 0;
    ri->vdc_collision_flag = false;
    ri->hblank_active = false;
    memset(ri->frame_buffer, 0, sizeof(ri->frame_buffer));
    ri->irq_counter = 0;
    ri->irq_active = false;
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
        
        // Map pan based on channel index parameters: channel 0-2 left-heavy, 3-5 right-heavy
        ri->psg_channel_pan[i] = (i < 3) ? 0xF0 : 0x0F;
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

void tsfi_riinterface_tick_irq(TSFiRiInterface *ri, double dt) {
    if (!ri) return;
    
    // Trigger periodic timer IRQ interrupts on 15ms thresholds
    static double accumulator = 0.0;
    accumulator += dt;
    if (accumulator >= 0.015) {
        ri->irq_counter++;
        ri->irq_active = true;
        accumulator = 0.0;
        
        // Cycle colors on IRQ fire to demonstrate dynamic interrupt-driven sync
        uint16_t first_color = ri->hudson_vce_color_reg[0];
        for (int i = 0; i < 15; i++) {
            ri->hudson_vce_color_reg[i] = ri->hudson_vce_color_reg[i + 1];
        }
        ri->hudson_vce_color_reg[15] = first_color;
    } else {
        ri->irq_active = false;
    }
}

void tsfi_riinterface_run_8step_loop(TSFiRiInterface *ri, double camera_velocity, double *pos_x, double *prev_pos_x, int count) {
    if (!ri) return;
    
    // Stage 1: RiBegin - CPU/VDC Initialization
    tsfi_riinterface_init(ri);
    
    // Stage 2: RiFrameBegin - Trigger initial timer IRQ tick
    tsfi_riinterface_tick_irq(ri, 0.016);
    
    // Stage 3: RiCamera - Map camera velocity to PSG tone
    tsfi_riinterface_modulate_psg(ri, camera_velocity);
    
    // Stage 4: RiWorldBegin - Establish VCE palette base
    tsfi_riinterface_world_begin(ri);
    
    // Stage 5: Traversal (`RiSphere`) - Emit geometry to VDC active sprite registry
    tsfi_riinterface_sphere(ri, 1, 8.0);
    
    // Stage 6: RiWorldEnd - Verlet FET discharge calculations written to display memory
    tsfi_riinterface_discharge_verlet(ri, pos_x, prev_pos_x, count, 0.1, 0.99);
    
    // Stage 7: RiFrameEnd - Execute VDC DMA hardware block transfer
    tsfi_riinterface_vdc_dma_copy(ri, 1, 5, 2);
    
    // Stage 8: RiEnd - Shutdown world scope
    tsfi_riinterface_world_end(ri);
}

void tsfi_riinterface_write_wavetable(TSFiRiInterface *ri, int channel, const uint8_t *wave_data) {
    if (!ri || !wave_data || channel < 0 || channel >= 6) return;
    
    for (int i = 0; i < 32; i++) {
        // Enforce 5-bit depth constraint (value limit 0-31) matching HuC6280 PSG wavetable registers
        ri->psg_channel_wavetable[channel][i] = wave_data[i] & 0x1F;
    }
}

void tsfi_riinterface_enable_lfo(TSFiRiInterface *ri, uint8_t ctrl, uint8_t freq) {
    if (!ri) return;
    ri->psg_lfo_ctrl = ctrl;
    ri->psg_lfo_freq = freq;
}

void tsfi_riinterface_set_lfo_phase(TSFiRiInterface *ri, uint8_t phase) {
    if (!ri) return;
    ri->psg_lfo_phase = phase;
}

void tsfi_riinterface_enable_noise(TSFiRiInterface *ri, int channel, bool enable, uint8_t frequency) {
    if (!ri || channel < 4 || channel > 5) return;
    
    int idx = channel - 4;
    // Map noise configuration (Bit 7: enable, Bits 0-4: noise frequency parameter)
    ri->psg_noise_ctrl[idx] = (enable ? 0x80 : 0x00) | (frequency & 0x1F);
}

void tsfi_riinterface_scroll_background(TSFiRiInterface *ri, uint16_t x, uint16_t y) {
    if (!ri) return;
    ri->vdc_scroll_x = x;
    ri->vdc_scroll_y = y;
}

void tsfi_riinterface_write_dda(TSFiRiInterface *ri, int channel, uint8_t sample) {
    if (!ri || channel < 0 || channel >= 6) return;
    
    // Clamp to 5-bit depth constraint (value limit 0-31) matching HuC6280 PSG D/A registers
    ri->psg_channel_dda[channel] = sample & 0x1F;
}

void tsfi_riinterface_check_sprite_collision(TSFiRiInterface *ri, double s1_x, double s1_y, double s2_x, double s2_y, double threshold) {
    if (!ri) return;
    
    double dx = s1_x - s2_x;
    double dy = s1_y - s2_y;
    double distance = sqrt(dx * dx + dy * dy);
    
    if (distance <= threshold) {
        ri->vdc_collision_flag = true;
    } else {
        ri->vdc_collision_flag = false;
    }
}

void tsfi_riinterface_set_raster_line(TSFiRiInterface *ri, uint16_t line) {
    if (!ri) return;
    ri->vdc_raster_interrupt_line = line;
    ri->hblank_active = true; // Trigger H-Blank interrupt cycle simulation
}
