#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

// Geometric rendering loop simulating PM4 structures for the Teddy Bear
void render_sick_teddy_bear(uint32_t *px, int w, int h, int frame) {
    memset(px, 0, w * h * 4); // Clear screen to void (black)
    
    // Base Colors
    uint32_t brown = 0xFF5C4033;
    uint32_t dark_brown = 0xFF3E2723;
    uint32_t sick_green = 0xFF556B2F;
    uint32_t eye_color = (frame % 60 < 30) ? 0xFF00FF00 : 0xFF005500; // Kr0wZ Green pulse
    
    // Head (16 inch canvas = massive geometry)
    for(int y=200; y<600; y++) {
        for(int x=300; x<700; x++) {
            // Rough circle equation
            int dx = x - 500; int dy = y - 400;
            if (dx*dx + dy*dy < 40000) px[y*w + x] = brown;
        }
    }
    
    // Ears
    for(int y=100; y<300; y++) {
        for(int x=250; x<400; x++) {
            int dx = x - 325; int dy = y - 200;
            if (dx*dx + dy*dy < 8000) px[y*w + x] = dark_brown;
        }
        for(int x=600; x<750; x++) {
            int dx = x - 675; int dy = y - 200;
            if (dx*dx + dy*dy < 8000) px[y*w + x] = dark_brown;
        }
    }
    
    // Eyes (Pulsing Telemetry)
    for(int y=300; y<380; y++) {
        for(int x=400; x<480; x++) {
            int dx = x - 440; int dy = y - 340;
            if (dx*dx + dy*dy < 1000) px[y*w + x] = eye_color; // Left Eye
        }
        for(int x=520; x<600; x++) {
            int dx = x - 560; int dy = y - 340;
            if (dx*dx + dy*dy < 1000) px[y*w + x] = 0xFFFF0000; // Right Eye (Fractured Red)
        }
    }
    
    // The Sick Body (Corrupted PM4 Geometry)
    for(int y=550; y<1200; y++) {
        for(int x=250; x<750; x++) {
            int dx = x - 500; int dy = y - 875;
            if (dx*dx*0.6 + dy*dy < 100000) {
                // Sickness noise overlay
                if ((x * y * frame) % 100 < 5) px[y*w + x] = sick_green;
                else px[y*w + x] = brown;
            }
        }
    }
}

int main() {
    printf("=== TSFi Teddy Bear Rendering Benchmark ===\n");
    int w = 1024, h = 1536;
    uint32_t *px = malloc(w * h * 4);
    if (!px) return 1;

    int frames = 1000;
    for(int i=0; i<frames; i++) {
        render_sick_teddy_bear(px, w, h, i);
        if (i % 10 == 0) { printf("Rendered frame %d\n", i); fflush(stdout); }
    }

    printf("[SUCCESS] Rendered %d frames locally.\n", frames);
    free(px);
    return 0;
}
