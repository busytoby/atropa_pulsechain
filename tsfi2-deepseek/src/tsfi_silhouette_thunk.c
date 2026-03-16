#include <stdint.h>
#include <math.h>

// TSFi Dynamic Silhouette Extractor
// Scans a photorealistic 256x256 image and extracts a binary map of the physical entity.
// Also generates the affine-transformed sickness masks for the generative pipeline.

static inline float fractf(float x) { return x - floorf(x); }
static inline float procedural_noise(float px, float py, float time_offset) {
    float dot_val = (px * 12.9898f) + (py * 78.233f) + time_offset;
    return fractf(sinf(dot_val) * 43758.5453f);
}

// Pass 1: Extract the organic silhouette from the photorealistic Anchor Bear
void extract_silhouette(const uint8_t* in_pixels, uint8_t* out_silhouette, int w, int h) {
    // A primitive color-variance threshold (Sobel-lite) to isolate the subject from the dark background
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int idx = (y * w * 3) + (x * 3);
            int sid = (y * w) + x;
            
            uint8_t r = in_pixels[idx];
            uint8_t g = in_pixels[idx+1];
            uint8_t b = in_pixels[idx+2];
            
            // If the pixel is reasonably bright and carries color (not pure dark grey/black floor)
            if (r > 30 || g > 30 || b > 30) {
                // If it leans warm (brown fur) or is generally bright, mark as entity mass
                if (r > b || (r > 80 && g > 80)) {
                    out_silhouette[sid] = 1; // Entity Mass
                    continue;
                }
            }
            out_silhouette[sid] = 0; // Void / Background
        }
    }
}

// Pass 2: Apply Sickness and Affine Kinematics to the Silhouette
void generate_dynamic_mask(const uint8_t* in_pixels, const uint8_t* in_silhouette, uint8_t* out_mask, int w, int h, int epoch) {
    float sickness = epoch > 50 ? 100.0f : ((float)epoch / 50.0f) * 100.0f;
    float time_offset = epoch * 0.1f;
    
    // Kinematic Affine Transforms
    float g_x = 0.0f;
    float scale_y = 1.0f;
    
    if (epoch > 50 && epoch <= 80) {
        // Stand up
        scale_y = 1.0f + (((epoch - 50) / 30.0f) * 0.5f); 
    } else if (epoch > 80) {
        scale_y = 1.5f;
        g_x = -((epoch - 80) * 0.02f);
    }
    
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int idx = (y * w * 3) + (x * 3);
            
            // Reverse map the affine transform to find the source pixel in the original silhouette
            // If scale_y is 1.5, we want to squish the lookup so the bear stretches up.
            // We anchor the stretch to the bottom of the image (y = h)
            float src_y = h - ((h - y) / scale_y);
            float src_x = (float)x - (g_x * w);
            
            int ix = (int)src_x;
            int iy = (int)src_y;
            
            // Bounds check
            if (ix >= 0 && ix < w && iy >= 0 && iy < h) {
                int sid = (iy * w) + ix;
                int src_idx = (iy * w * 3) + (ix * 3);
                
                if (in_silhouette[sid] == 1) {
                    // Inside the organic bear mask!
                    // Copy the original photorealistic pixel
                    out_mask[idx] = in_pixels[src_idx];
                    out_mask[idx+1] = in_pixels[src_idx+1];
                    out_mask[idx+2] = in_pixels[src_idx+2];
                    
                    // Apply organic sickness mutation specifically to the fur
                    float px = (float)x / w;
                    float py = (float)y / h;
                    float noise = procedural_noise(px * 100.0f, py * 100.0f, time_offset);
                    
                    if ((noise * 100.0f) < sickness) {
                        out_mask[idx] = 51; // Rotting Kr0wZ Green
                        out_mask[idx+1] = 65;
                        out_mask[idx+2] = 0;
                    }
                } else {
                    // Background void (Fill with neutral grey to allow VAE expansion)
                    out_mask[idx] = 180; out_mask[idx+1] = 180; out_mask[idx+2] = 180;
                }
            } else {
                // Out of transformed bounds
                out_mask[idx] = 180; out_mask[idx+1] = 180; out_mask[idx+2] = 180;
            }
        }
    }
}
