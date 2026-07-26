#ifndef TSFI_RIINTERFACE_H
#define TSFI_RIINTERFACE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint16_t hudson_vce_color_reg[16];
    int active_sprite_id;
    double clip_min_x, clip_min_y;
    double clip_max_x, clip_max_y;
    bool is_world_active;
    uint16_t psg_channel_freq[6]; // huC6280 PSG voice channels
    uint8_t psg_channel_vol[6];
    uint8_t frame_buffer[256 * 256]; // Simulated VDC screen memory (256x256)
} TSFiRiInterface;

// Initialize the RiInterface context
void tsfi_riinterface_init(TSFiRiInterface *ri);

// RiWorldBegin callback: cycles/resets the mirrored Hudson VCE color palette registers
void tsfi_riinterface_world_begin(TSFiRiInterface *ri);

// RiWorldEnd callback
void tsfi_riinterface_world_end(TSFiRiInterface *ri);

// RiSphere callback: maps 3D sphere coordinate boundaries to the active Hudson VDC sprite layer
bool tsfi_riinterface_sphere(TSFiRiInterface *ri, int sprite_id, double radius);

// Emulates Tom Hudson's clipLine boundary checks for coordinate vectors
bool tsfi_riinterface_clip_check(const TSFiRiInterface *ri, double x, double y);

// Dynamic sound modulation based on RenderMan camera panning speed
void tsfi_riinterface_modulate_psg(TSFiRiInterface *ri, double velocity);

// Hardware-level VDC DMA block transfer simulation
void tsfi_riinterface_vdc_dma_copy(TSFiRiInterface *ri, uint16_t src_idx, uint16_t dest_idx, uint16_t length);

// Simulates the discharge cycles of field-effect transistors (FETs) using Verlet integration
void tsfi_riinterface_discharge_verlet(TSFiRiInterface *ri, double *pos_x, double *prev_pos_x, int count, double dt, double decay);

#endif // TSFI_RIINTERFACE_H
