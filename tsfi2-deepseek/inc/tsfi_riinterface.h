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

#endif // TSFI_RIINTERFACE_H
