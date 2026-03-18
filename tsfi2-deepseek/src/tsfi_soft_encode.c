#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dlfcn.h>
#include "x264.h"

// Dynamic libx264 handle
static void *x264_lib = NULL;
static x264_t* (*p_x264_encoder_open)(x264_param_t*) = NULL;
static int (*p_x264_encoder_headers)(x264_t*, x264_nal_t**, int*) = NULL;
static int (*p_x264_param_default_preset)(x264_param_t*, const char*, const char*) = NULL;
static int (*p_x264_param_apply_profile)(x264_param_t*, const char*) = NULL;
static int (*p_x264_picture_alloc)(x264_picture_t*, int, int, int) = NULL;
static int (*p_x264_encoder_encode)(x264_t*, x264_nal_t**, int*, x264_picture_t*, x264_picture_t*) = NULL;

static x264_t *encoder = NULL;
static x264_picture_t pic_in, pic_out;
static int g_w = 0, g_h = 0;
static uint8_t g_sps[256];
static size_t g_sps_len = 0;
static uint8_t g_pps[256];
static size_t g_pps_len = 0;

int tsfi_soft_encode_init(int w, int h, uint8_t **out_sps, size_t *out_sps_len, uint8_t **out_pps, size_t *out_pps_len) {
    x264_lib = dlopen("/usr/lib/x86_64-linux-gnu/libx264.so.165", RTLD_LAZY);
    if (!x264_lib) { printf("[FRACTURE] Failed to load libx264.so.165\n"); return -1; }

    p_x264_encoder_open = dlsym(x264_lib, "x264_encoder_open_165");
    p_x264_encoder_headers = dlsym(x264_lib, "x264_encoder_headers");
    p_x264_param_default_preset = dlsym(x264_lib, "x264_param_default_preset");
    p_x264_param_apply_profile = dlsym(x264_lib, "x264_param_apply_profile");
    p_x264_picture_alloc = dlsym(x264_lib, "x264_picture_alloc");
    p_x264_encoder_encode = dlsym(x264_lib, "x264_encoder_encode");

    if (!p_x264_encoder_open || !p_x264_encoder_encode) return -1;

    x264_param_t param;
    p_x264_param_default_preset(&param, "veryfast", "zerolatency");
    param.i_bitdepth = 8;
    param.i_csp = X264_CSP_I420;
    param.i_width = w;
    param.i_height = h;
    param.b_vfr_input = 0;
    param.b_repeat_headers = 0;
    param.b_annexb = 0; // We need AVCC format for RTMP (4-byte length prefix), not Annex-B (00 00 00 01)
    
    // QoS Target: 2.5 Mbps
    param.rc.i_rc_method = X264_RC_ABR;
    param.rc.i_bitrate = 2500;
    param.rc.i_vbv_max_bitrate = 2500;
    param.rc.i_vbv_buffer_size = 5000;
    
    param.i_fps_num = 24;
    param.i_fps_den = 1;
    param.i_keyint_max = 48; // GOP 2s

    p_x264_param_apply_profile(&param, "main");

    encoder = p_x264_encoder_open(&param);
    if (!encoder) return -1;

    p_x264_picture_alloc(&pic_in, param.i_csp, param.i_width, param.i_height);

    x264_nal_t *nals; int i_nals;
    p_x264_encoder_headers(encoder, &nals, &i_nals);
    for (int i = 0; i < i_nals; i++) {
        if (nals[i].i_type == NAL_SPS) {
            memcpy(g_sps, nals[i].p_payload + 4, nals[i].i_payload - 4); // Strip prefix if annexb was on, but we set it to 0 so it's a 4-byte size
            g_sps_len = nals[i].i_payload - 4;
        } else if (nals[i].i_type == NAL_PPS) {
            memcpy(g_pps, nals[i].p_payload + 4, nals[i].i_payload - 4);
            g_pps_len = nals[i].i_payload - 4;
        }
    }

    *out_sps = g_sps; *out_sps_len = g_sps_len;
    *out_pps = g_pps; *out_pps_len = g_pps_len;
    g_w = w; g_h = h;
    return 0;
}

static void rgb_to_yuv420p(uint8_t *rgb, uint8_t *y_plane, uint8_t *u_plane, uint8_t *v_plane, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int i = (y * width + x) * 3;
            int r = rgb[i], g = rgb[i+1], b = rgb[i+2];
            y_plane[y * width + x] = (uint8_t)((( 66 * r + 129 * g +  25 * b + 128) >> 8) + 16);
            if (y % 2 == 0 && x % 2 == 0) {
                int uv_idx = (y / 2) * (width / 2) + (x / 2);
                u_plane[uv_idx] = (uint8_t)(((-38 * r -  74 * g + 112 * b + 128) >> 8) + 128);
                v_plane[uv_idx] = (uint8_t)(((112 * r -  94 * g -  18 * b + 128) >> 8) + 128);
            }
        }
    }
}

static uint8_t g_out_buf[2 * 1024 * 1024];

int tsfi_soft_encode_frame(uint8_t *rgb, int w, int h, uint8_t **out_nal, size_t *out_len, uint32_t frame_idx) {
    if (!encoder || w != g_w || h != g_h) return -1;
    
    rgb_to_yuv420p(rgb, pic_in.img.plane[0], pic_in.img.plane[1], pic_in.img.plane[2], w, h);
    pic_in.i_pts = frame_idx;

    x264_nal_t *nals;
    int i_nals;
    int frame_size = p_x264_encoder_encode(encoder, &nals, &i_nals, &pic_in, &pic_out);
    
    if (frame_size > 0) {
        // x264 with b_annexb=0 outputs NALUs prefixed by 4-byte size in big-endian, exactly what RTMP expects.
        // We just need to find the video NALU (skip SEI if present).
        size_t offset = 0;
        *out_len = 0;
        for (int i = 0; i < i_nals; i++) {
            if (nals[i].i_type == NAL_SLICE || nals[i].i_type == NAL_SLICE_IDR) {
                memcpy(g_out_buf + offset, nals[i].p_payload, nals[i].i_payload);
                offset += nals[i].i_payload;
            }
        }
        *out_nal = g_out_buf;
        *out_len = offset;
        return 0;
    }
    return -1;
}
