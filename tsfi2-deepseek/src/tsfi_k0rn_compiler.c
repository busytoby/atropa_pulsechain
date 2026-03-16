#include "tsfi_k0rn_ops.h"
#include "lau_memory.h"
#include "tsfi_wiring.h"
#include "tsfi_io.h"
#include "tsfi_c_math.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define sinf tsfi_sinf
#define cosf tsfi_cosf

// --- Geometric Component Helpers ---

static void k0rn_add_op(K0RnStream *s, K0RnOp op) {
    if (s->op_count < s->stream_capacity) {
        s->ops[s->op_count++] = op;
    }
}

void k0rn_add_stem(K0RnStream *s, float x, float y1, float y2, float rad) {
    k0rn_add_op(s, (K0RnOp){ .type=K0RN_OP_CAPSULE, .p1=rad, .x=x, .y=y1, .z=x, .w=y2 });
}

void k0rn_add_bar(K0RnStream *s, float y, float x1, float x2, float rad) {
    k0rn_add_op(s, (K0RnOp){ .type=K0RN_OP_CAPSULE, .p1=rad, .x=x1, .y=y, .z=x2, .w=y });
}

void k0rn_add_spine(K0RnStream *s, float x1, float y1, float x2, float y2, float rad) {
    k0rn_add_op(s, (K0RnOp){ .type=K0RN_OP_CAPSULE, .p1=rad, .x=x1, .y=y1, .z=x2, .w=y2 });
}

void k0rn_add_arc(K0RnStream *s, float cx, float cy, float r, float a1, float a2, float rad, int steps) {
    float step_size = (a2 - a1) / (float)steps;
    float overlap = step_size * 0.2f; // Add 20% overlap to prevent gaps/dots
    for (int i = 0; i < steps; i++) {
        float angle = a1 + (float)i * step_size;
        float next_angle = angle + step_size + overlap;
        k0rn_add_spine(s, cx + cosf(angle) * r, cy + sinf(angle) * r,
                          cx + cosf(next_angle) * r, cy + sinf(next_angle) * r, rad);
    }
}

// --- The Teddy Bear Genome ---
K0RnStream* tsfi_k0rn_compile_teddy(void) {
    K0RnStream *s = (K0RnStream*)lau_malloc_wired(sizeof(K0RnStream));
    s->stream_capacity = 16;
    s->op_count = 8;
    s->ops = (K0RnOp*)lau_memalign(512, sizeof(K0RnOp) * s->stream_capacity);
    
    int i = 0;
    // Note: pb (z,w) = pa (x,y) for spheres
    // 0: Body (Capsule)
    s->ops[i++] = (K0RnOp){ .type=K0RN_OP_CAPSULE, .p1=0.25f, .x=0, .y=-0.2f, .z=0, .w=0.2f };
    // 1: Head (Sphere)
    s->ops[i++] = (K0RnOp){ .type=K0RN_OP_SPHERE, .p1=0.2f, .x=0, .y=0.25f, .z=0, .w=0.25f };
    // 2: Left Ear
    s->ops[i++] = (K0RnOp){ .type=K0RN_OP_SPHERE, .p1=0.08f, .x=-0.15f, .y=0.4f, .z=-0.15f, .w=0.4f };
    // 3: Right Ear
    s->ops[i++] = (K0RnOp){ .type=K0RN_OP_SPHERE, .p1=0.08f, .x=0.15f, .y=0.4f, .z=0.15f, .w=0.4f };
    // 4: Left Arm
    s->ops[i++] = (K0RnOp){ .type=K0RN_OP_CAPSULE, .p1=0.08f, .x=-0.3f, .y=-0.1f, .z=-0.4f, .w=0.1f };
    // 5: Right Arm
    s->ops[i++] = (K0RnOp){ .type=K0RN_OP_CAPSULE, .p1=0.08f, .x=0.3f, .y=-0.1f, .z=0.4f, .w=0.1f };
    // 6: Left Leg
    s->ops[i++] = (K0RnOp){ .type=K0RN_OP_CAPSULE, .p1=0.09f, .x=-0.15f, .y=-0.5f, .z=-0.15f, .w=-0.2f };
    // 7: Right Leg
    s->ops[i++] = (K0RnOp){ .type=K0RN_OP_CAPSULE, .p1=0.09f, .x=0.15f, .y=-0.5f, .z=0.15f, .w=-0.2f };
    
    return s;
}

