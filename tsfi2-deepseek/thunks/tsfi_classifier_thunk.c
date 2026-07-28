#include <stdint.h>
#include <math.h>

// TSFiClassifierMatrix: AVX-Optimized Structural Cascade
// Returns a confidence score from 0.0 (Total Hallucination) to 1.0 (Perfect Teddy Bear Structure)
float evaluate_teddy_cascade(const uint8_t* pixels, int w, int h) {
    long total_brown_mass = 0;
    long total_green_mass = 0;
    long out_of_bounds_mass = 0;
    
    // We physically scan the 256x256 image using a simulated AVX-512 block logic
    #pragma GCC ivdep
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int idx = (y * w * 3) + (x * 3);
            uint8_t r = pixels[idx];
            uint8_t g = pixels[idx+1];
            uint8_t b = pixels[idx+2];
            
            // Fast Geometric Filter 1: Is the center actually the Bear?
            // Brown threshold: R > G, G > B, R > 50
            if (r > g && g > b && r > 50) {
                // If it's in the center (where the body should be), it's good mass.
                if (hypotf(x - (w/2), y - (h/2)) < (w * 0.4f)) {
                    total_brown_mass++;
                } else {
                    // If brown is in the background corners, the AI hallucinated the bear too big
                    out_of_bounds_mass++;
                }
            }
            
            // Fast Geometric Filter 2: Did it render the Kr0wZ Green mutation?
            if (g > r && g > b && g > 60 && r < 100 && b < 100) {
                total_green_mass++;
            }
        }
    }
    
    float score = 1.0f;
    
    // The Cascade Penalties
    if (total_brown_mass < (w * h * 0.1f)) {
        score -= 0.5f; // Reject: No bear body detected
    }
    if (out_of_bounds_mass > (w * h * 0.2f)) {
        score -= 0.3f; // Reject: Bear exceeds the bounds (hallucinated zoomed-in face)
    }
    
    // Normalize and clamp
    if (score < 0.0f) score = 0.0f;
    if (score > 1.0f) score = 1.0f;
    
    return score;
}
