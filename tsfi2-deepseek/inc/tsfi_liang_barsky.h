#ifndef TSFI_LIANG_BARSKY_H
#define TSFI_LIANG_BARSKY_H

#include <immintrin.h>
#include <stdbool.h>

/**
 * @brief Liang-Barsky Arena: Parametric Latent Clipping
 * 
 * Logic: Treats the 4D VAE latent space as a clipping volume.
 * Latent trajectories (Seek paths) are clipped against the 
 * accredited vessel's structural window.
 */

typedef struct {
    float x_min, x_max;
    float y_min, y_max;
    float z_min, z_max;
    float w_min, w_max; // 4th Latent Dimension
} TsfiClippingVolume;

// Clips a latent vector (ZMM) against the volume using Liang-Barsky inequalities.
// Returns a bitmask (1 = kept, 0 = culled/clipped).
__mmask16 tsfi_liang_barsky_clip_zmm(__m512 *latent_vec, const TsfiClippingVolume *vol);

#endif // TSFI_LIANG_BARSKY_H
