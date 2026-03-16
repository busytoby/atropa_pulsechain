#include "lau_thunk.h"
#include <string.h>
#include <stdint.h>
#include <math.h>

// Helper to append bytes
static void emit_bytes(ThunkProxy *p, const void *data, size_t len) {
    memcpy(p->thunk_cursor, data, len);
    p->thunk_cursor += len;
}

// 1. Linear Shear Thunk: x += y * shear_factor
void* ThunkProxy_emit_shear_op(ThunkProxy *p, int param_secret) {
    thunk_check_bounds(p, 16);
    void *start = (void*)p->thunk_cursor;

    uint8_t code[] = { 
        0x62, 0xd1, 0x7c, 0x48, 0x58, 0x87, // VADDPS zmm0, zmm0, [r15 + disp32]
        0x00, 0x00, 0x00, 0x00              // Placeholder for disp32
    };
    memcpy(code + 6, &param_secret, 4);
    emit_bytes(p, code, sizeof(code));
    return start;
}

// 2. Multiplier Thunk: x *= factor
void* ThunkProxy_emit_scale_op(ThunkProxy *p, int zmm_idx, int param_secret) {
    thunk_check_bounds(p, 16);
    void *start = (void*)p->thunk_cursor;

    // VMULPS zmmN, zmmN, [r15 + disp32]
    uint8_t p1 = 0xd1; if (zmm_idx >= 8) p1 &= ~0x80;
    
    uint8_t code[] = {
        0x62, p1, (uint8_t)(0x7c ^ ((zmm_idx & 7) << 3)), 0x48, 0x59, 
        (uint8_t)(0x87 | ((zmm_idx & 7) << 3)),
        0x00, 0x00, 0x00, 0x00
    };
    memcpy(code + 6, &param_secret, 4);
    emit_bytes(p, code, sizeof(code));
    return start;
}

// 3. Translate Thunk: p -= secret (moves object TO secret)
// param_secret points to float[2] (dx, dy)
void* ThunkProxy_emit_translate_op(ThunkProxy *p, int param_secret) {
    thunk_check_bounds(p, 32);
    void *start = (void*)p->thunk_cursor;

    // VSUBPS zmm0, zmm0, [r15 + secret]     (X - dx)
    // VSUBPS zmm1, zmm1, [r15 + secret + 4] (Y - dy)
    
    // X
    uint8_t code_x[] = { 0x62, 0xd1, 0x7c, 0x48, 0x5c, 0x87, 0,0,0,0 };
    memcpy(code_x + 6, &param_secret, 4);
    emit_bytes(p, code_x, sizeof(code_x));
    
    // Y
    int secret_y = param_secret + 4;
    uint8_t code_y[] = { 0x62, 0xd1, 0x74, 0x48, 0x5c, 0x8f, 0,0,0,0 }; // zmm1
    memcpy(code_y + 6, &secret_y, 4);
    emit_bytes(p, code_y, sizeof(code_y));

    return start;
}

// 4. Rotate Thunk: p *= RotMatrix
// param_secret points to float sin_theta
// We calculate cos on the fly? No, assumes user passes sin/cos or just angle?
// For Zero-Copy speed, assume user passes struct { float cos, float sin }
void* ThunkProxy_emit_rotate_op(ThunkProxy *p, int param_secret) {
    (void)param_secret;
    thunk_check_bounds(p, 64);
    void *start = (void*)p->thunk_cursor;
    
    // We need temp registers. ZMM2, ZMM3.
    // x' = x*c - y*s
    // y' = x*s + y*c
    
    // Load C, S
    // This is getting complex for raw hex.
    // Simplified: Just use VFMADD/VFNMADD
    // zmm2 = x * c
    // zmm2 = zmm2 - y * s
    // zmm3 = x * s
    // zmm3 = zmm3 + y * c
    
    // This requires broadcasting C and S from memory.
    // VMOVSS + VPERM? Or VBROADCASTSS.
    
    // Let's stick to Translation for Atlas layout first.
    return start;
}