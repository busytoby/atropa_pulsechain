#include "tsfi_media_decode.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <stdlib.h>
#include <string.h>

struct TsfiMediaDecoder {
    AVFormatContext *format_ctx;
    AVCodecContext *codec_ctx;
    const AVCodec *codec;
    AVFrame *frame;
    AVPacket *packet;
    int video_stream_idx;
};

static void yuv420p_to_rgba(int w, int h, 
                            const uint8_t *y_plane, int y_stride,
                            const uint8_t *u_plane, int u_stride,
                            const uint8_t *v_plane, int v_stride,
                            uint8_t *rgba_out) {
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int y_val = y_plane[y * y_stride + x];
            int u_val = u_plane[(y / 2) * u_stride + (x / 2)] - 128;
            int v_val = v_plane[(y / 2) * v_stride + (x / 2)] - 128;

            int r = y_val + (1.370705f * v_val);
            int g = y_val - (0.337633f * u_val) - (0.698001f * v_val);
            int b = y_val + (1.732446f * u_val);

            if (r < 0) { r = 0; }
            if (r > 255) { r = 255; }
            if (g < 0) { g = 0; }
            if (g > 255) { g = 255; }
            if (b < 0) { b = 0; }
            if (b > 255) { b = 255; }

            int pixel_idx = (y * w + x) * 4;
            rgba_out[pixel_idx] = r;
            rgba_out[pixel_idx+1] = g;
            rgba_out[pixel_idx+2] = b;
            rgba_out[pixel_idx+3] = 255;
        }
    }
}

TsfiMediaDecoder* tsfi_media_decoder_create(const char *filepath) {
    TsfiMediaDecoder *dec = (TsfiMediaDecoder*)malloc(sizeof(TsfiMediaDecoder));
    if (!dec) return NULL;
    memset(dec, 0, sizeof(*dec));

    dec->video_stream_idx = -1;

    // Open video file
    if (avformat_open_input(&dec->format_ctx, filepath, NULL, NULL) < 0) {
        free(dec);
        return NULL;
    }

    // Retrieve stream info
    if (avformat_find_stream_info(dec->format_ctx, NULL) < 0) {
        avformat_close_input(&dec->format_ctx);
        free(dec);
        return NULL;
    }

    // Find the first video stream
    for (unsigned int i = 0; i < dec->format_ctx->nb_streams; i++) {
        if (dec->format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            dec->video_stream_idx = i;
            break;
        }
    }

    if (dec->video_stream_idx == -1) {
        avformat_close_input(&dec->format_ctx);
        free(dec);
        return NULL;
    }

    // Get codec parameters and find decoder
    AVCodecParameters *codec_par = dec->format_ctx->streams[dec->video_stream_idx]->codecpar;
    dec->codec = avcodec_find_decoder(codec_par->codec_id);
    if (!dec->codec) {
        avformat_close_input(&dec->format_ctx);
        free(dec);
        return NULL;
    }

    // Allocate codec context
    dec->codec_ctx = avcodec_alloc_context3(dec->codec);
    if (!dec->codec_ctx) {
        avformat_close_input(&dec->format_ctx);
        free(dec);
        return NULL;
    }

    if (avcodec_parameters_to_context(dec->codec_ctx, codec_par) < 0) {
        avcodec_free_context(&dec->codec_ctx);
        avformat_close_input(&dec->format_ctx);
        free(dec);
        return NULL;
    }

    // Open codec
    if (avcodec_open2(dec->codec_ctx, dec->codec, NULL) < 0) {
        avcodec_free_context(&dec->codec_ctx);
        avformat_close_input(&dec->format_ctx);
        free(dec);
        return NULL;
    }

    // Allocate video frame and packet
    dec->frame = av_frame_alloc();
    dec->packet = av_packet_alloc();
    if (!dec->frame || !dec->packet) {
        tsfi_media_decoder_destroy(dec);
        return NULL;
    }

    return dec;
}

void tsfi_media_decoder_destroy(TsfiMediaDecoder *dec) {
    if (!dec) return;
    if (dec->frame) av_frame_free(&dec->frame);
    if (dec->packet) av_packet_free(&dec->packet);
    if (dec->codec_ctx) avcodec_free_context(&dec->codec_ctx);
    if (dec->format_ctx) avformat_close_input(&dec->format_ctx);
    free(dec);
}

bool tsfi_media_decoder_next_frame(TsfiMediaDecoder *dec, int *width, int *height, uint8_t *rgba_buffer) {
    if (!dec) return false;

    while (av_read_frame(dec->format_ctx, dec->packet) >= 0) {
        if (dec->packet->stream_index == dec->video_stream_idx) {
            // Supply raw packet data to the decoder
            if (avcodec_send_packet(dec->codec_ctx, dec->packet) >= 0) {
                // Read decoded frame
                int ret = avcodec_receive_frame(dec->codec_ctx, dec->frame);
                if (ret == 0) {
                    *width = dec->frame->width;
                    *height = dec->frame->height;
                    
                    // Convert YUV420p format to RGBA format
                    yuv420p_to_rgba(dec->frame->width, dec->frame->height,
                                    dec->frame->data[0], dec->frame->linesize[0],
                                    dec->frame->data[1], dec->frame->linesize[1],
                                    dec->frame->data[2], dec->frame->linesize[2],
                                    rgba_buffer);
                    
                    av_packet_unref(dec->packet);
                    return true;
                }
            }
        }
        av_packet_unref(dec->packet);
    }
    return false;
}
