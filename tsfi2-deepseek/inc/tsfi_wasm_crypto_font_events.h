#ifndef TSFI_WASM_CRYPTO_FONT_EVENTS_H
#define TSFI_WASM_CRYPTO_FONT_EVENTS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// -----------------------------------------------------------------------------
// 1. WebAssembly (Wasm) MVP Bytecode Runtime
// -----------------------------------------------------------------------------
typedef struct {
    uint32_t stack[128];
    int sp;
    uint8_t memory[4096];
    bool is_halted;
} TsfiWasmVM;

void tsfi_wasm_init(TsfiWasmVM *vm);
bool tsfi_wasm_push(TsfiWasmVM *vm, uint32_t val);
uint32_t tsfi_wasm_pop(TsfiWasmVM *vm);
bool tsfi_wasm_execute_bytecode(TsfiWasmVM *vm, const uint8_t *code, size_t code_len);

// -----------------------------------------------------------------------------
// 2. Web Crypto API & Cryptographic Primitive Engine (SubtleCrypto)
// -----------------------------------------------------------------------------
typedef struct {
    uint8_t digest_sha256[32];
    uint8_t signature_secp[64];
    bool is_verified;
} TsfiCryptoResult;

void tsfi_crypto_sha256(const uint8_t *data, size_t len, uint8_t *out_hash32);
void tsfi_crypto_random_bytes(uint8_t *out_buf, size_t len);
bool tsfi_crypto_secp256k1_sign(const uint8_t *hash32, const uint8_t *privkey32, TsfiCryptoResult *res_out);

// -----------------------------------------------------------------------------
// 3. TrueType Bézier Vector Font Glyph Rasterizer
// -----------------------------------------------------------------------------
typedef struct {
    float x0, y0;
    float cx, cy; // Control point
    float x1, y1;
} TsfiBezierQuad;

typedef struct {
    TsfiBezierQuad curves[32];
    int curve_count;
    int glyph_w;
    int glyph_h;
} TsfiGlyphOutline;

void tsfi_font_init_glyph(TsfiGlyphOutline *glyph, int w, int h);
bool tsfi_font_add_quad_curve(TsfiGlyphOutline *glyph, float x0, float y0, float cx, float cy, float x1, float y1);
void tsfi_font_rasterize_glyph(const TsfiGlyphOutline *glyph, uint32_t *pixels, int scanout_w, int scanout_h, int target_x, int target_y, uint32_t color);

// -----------------------------------------------------------------------------
// 4. DOM MutationObserver & Event Dispatch Tree
// -----------------------------------------------------------------------------
typedef enum {
    DOM_EVENT_CLICK = 0,
    DOM_EVENT_INPUT,
    DOM_EVENT_KEYPRESS
} TsfiDomEventType;

typedef struct {
    TsfiDomEventType type;
    char target_id[32];
    int mouse_x, mouse_y;
    int keycode;
    bool default_prevented;
    bool propagation_stopped;
} TsfiDomEvent;

typedef void (*TsfiDomEventListener)(const TsfiDomEvent *event, void *user_data);

typedef struct {
    char target_id[32];
    TsfiDomEventType type;
    TsfiDomEventListener listener;
    void *user_data;
} TsfiEventRegistration;

typedef struct {
    TsfiEventRegistration listeners[32];
    int listener_count;
    int mutation_count;
} TsfiDomEventDispatcher;

void tsfi_dom_events_init(TsfiDomEventDispatcher *disp);
bool tsfi_dom_add_event_listener(TsfiDomEventDispatcher *disp, const char *target_id, TsfiDomEventType type, TsfiDomEventListener cb, void *user_data);
int tsfi_dom_dispatch_event(TsfiDomEventDispatcher *disp, TsfiDomEvent *event);
void tsfi_dom_record_mutation(TsfiDomEventDispatcher *disp, const char *target_id, const char *mutation_type);

#endif // TSFI_WASM_CRYPTO_FONT_EVENTS_H
