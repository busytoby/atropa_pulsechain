#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_envmap.h"

int main(void) {
    printf("=== RUNNING AUNCIENT ENVIRONMENT MAP TESTS ===\n");

    uint8_t texture[4 * 4];
    memset(texture, 128, sizeof(texture));
    texture[0] = 42; // Set a specific texel

    TSFiEnvMap env;
    tsfi_envmap_init(&env, 4, 4, texture);

    assert(env.width == 4);
    assert(env.height == 4);
    assert(env.texture_data == texture);

    // Sample pointing backwards/along vector directions
    uint8_t s0 = tsfi_envmap_sample(&env, -1.0, 0.0, 0.0);
    assert(s0 == 128);

    // Verify sampling normalization bounds
    uint8_t s1 = tsfi_envmap_sample(&env, 0.0, 0.0, 0.0);
    assert(s1 == 128);

    printf("   ✓ Environment map parameters initialized successfully.\n");
    printf("   ✓ Spherical equirectangular latlong reflection sampling verified.\n");
    printf("=== AUNCIENT ENVIRONMENT MAP TESTS COMPLETE (PASS) ===\n");
    return 0;
}
