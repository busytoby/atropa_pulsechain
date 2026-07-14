#include <stdio.h>
#include <stdlib.h>
#include "tsfi2_animation.h"

int main(void) {
    printf("Testing TSFI2 Animation Library in C...\n");
    
    // Create a 320x240 canvas
    tsfi2_canvas_t *canvas = tsfi2_canvas_create(320, 240);
    if (!canvas) {
        fprintf(stderr, "Failed to create canvas.\n");
        return 1;
    }
    
    // Clear canvas to premium dark background (15, 17, 26)
    tsfi2_canvas_clear(canvas, 15, 17, 26);
    
    // Draw glowing vector line
    tsfi2_draw_line_glow(canvas, 20.0f, 20.0f, 300.0f, 220.0f, 242, 201, 76);
    
    // Draw filled node circle in center
    tsfi2_draw_circle(canvas, 160, 120, 15, 235, 87, 87, 1);
    
    // Draw outlined node circle on top
    tsfi2_draw_circle(canvas, 160, 120, 18, 255, 255, 255, 0);
    
    // Save to PPM image format
    FILE *f = fopen("assets/test_animation_out.ppm", "wb");
    if (!f) {
        perror("Failed to open output PPM file");
        tsfi2_canvas_free(canvas);
        return 1;
    }
    
    fprintf(f, "P6\n%d %d\n255\n", canvas->width, canvas->height);
    fwrite(canvas->data, 1, canvas->width * canvas->height * 3, f);
    fclose(f);
    
    tsfi2_canvas_free(canvas);
    printf("✓ Successfully saved test frame to assets/test_animation_out.ppm\n");
    return 0;
}
