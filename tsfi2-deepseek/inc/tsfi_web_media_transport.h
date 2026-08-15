#ifndef TSFI_WEB_MEDIA_TRANSPORT_H
#define TSFI_WEB_MEDIA_TRANSPORT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// -----------------------------------------------------------------------------
// 1. HTTP/3 & QUIC UDP Packet Engine (RFC 9000 / RFC 9204)
// -----------------------------------------------------------------------------
typedef struct {
    uint8_t header_form; // 1=Long Header (Initial/Handshake), 0=Short Header (1-RTT Data)
    uint32_t version;
    uint8_t dcil;        // Destination Connection ID Length
    uint8_t dest_conn_id[20];
    uint8_t scil;        // Source Connection ID Length
    uint8_t src_conn_id[20];
    uint64_t packet_number;
    uint32_t payload_len;
    const uint8_t *payload;
} TsfiQuicPacket;

bool tsfi_quic_encode_initial_packet(
    const uint8_t *dest_cid, uint8_t dcil,
    const uint8_t *src_cid, uint8_t scil,
    uint64_t packet_num,
    const uint8_t *payload, size_t payload_len,
    uint8_t *out_buf, size_t max_len, size_t *out_len
);

bool tsfi_quic_decode_packet(
    const uint8_t *in_buf, size_t in_len,
    TsfiQuicPacket *packet_out
);

// -----------------------------------------------------------------------------
// 2. HTTP Gzip & Deflate Stream Decompressor (RFC 1951 / 1952)
// -----------------------------------------------------------------------------
typedef struct {
    uint8_t id1, id2; // 0x1F, 0x8B
    uint8_t cm;       // Compression Method (8 = Deflate)
    uint8_t flg;
    uint32_t mtime;
    uint8_t xfl;
    uint8_t os;
} TsfiGzipHeader;

bool tsfi_gzip_validate_header(const uint8_t *data, size_t len, TsfiGzipHeader *hdr_out, size_t *hdr_len_out);
size_t tsfi_deflate_decompress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_max_len);

// -----------------------------------------------------------------------------
// 3. PNG (RFC 2083) Image Stream Decoder & ARGB Scanout Rasterizer
// -----------------------------------------------------------------------------
typedef struct {
    uint32_t width;
    uint32_t height;
    uint8_t bit_depth;
    uint8_t color_type; // 2=RGB, 6=RGBA
    uint8_t compression;
    uint8_t filter;
    uint8_t interlace;
} TsfiPngHeader;

bool tsfi_png_parse_header(const uint8_t *png_data, size_t len, TsfiPngHeader *hdr_out);
bool tsfi_png_decode_to_argb(const uint8_t *png_data, size_t len, uint32_t *out_pixels, int out_w, int out_h);

// -----------------------------------------------------------------------------
// 4. CSS Transitions & Animation Keyframe Engine
// -----------------------------------------------------------------------------
typedef enum {
    EASING_LINEAR = 0,
    EASING_EASE_IN,
    EASING_EASE_OUT,
    EASING_EASE_IN_OUT
} TsfiEasingType;

typedef struct {
    float start_val;
    float end_val;
    double start_time_sec;
    double duration_sec;
    TsfiEasingType easing;
    bool is_active;
} TsfiCssAnimation;

void tsfi_animation_init(TsfiCssAnimation *anim, float start_val, float end_val, double duration_sec, TsfiEasingType easing, double current_time);
float tsfi_animation_sample(const TsfiCssAnimation *anim, double current_time);
uint32_t tsfi_color_interpolate_argb(uint32_t c1, uint32_t c2, float t);

#endif // TSFI_WEB_MEDIA_TRANSPORT_H