void tsfi_k0rn_animate_teddy(K0RnStream *s, float t) {
    if (!s || s->op_count < 8) return;
    s->ops[4].z = -0.4f + cosf(t * 5.0f) * 0.1f;
    s->ops[4].w = 0.1f + sinf(t * 5.0f) * 0.1f;
    s->ops[5].z = 0.4f + cosf(t * 5.0f + ((float)TSFI_SECRET_CORE)) * 0.1f;
    s->ops[5].w = 0.1f + sinf(t * 5.0f + ((float)TSFI_SECRET_CORE)) * 0.1f;
}

K0RnStream* tsfi_k0rn_merge_streams(K0RnStream **streams, int count) {
    uint32_t total_ops = 0;
    for (int i = 0; i < count; i++) if (streams[i]) total_ops += streams[i]->op_count;
    K0RnStream *res = (K0RnStream*)lau_malloc_wired(sizeof(K0RnStream));
    res->stream_capacity = total_ops;
    res->op_count = total_ops;
    res->ops = (K0RnOp*)lau_memalign(512, sizeof(K0RnOp) * total_ops);
    int current_op = 0;
    for (int i = 0; i < count; i++) {
        if (!streams[i]) continue;
        memcpy(&res->ops[current_op], streams[i]->ops, sizeof(K0RnOp) * streams[i]->op_count);
        current_op += streams[i]->op_count;
    }
    return res;
}

