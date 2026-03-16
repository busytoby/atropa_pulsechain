#include "tsfi_c_math.h"
#include "tsfi_nand_trap_firmware.h"
#include <math.h>
#include <immintrin.h>
#include <string.h>

/**
 * TSFi Refined Ipomoea Lifecycle Choreography
 * 
 * Stages:
 * - 0.0 - 0.1: SEED (Tight mass at base)
 * - 0.1 - 0.2: SPROUT (Initial vertical line)
 * - 0.2 - 0.3: STALK (Growth and thickening)
 * - 0.3 - 0.4: TYPE (Lateral leaf/sepal emergence)
 * - 0.4 - 0.6: BUD (Bulbous apex formation)
 * - 0.6 - 0.8: BLOOM (5-lobed White Ipomoea expansion)
 * - 0.8 - 1.0: SENESCENCE (Wilting and collapse)
 */
void tsfi_nand_ipomoea_lifecycle(NandTrapState *nand, float t) {
    if (!nand) return;

    for (int i = 0; i < GRANS; i++) {
        float phase = (float)i / (float)GRANS * 2.0f * ((float)TSFI_SECRET_CORE);
        float x = 0, y = 0, z = 0, w = 0;

        if (t < 0.1f) {
            // 1. SEED: Small sphere at (0, -0.4, 0)
            float nt = t / 0.1f;
            float r = 0.05f * nt;
            x = r * cosf(phase * 7.0f);
            z = r * sinf(phase * 3.0f);
            y = -0.4f + r * cosf(phase);
            w = 0.9f;
        } else if (t < 0.2f) {
            // 2. SPROUT: Line from -0.4 to -0.2
            float nt = (t - 0.1f) / 0.1f;
            x = 0.01f * cosf(phase * 10.0f);
            z = 0.01f * sinf(phase * 10.0f);
            y = -0.4f + (0.2f * nt) * ((float)i / GRANS);
            w = 0.8f;
        } else if (t < 0.3f) {
            // 3. STALK: Growth to y=0.1
            float nt = (t - 0.2f) / 0.1f;
            x = 0.02f * cosf(phase * 5.0f);
            z = 0.02f * sinf(phase * 5.0f);
            y = -0.4f + (0.5f * nt + 0.2f) * ((float)i / GRANS);
            w = 1.0f;
        } else if (t < 0.4f) {
            // 4. TYPE: Sepals/Leaves emerge at stalk base
            float nt = (t - 0.3f) / 0.1f;
            y = -0.4f + 0.7f * ((float)i / GRANS);
            if (i < GRANS / 2) { // Stalk
                x = 0.03f * cosf(phase); z = 0.03f * sinf(phase);
            } else { // Leaf
                float leaf_t = (float)(i - GRANS/2) / (GRANS/2);
                float leaf_r = 0.15f * nt * leaf_t;
                x = leaf_r * cosf(phase); z = leaf_r * sinf(phase);
                y = -0.2f + 0.1f * leaf_t;
            }
            w = 1.0f;
        } else if (t < 0.6f) {
            // 5. BUD: Bulb forms at top
            float nt = (t - 0.4f) / 0.2f;
            if (i < GRANS * 0.7f) { // Stalk
                y = -0.4f + 0.7f * ((float)i / (GRANS * 0.7f));
                x = 0.03f * cosf(phase); z = 0.03f * sinf(phase);
            } else { // Bud bulb
                float bud_t = (float)(i - GRANS*0.7f) / (GRANS*0.3f);
                float bud_r = 0.05f + 0.1f * nt * sinf(bud_t * ((float)TSFI_SECRET_CORE));
                x = bud_r * cosf(phase); z = bud_r * sinf(phase);
                y = 0.3f + 0.15f * bud_t;
            }
            w = 1.0f;
        } else if (t < 0.8f) {
            // 6. BLOOM: Funnel expansion
            float nt = (t - 0.6f) / 0.2f;
            float angle = phase;
            float radius = 0.05f + 0.45f * nt * (1.0f + 0.25f * cosf(5.0f * angle));
            x = radius * cosf(angle);
            z = radius * sinf(angle);
            y = 0.3f + 0.5f * nt * (radius / 0.5f);
            w = 1.0f;
        } else {
            // 7. SENESCENCE: Collapse and browning
            float nt = (t - 0.8f) / 0.2f;
            float angle = phase;
            float radius = 0.5f * (1.0f - nt * 0.8f);
            x = radius * cosf(angle + nt * 4.0f);
            z = radius * sinf(angle + nt * 4.0f);
            y = 0.3f + 0.5f * (1.0f - nt) - 0.2f * nt;
            w = 1.0f - nt;
        }

        nand->fibers[i].x = x;
        nand->fibers[i].y = y;
        nand->fibers[i].z = z;
        nand->fibers[i].weight = w;
    }
    nand->epoch = t;
}
