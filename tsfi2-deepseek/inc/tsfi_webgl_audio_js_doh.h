#ifndef TSFI_WEBGL_AUDIO_JS_DOH_H
#define TSFI_WEBGL_AUDIO_JS_DOH_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// -----------------------------------------------------------------------------
// 1. WebGL / Vulkan Programmable Shader Pipeline
// -----------------------------------------------------------------------------
typedef struct {
    float x, y, z;
    uint32_t color;
} TsfiGlVertex;

typedef struct {
    TsfiGlVertex vertices[256];
    int vertex_count;
    float mvp_matrix[16];
    bool depth_test_enabled;
} TsfiWebGLContext;

void tsfi_webgl_init(TsfiWebGLContext *ctx);
bool tsfi_webgl_buffer_data(TsfiWebGLContext *ctx, const TsfiGlVertex *verts, int count);
void tsfi_webgl_set_uniform_mat4(TsfiWebGLContext *ctx, const float *mat4);
void tsfi_webgl_draw_arrays_to_scanout(const TsfiWebGLContext *ctx, uint32_t *pixels, int w, int h);

// -----------------------------------------------------------------------------
// 2. Web Audio Synthesizer & AudioContext (ALSA / PulseAudio PCM)
// -----------------------------------------------------------------------------
typedef enum {
    OSC_SINE = 0,
    OSC_SQUARE,
    OSC_SAWTOOTH,
    OSC_TRIANGLE
} TsfiOscillatorType;

typedef struct {
    TsfiOscillatorType type;
    float frequency_hz;
    float gain;
    float phase;
} TsfiWebAudioOscillator;

typedef struct {
    int sample_rate;
    TsfiWebAudioOscillator oscs[8];
    int osc_count;
} TsfiWebAudioContext;

void tsfi_web_audio_init(TsfiWebAudioContext *ctx, int sample_rate);
bool tsfi_web_audio_add_oscillator(TsfiWebAudioContext *ctx, TsfiOscillatorType type, float freq_hz, float gain);
size_t tsfi_web_audio_render_pcm16(TsfiWebAudioContext *ctx, int16_t *out_pcm, size_t sample_frames);

// -----------------------------------------------------------------------------
// 3. ECMAScript / MicroJS Bytecode VM for DOM
// -----------------------------------------------------------------------------
typedef enum {
    JS_OP_NOP = 0,
    JS_OP_SET_TEXT,
    JS_OP_SET_STYLE_COLOR,
    JS_OP_ADD_NUM,
    JS_OP_HALT
} TsfiJsOpCode;

typedef struct {
    TsfiJsOpCode op;
    char target_id[32];
    char string_arg[64];
    int int_arg;
} TsfiJsInstruction;

typedef struct {
    TsfiJsInstruction bytecode[64];
    int instruction_count;
    int accumulator;
    char last_dom_update[128];
} TsfiMicroJsVM;

void tsfi_micro_js_init(TsfiMicroJsVM *vm);
bool tsfi_micro_js_emit_instruction(TsfiMicroJsVM *vm, TsfiJsOpCode op, const char *target_id, const char *str_arg, int int_arg);
int tsfi_micro_js_exec(TsfiMicroJsVM *vm);

// -----------------------------------------------------------------------------
// 4. DNS-over-HTTPS (DoH, RFC 8484) & TLS SNI Pinning
// -----------------------------------------------------------------------------
typedef struct {
    char hostname[256];
    char resolved_ip[64];
    int ttl_sec;
    bool is_tls_pinned;
    uint8_t public_key_sha256[32];
} TsfiDohResolution;

bool tsfi_doh_encode_dns_query(const char *hostname, uint8_t *out_query_buf, size_t max_len, size_t *query_len_out);
bool tsfi_doh_parse_dns_response(const uint8_t *resp_buf, size_t resp_len, TsfiDohResolution *res_out);

#endif // TSFI_WEBGL_AUDIO_JS_DOH_H
