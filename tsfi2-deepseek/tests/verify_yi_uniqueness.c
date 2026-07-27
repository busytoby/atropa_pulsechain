#include "tsfi_vision.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define WIDTH 300
#define HEIGHT 300

// Simple drawing helper to simulate rendering the YI shape in C
void draw_yi_wireframe(uint32_t *pixels, uint64_t manifold, uint64_t monopole, uint64_t dynamo, uint64_t ring) {
    memset(pixels, 0, WIDTH * HEIGHT * sizeof(uint32_t));
    
    // Draw 300 lines of the parameterized Lissajous shape
    int num_points = 300;
    for (int i = 0; i < num_points; i++) {
        double theta = (i * 2.0 * 3.1415926535) / num_points;
        
        double w_arg = theta * (manifold % 100);
        double x_arg = theta * (monopole % 100);
        double y_arg = theta * (dynamo % 100);
        double z_arg = theta * (ring % 100);
        
        double qw = cos(w_arg);
        double qx = sin(x_arg);
        double qy = sin(y_arg);
        double qz = sin(z_arg);
        
        double w_warp = 1.0 + 0.25 * qw;
        double x = qx * 80.0 * w_warp;
        double y = qy * 80.0 * w_warp;
        double z = qz * 80.0 * w_warp;
        
        // Simple isometric projection
        double px = (x - y) * cos(3.14159 / 6.0) + WIDTH / 2.0;
        double py = (x + y) * sin(3.14159 / 6.0) - z + HEIGHT / 2.0;
        
        int ix = (int)px;
        int iy = (int)py;
        if (ix >= 0 && ix < WIDTH && iy >= 0 && iy < HEIGHT) {
            pixels[iy * WIDTH + ix] = 0xFFFFFFFF; // Draw white pixel
        }
    }
}

int main() {
    printf("=========================================================\n");
    printf("Auncient TSFi Vision: YI Quaternion Uniqueness Verification\n");
    printf("=========================================================\n");

    // Allocated buffers for the 4 YIs
    uint32_t *yi1 = malloc(WIDTH * HEIGHT * sizeof(uint32_t));
    uint32_t *yi2 = malloc(WIDTH * HEIGHT * sizeof(uint32_t));
    uint32_t *yi3 = malloc(WIDTH * HEIGHT * sizeof(uint32_t));
    uint32_t *yi4 = malloc(WIDTH * HEIGHT * sizeof(uint32_t));

    // YI 1 (953467954114363)
    draw_yi_wireframe(yi1, 1631052951685751ULL, 563316215773011ULL, 231949031122517ULL, 432408158703388ULL);
    // YI 2 (1000000000000037)
    draw_yi_wireframe(yi2, 846763449578335ULL, 211355463586047ULL, 977997212409288ULL, 16566186896647ULL);
    // YI 3 (1000000000000091)
    draw_yi_wireframe(yi3, 1216489409367429ULL, 897479868497322ULL, 1271563842464607ULL, 199418747894736ULL);
    // YI 4 (1000000000000159)
    draw_yi_wireframe(yi4, 2142565876433067ULL, 406291974248150ULL, 794728974323097ULL, 750727654864305ULL);

    // Calculate cross-similarities using tsfi_vision_calculate_similarity
    float sim12 = tsfi_vision_calculate_similarity(yi1, yi2, WIDTH, HEIGHT);
    float sim13 = tsfi_vision_calculate_similarity(yi1, yi3, WIDTH, HEIGHT);
    float sim14 = tsfi_vision_calculate_similarity(yi1, yi4, WIDTH, HEIGHT);
    float sim23 = tsfi_vision_calculate_similarity(yi2, yi3, WIDTH, HEIGHT);
    float sim24 = tsfi_vision_calculate_similarity(yi2, yi4, WIDTH, HEIGHT);
    float sim34 = tsfi_vision_calculate_similarity(yi3, yi4, WIDTH, HEIGHT);

    printf("[VISION] Similarity Matrix (Auncient TSFi Vision Algorithm):\n");
    printf("  YI 1 vs YI 2 Similarity: %.5f\n", sim12);
    printf("  YI 1 vs YI 3 Similarity: %.5f\n", sim13);
    printf("  YI 1 vs YI 4 Similarity: %.5f\n", sim14);
    printf("  YI 2 vs YI 3 Similarity: %.5f\n", sim23);
    printf("  YI 2 vs YI 4 Similarity: %.5f\n", sim24);
    printf("  YI 3 vs YI 4 Similarity: %.5f\n", sim34);

    // Assert that no two unique YI quaternions are identical (which would give similarity = 1.0)
    if (sim12 > 0.98f || sim13 > 0.98f || sim14 > 0.98f || sim23 > 0.98f || sim24 > 0.98f || sim34 > 0.98f) {
        printf("[FAIL] Visual uniqueness check failed! Identical or near-identical shapes detected.\n");
        exit(1);
    }

    printf("[SUCCESS] Visual uniqueness verified: similarity is below the threshold (< 0.98).\n");

    printf("=========================================================\n");

    free(yi1);
    free(yi2);
    free(yi3);
    free(yi4);
    return 0;
}
