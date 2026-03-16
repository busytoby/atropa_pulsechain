#include "tsfi_liang_barsky.h"
#include <stdio.h>
#include <math.h>

/**
 * @brief Liang-Barsky CLIPTEST (Parametric Inequality)
 */
static inline bool cliptest(float p, float q, float *t_near, float *t_far) {
    if (p < 0.0f) {
        float t = q / p;
        if (t > *t_far) return false;
        if (t > *t_near) *t_near = t;
    } else if (p > 0.0f) {
        float t = q / p;
        if (t < *t_near) return false;
        if (t < *t_far) *t_far = t;
    } else if (q < 0.0f) {
        return false;
    }
    return true;
}

/**
 * @brief Clips a ZMM latent vector against the Liang-Barsky Arena.
 * Returns a __mmask16 where 1 indicates the lane survived and 0 indicates culling.
 */
__mmask16 tsfi_liang_barsky_clip_zmm(__m512 *latent_vec, const TsfiClippingVolume *vol) {
    if (!latent_vec || !vol) return 0;

    float data[16];
    _mm512_storeu_ps(data, *latent_vec);
    __mmask16 result_mask = 0;

    // Process each 4-float group (X, Y, Z, W)
    for (int i = 0; i < 16; i += 4) {
        // PPO HARDENING: Cull NaN/Inf immediately
        if (!isfinite(data[i]) || !isfinite(data[i+1]) || !isfinite(data[i+2]) || !isfinite(data[i+3])) {
            data[i] = 0; data[i+1] = 0; data[i+2] = 0; data[i+3] = 0;
            continue;
        }

        float t_near = 0.0f;
        float t_far = 1.0f;
        
        bool ok = true;
        ok &= cliptest(-1.0f, data[i]   - vol->x_min, &t_near, &t_far);
        ok &= cliptest( 1.0f, vol->x_max - data[i],   &t_near, &t_far);
        ok &= cliptest(-1.0f, data[i+1] - vol->y_min, &t_near, &t_far);
        ok &= cliptest( 1.0f, vol->y_max - data[i+1], &t_near, &t_far);
        ok &= cliptest(-1.0f, data[i+2] - vol->z_min, &t_near, &t_far);
        ok &= cliptest( 1.0f, vol->z_max - data[i+2], &t_near, &t_far);
        ok &= cliptest(-1.0f, data[i+3] - vol->w_min, &t_near, &t_far);
        ok &= cliptest( 1.0f, vol->w_max - data[i+3], &t_near, &t_far);

        if (ok) {
            // Lane group survived
            result_mask |= (0xF << i);
        } else {
            // CULL group
            data[i] = 0; data[i+1] = 0; data[i+2] = 0; data[i+3] = 0;
        }
    }

    *latent_vec = _mm512_loadu_ps(data);
    return result_mask;
}