K0RnStream* tsfi_k0rn_compile_char(uint32_t c) {
    K0RnStream *s = (K0RnStream*)lau_malloc_wired(sizeof(K0RnStream));
    s->stream_capacity = 32;
    s->op_count = 0;
    s->ops = (K0RnOp*)lau_memalign(512, sizeof(K0RnOp) * s->stream_capacity);

    if (c == 0x41) { // 'A'
        // Explicit Symmetric 'A' (5 segments)
        k0rn_add_spine(s, -0.4f, -0.7f, -0.2f, 0.0f, 0.08f); // Bottom-left to Mid-left
        k0rn_add_spine(s, -0.2f,  0.0f,  0.0f, 0.7f, 0.08f); // Mid-left to Top-center
        k0rn_add_spine(s,  0.4f, -0.7f,  0.2f, 0.0f, 0.08f); // Bottom-right to Mid-right
        k0rn_add_spine(s,  0.2f,  0.0f,  0.0f, 0.7f, 0.08f); // Mid-right to Top-center
        k0rn_add_bar(s, -0.1f, -0.25f, 0.25f, 0.07f);        // Crossbar
    } else if (c == 0x42) {
        // Vertical Stem
        k0rn_add_stem(s, -0.3f, -0.6f, 0.6f, 0.08f);
        // Upper Bowl
        k0rn_add_arc(s, -0.3f, 0.3f, 0.3f, -1.57f, 1.57f, 0.07f, 12);
        // Lower Bowl
        k0rn_add_arc(s, -0.3f, -0.3f, 0.3f, -1.57f, 1.57f, 0.07f, 12);
                    } else if (c == 0x327E) { // ㉾ (Circled Ieung)
        // Increase capacity
        // 24 (Outer) + 16 (Inner) + 3 (T) = 43 ops.
        K0RnOp *new_ops = (K0RnOp*)lau_memalign(512, sizeof(K0RnOp) * 64);
        memcpy(new_ops, s->ops, sizeof(K0RnOp) * s->op_count);
        lau_free(s->ops);
        s->ops = new_ops;
        s->stream_capacity = 64;

        // Outer Circle (Smooth 24 segments, Thicker 0.08 for stability)
        k0rn_add_arc(s, 0.0f, 0.0f, 0.8f, 0.0f, 6.283f, 0.08f, 24);
        
        // Inner Circle (Smooth 16 segments, Thicker 0.10f to ensure merge)
        k0rn_add_arc(s, 0.0f, 0.25f, 0.25f, 0.0f, 6.283f, 0.10f, 16);
        
        // T-Shape (Spines) - Already Thicker (0.08)
        k0rn_add_spine(s, -0.25f, -0.25f, 0.25f, -0.25f, 0.08f);
        k0rn_add_spine(s, 0.0f, -0.25f, 0.0f, -0.6f, 0.08f);
    } else {        // Broad Asset Recognition
        char asset_name[32]; snprintf(asset_name, 31, "0x%X", c);
        K0RnStream *loaded = tsfi_k0rn_load(asset_name);
        if (loaded) {
            lau_free(s->ops); lau_free(s);
            return loaded;
        }

        // Fallback: Procedural Stroke-Walker for unknown chars
        uint32_t state = c * 0x45D9F3B + 0x11111111;
        state = (state << 13) ^ state;
        state = state * 1664525 + 1013904223;
    
        int num_strokes = (state % 4) + 3; 
        bool symmetry_x = (state & 0x80000000);
        float rad_f = 0.04f + ((state >> 24) & 0xF) * 0.003f;
        
        int current_pt = (state >> 16) % 15;
        for (int i = 0; i < num_strokes; i++) {
            state = state * 1664525 + 1013904223;
            int next_pt = (state >> 8) % 15;
            float x1 = ((current_pt % 3) - 1.0f) * 0.45f;
            float y1 = ((current_pt / 3) - 2.0f) * -0.35f;
            float x2 = ((next_pt % 3) - 1.0f) * 0.45f;
            float y2 = ((next_pt / 3) - 2.0f) * -0.35f;
            
            k0rn_add_spine(s, x1, y1, x2, y2, rad_f);
            if (symmetry_x) k0rn_add_spine(s, -x1, y1, -x2, y2, rad_f);
            current_pt = next_pt;
        }
    }
    return s;
}

K0RnStream* tsfi_k0rn_interpolate(K0RnStream *a, K0RnStream *b, float t) {
    uint32_t max_ops = (a->op_count > b->op_count) ? a->op_count : b->op_count;
    K0RnStream *res = (K0RnStream*)lau_malloc_wired(sizeof(K0RnStream));
    res->stream_capacity = max_ops;
    res->op_count = max_ops;
    res->ops = (K0RnOp*)lau_memalign(512, sizeof(K0RnOp) * max_ops);
    for (uint32_t i = 0; i < max_ops; i++) {
        K0RnOp opA = (i < a->op_count) ? a->ops[i] : (K0RnOp){.type=K0RN_OP_SPHERE, .p1=0};
        K0RnOp opB = (i < b->op_count) ? b->ops[i] : (K0RnOp){.type=K0RN_OP_SPHERE, .p1=0};
        res->ops[i].type = (t < 0.5f) ? opA.type : opB.type;
        res->ops[i].p1 = opA.p1 + (opB.p1 - opA.p1) * t;
        res->ops[i].p2 = opA.p2 + (opB.p2 - opA.p2) * t;
        res->ops[i].x  = opA.x  + (opB.x  - opA.x)  * t;
        res->ops[i].y  = opA.y  + (opB.y  - opA.y)  * t;
        res->ops[i].z  = opA.z  + (opB.z  - opA.z)  * t;
        res->ops[i].w  = opA.w  + (opB.w  - opA.w)  * t;
    }
    return res;
}
