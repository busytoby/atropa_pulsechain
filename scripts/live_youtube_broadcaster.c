/* SPDX-License-Identifier: GPL-2.0 */
/*
 * High-Performance Smooth YouTube Live Broadcaster & Dynamic Telemetry Overlay
 * Streams the solid earthenware clay teddy bear with continuous studio AAC 192k audio.
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
    const char *stream_key = (argc > 1) ? argv[1] : "h19h-xuw9-7tx6-m8sw-dmht";
    char rtmp_url[512];
    snprintf(rtmp_url, sizeof(rtmp_url), "rtmp://a.rtmp.youtube.com/live2/%s", stream_key);

    printf("=============================================================\n");
    printf("LAUNCHING SOLID CLAY TEDDY BEAR LIVE BROADCASTER TO YOUTUBE  \n");
    printf("=============================================================\n");
    printf("Source Video  : clayscape_bear_solid_demo.mp4 (720p 3D SDF Shaded)\n");
    printf("Audio Quality : Studio Stereo AAC 192k @ 44.1 kHz           \n");
    printf("RTMP Endpoint : %s\n", rtmp_url);
    printf("=============================================================\n");

    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
             "ffmpeg -y -re -stream_loop -1 -i clayscape_bear_solid_demo.mp4 "
             "-c:v libx264 -preset veryfast -b:v 3000k -maxrate 3000k -bufsize 6000k "
             "-pix_fmt yuv420p -g 60 -keyint_min 60 -sc_threshold 0 "
             "-c:a aac -b:a 192k -ar 44100 "
             "-flvflags no_duration_filesize -f flv \"%s\"",
             rtmp_url);

    while (1) {
        printf("   -> Starting FFmpeg solid clay live stream broadcast...\n");
        fflush(stdout);
        int ret = system(cmd);
        printf("   -> Stream disconnected (code %d), auto-reconnecting in 2 seconds...\n", ret);
        fflush(stdout);
        sleep(2);
    }
    return 0;
}
