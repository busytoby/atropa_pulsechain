#ifndef TSFI_K0RN_OPS_H
#define TSFI_K0RN_OPS_H

#include <stdint.h>
#include "tsfi_types.h"

// Match the layout in logical_pipeline_thunks/tsfi_font.comp
// We use 16-byte alignment to match vec4/GPU requirements.

typedef struct {
    uint32_t type;      // 0=Sphere, 1=Box, 2=Capsule
    uint32_t flags;     // 0=Leaf, 1=Node
    float    p1;        // Radius / Width / K-factor
    float    p2;        // Height / Length
    float    x, y, z, w; // Position & Orientation (vec4)
    uint32_t padding[8]; // Pad to 64 bytes (512-bit)
} __attribute__((packed)) K0RnOp;

// The K0Rn Stream is now a Living Wave System.
// It contains the Genome (Ops) and the Logic (Thunks) to evolve itself.
DEFINE_MAPPED_STRUCT(K0RnStream,
    uint32_t op_count;
    uint32_t stream_capacity;
    uint32_t _pad[2];   
    K0RnOp   *ops;      // Pointer to the op buffer (Allocated separately or embedded?)
                        // Let's keep it simple: Pointer for now.
)

// Op Types
#define K0RN_OP_SPHERE  0
#define K0RN_OP_BOX     1
#define K0RN_OP_CAPSULE 2
#define K0RN_OP_UNION   3
#define K0RN_OP_SMOOTH  4
#define K0RN_OP_SUB     5

// API for Stream Manipulation
K0RnStream* tsfi_k0rn_interpolate(K0RnStream *a, K0RnStream *b, float t);
K0RnStream* tsfi_k0rn_merge_streams(K0RnStream **streams, int count);
K0RnStream* tsfi_k0rn_compile_char(uint32_t c);
K0RnStream* tsfi_k0rn_compile_teddy(void);

#endif
