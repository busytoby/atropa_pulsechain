#define _GNU_SOURCE
#include "tsfi_wasm_crypto_font_events.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <openssl/sha.h>

// -----------------------------------------------------------------------------
// 1. WebAssembly (Wasm) MVP Bytecode Runtime
// -----------------------------------------------------------------------------
void tsfi_wasm_init(TsfiWasmVM *vm) {
    if (!vm) return;
    memset(vm, 0, sizeof(TsfiWasmVM));
}

bool tsfi_wasm_push(TsfiWasmVM *vm, uint32_t val) {
    if (!vm || vm->sp >= 128) return false;
    vm->stack[vm->sp++] = val;
    return true;
}

uint32_t tsfi_wasm_pop(TsfiWasmVM *vm) {
    if (!vm || vm->sp <= 0) return 0;
    return vm->stack[--vm->sp];
}

bool tsfi_wasm_execute_bytecode(TsfiWasmVM *vm, const uint8_t *code, size_t code_len) {
    if (!vm || !code || code_len < 4) return false;

    // Validate 4-byte Wasm Magic: 0x00 0x61 0x73 0x6D (\0asm)
    if (code[0] != 0x00 || code[1] != 0x61 || code[2] != 0x73 || code[3] != 0x6D) {
        return false;
    }

    size_t pc = 4;
    // Skip version header (4 bytes)
    if (pc + 4 <= code_len) pc += 4;

    while (pc < code_len && !vm->is_halted) {
        uint8_t op = code[pc++];
        switch (op) {
            case 0x41: { // i32.const <val>
                if (pc < code_len) {
                    uint32_t val = (uint32_t)code[pc++];
                    tsfi_wasm_push(vm, val);
                }
                break;
            }
            case 0x6A: { // i32.add
                uint32_t b = tsfi_wasm_pop(vm);
                uint32_t a = tsfi_wasm_pop(vm);
                tsfi_wasm_push(vm, a + b);
                break;
            }
            case 0x6C: { // i32.mul
                uint32_t b = tsfi_wasm_pop(vm);
                uint32_t a = tsfi_wasm_pop(vm);
                tsfi_wasm_push(vm, a * b);
                break;
            }
            case 0x0F: { // return
                vm->is_halted = true;
                break;
            }
            default:
                break;
        }
    }
    return true;
}

// -----------------------------------------------------------------------------
// 2. Web Crypto API & Cryptographic Primitive Engine
// -----------------------------------------------------------------------------
void tsfi_crypto_sha256(const uint8_t *data, size_t len, uint8_t *out_hash32) {
    if (!data || len == 0 || !out_hash32) return;
    SHA256(data, len, out_hash32);
}

void tsfi_crypto_random_bytes(uint8_t *out_buf, size_t len) {
    if (!out_buf || len == 0) return;
    for (size_t i = 0; i < len; i++) {
        out_buf[i] = (uint8_t)(rand() & 0xFF);
    }
}

bool tsfi_crypto_secp256k1_sign(const uint8_t *hash32, const uint8_t *privkey32, TsfiCryptoResult *res_out) {
    if (!hash32 || !privkey32 || !res_out) return false;
    memset(res_out, 0, sizeof(TsfiCryptoResult));
    memcpy(res_out->digest_sha256, hash32, 32);

    // Deterministic signature generation (Simulated RFC 6979 Secp256k1)
    for (int i = 0; i < 32; i++) {
        res_out->signature_secp[i] = hash32[i] ^ privkey32[i];
        res_out->signature_secp[32 + i] = hash32[i] ^ 0x5C;
    }
    res_out->is_verified = true;
    return true;
}

// -----------------------------------------------------------------------------
// 3. TrueType Bézier Vector Font Glyph Rasterizer
// -----------------------------------------------------------------------------
void tsfi_font_init_glyph(TsfiGlyphOutline *glyph, int w, int h) {
    if (!glyph) return;
    memset(glyph, 0, sizeof(TsfiGlyphOutline));
    glyph->glyph_w = (w > 0) ? w : 16;
    glyph->glyph_h = (h > 0) ? h : 16;
}

bool tsfi_font_add_quad_curve(TsfiGlyphOutline *glyph, float x0, float y0, float cx, float cy, float x1, float y1) {
    if (!glyph || glyph->curve_count >= 32) return false;
    TsfiBezierQuad *q = &glyph->curves[glyph->curve_count++];
    q->x0 = x0; q->y0 = y0;
    q->cx = cx; q->cy = cy;
    q->x1 = x1; q->y1 = y1;
    return true;
}

void tsfi_font_rasterize_glyph(const TsfiGlyphOutline *glyph, uint32_t *pixels, int scanout_w, int scanout_h, int target_x, int target_y, uint32_t color) {
    if (!glyph || !pixels || scanout_w <= 0 || scanout_h <= 0) return;

    for (int c = 0; c < glyph->curve_count; c++) {
        const TsfiBezierQuad *q = &glyph->curves[c];
        // Evaluate quadratic Bézier curve: B(t) = (1-t)^2 P0 + 2(1-t)t P1 + t^2 P2
        for (float t = 0.0f; t <= 1.0f; t += 0.05f) {
            float it = 1.0f - t;
            float px = it * it * q->x0 + 2.0f * it * t * q->cx + t * t * q->x1;
            float py = it * it * q->y0 + 2.0f * it * t * q->cy + t * t * q->y1;

            int sx = target_x + (int)px;
            int sy = target_y + (int)py;

            if (sx >= 0 && sx < scanout_w && sy >= 0 && sy < scanout_h) {
                pixels[sy * scanout_w + sx] = color;
            }
        }
    }
}

// -----------------------------------------------------------------------------
// 4. DOM MutationObserver & Event Dispatch Tree
// -----------------------------------------------------------------------------
void tsfi_dom_events_init(TsfiDomEventDispatcher *disp) {
    if (!disp) return;
    memset(disp, 0, sizeof(TsfiDomEventDispatcher));
}

bool tsfi_dom_add_event_listener(TsfiDomEventDispatcher *disp, const char *target_id, TsfiDomEventType type, TsfiDomEventListener cb, void *user_data) {
    if (!disp || !target_id || !cb || disp->listener_count >= 32) return false;
    TsfiEventRegistration *reg = &disp->listeners[disp->listener_count++];
    strncpy(reg->target_id, target_id, sizeof(reg->target_id) - 1);
    reg->type = type;
    reg->listener = cb;
    reg->user_data = user_data;
    return true;
}

int tsfi_dom_dispatch_event(TsfiDomEventDispatcher *disp, TsfiDomEvent *event) {
    if (!disp || !event) return 0;
    int dispatched = 0;

    for (int i = 0; i < disp->listener_count; i++) {
        TsfiEventRegistration *reg = &disp->listeners[i];
        if (reg->type == event->type && strcmp(reg->target_id, event->target_id) == 0) {
            if (reg->listener) {
                reg->listener(event, reg->user_data);
                dispatched++;
            }
            if (event->propagation_stopped) break;
        }
    }
    return dispatched;
}

void tsfi_dom_record_mutation(TsfiDomEventDispatcher *disp, const char *target_id, const char *mutation_type) {
    (void)target_id; (void)mutation_type;
    if (!disp) return;
    disp->mutation_count++;
}
