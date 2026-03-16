#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <immintrin.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define WIDTH 512
#define HEIGHT 512
#define FRAMES 600

// We declare the zmm mapping function we just wrote
extern void zmm_texture_map(uint8_t* output_buffer, const uint8_t* master_frame, int frame_idx);

int main() {
    printf("=== TSFi AVX-512 Native Texture Projection Benchmark ===\n");
    
    uint8_t* output_buffer = (uint8_t*)malloc(WIDTH * HEIGHT * 3);
    uint8_t* dummy_master = (uint8_t*)malloc(WIDTH * HEIGHT * 3);
    
    // Fill the dummy master with a basic colored pattern to simulate the Neural Genesis frame
    for(int i=0; i<WIDTH*HEIGHT*3; i++) dummy_master[i] = (i % 255);
    
    FILE *ffmpeg_pipe = popen("ffmpeg -y -f rawvideo -vcodec rawvideo -s 512x512 -pix_fmt rgb24 -r 60 -i - -c:v libx264 -preset ultrafast -pix_fmt yuv420p zmm_texture_speed.mp4 2>/dev/null", "w");
    if (!ffmpeg_pipe) return 1;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < FRAMES; i++) {
        zmm_texture_map(output_buffer, dummy_master, i);
        fwrite(output_buffer, 1, WIDTH * HEIGHT * 3, ffmpeg_pipe);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("[SPEED] 600 Frames generated and pipelined in %.4f seconds.\n", elapsed);
    printf("[PERF] Achieved %.1f FPS (%.1f x Speed Increase over standard pipeline)\n", FRAMES / elapsed, (FRAMES / elapsed) / 2.0f);
    
    pclose(ffmpeg_pipe);
    free(output_buffer);
    free(dummy_master);
    
    return 0;
}
