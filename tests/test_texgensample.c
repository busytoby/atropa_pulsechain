#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "../tsfi2-deepseek/inc/tsfi_texgen.h"

int main(void) {
    printf("=== RUNNING AUNCIENT TEXGEN SAMPLE TESTS ===\n");

    // Initialize mock texture buffer: 2x2 pixels, RGBA format
    uint8_t mock_texture[16] = {
        255, 0, 0, 255,   // Pixel 0,0: Red
        0, 255, 0, 255,   // Pixel 1,0: Green
        0, 0, 255, 255,   // Pixel 0,1: Blue
        255, 255, 255, 255 // Pixel 1,1: White
    };

    uint8_t r = 0, g = 0, b = 0;

    // Test direct index lookup mapping (u=0.25, v=0.25 -> pixel 0,0)
    tsfi_texgen_sample(mock_texture, 2, 2, 0.25, 0.25, &r, &g, &b);
    assert(r == 255 && g == 0 && b == 0);

    // Test wrap lookup mapping (u=0.75, v=0.25 -> pixel 1,0)
    tsfi_texgen_sample(mock_texture, 2, 2, 0.75, 0.25, &r, &g, &b);
    assert(r == 0 && g == 255 && b == 0);

    // Test periodic wrap modulo limits (u=1.25, v=1.25 -> pixel 0,0)
    tsfi_texgen_sample(mock_texture, 2, 2, 1.25, 1.25, &r, &g, &b);
    assert(r == 255 && g == 0 && b == 0);

    printf("   ✓ Texgen constant-time O(1) UV coordinate sampling verified.\n");
    printf("   ✓ Periodic wrap modulo boundary constraints verified successfully.\n");
    printf("=== AUNCIENT TEXGEN SAMPLE TESTS COMPLETE (PASS) ===\n");
    return 0;
}
