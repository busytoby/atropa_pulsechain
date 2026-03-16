#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

// DeepSeek's specific procedural hash noise
float fract(float x) {
    return x - floorf(x);
}

float procedural_noise(float px, float py, float time_offset) {
    float dot_val = (px * 12.9898f) + (py * 78.233f) + time_offset;
    return fract(sinf(dot_val) * 43758.5453f);
}

int main() {
    int W = 1024;
    int H = 1536;
    int FPS = 60;
    int DURATION = 4; // 4 Seconds
    int TOTAL_FRAMES = FPS * DURATION; // 240 frames
    
    uint8_t *pixels = (uint8_t*)malloc(W * H * 3);
    if (!pixels) return 1;

    for (int frame = 0; frame < TOTAL_FRAMES; frame++) {
        memset(pixels, 0, W * H * 3);
        
        float time_offset = frame * 0.1f;
        float macro_evolution = (float)frame / (float)TOTAL_FRAMES;
        float pulse = sinf(macro_evolution * M_PI * 10.0f); 
        
        float head_radius = 200.0f + (pulse * 5.0f);
        float sickness_spread = 0.8f - (macro_evolution * 0.3f);
        
        // --- TIMELINE KINEMATICS ---
        float g_x = 0.0f;
        float g_y = 0.0f;
        float body_stretch_y = 0.25f;
        
        // Trigger at 2 seconds (Frame 120)
        int TRIGGER_FRAME = 120;
        
        if (frame > TRIGGER_FRAME) {
            int active_frames = frame - TRIGGER_FRAME;
            
            // The Spontaneous Stand (Frames 120 to 150)
            if (active_frames < 30) {
                float stand_progress = (float)active_frames / 30.0f;
                g_y -= (stand_progress * 0.15f);
                body_stretch_y += (stand_progress * 0.15f);
            } 
            // The Stage-Left Walk (Frames 150 to 240)
            else {
                int walk_frames = active_frames - 30;
                float walk_progress = (float)walk_frames;
                
                g_y -= 0.15f; 
                body_stretch_y = 0.40f; 
                
                g_x -= (walk_progress * 0.015f); 
                g_y += (sinf(walk_progress * 0.5f) * 0.02f); 
            }
        }
        
        for (int y = 0; y < H; y++) {
            int row_idx = y * W * 3;
            float py = (float)y / H;
            
            // Quick bounding box check (optimization)
            if (py < (0.1f + g_y) || py > (0.95f + g_y)) continue;
            
            for (int x = 0; x < W; x++) {
                float px = (float)x / W;
                
                if (px < (0.1f + g_x) || px > (0.9f + g_x)) continue;
                
                int idx = row_idx + (x * 3);
                
                // Apply global kinematic offsets to all spatial anchors
                float ax = px - g_x;
                float ay = py - g_y;
                
                float dist_eye1 = hypotf(ax - 0.5f, ay - 0.4f);
                float dist_eye2 = hypotf(ax - 0.5f, ay - 0.6f);
                
                float pulse_radius = 0.02f + (pulse * 0.002f);
                
                if (dist_eye1 < pulse_radius || dist_eye2 < (0.045f + pulse * 0.005f)) {
                    pixels[idx] = 0; pixels[idx+1] = 255; pixels[idx+2] = 0;
                }
                else if (hypotf(ax - 0.362f, ay - 0.25f) < 0.075f || hypotf(ax - 0.662f, ay - 0.25f) < 0.075f) {
                    pixels[idx] = 255; pixels[idx+1] = 0; pixels[idx+2] = 0;
                }
                else if (hypotf(ax - 0.5f, ay - 0.5f) < (0.2f + (pulse * 0.01f))) {
                    pixels[idx] = 255; pixels[idx+1] = 0; pixels[idx+2] = 0;
                }
                else if (hypotf(ax - 0.5f, (ay - 0.75f) / (body_stretch_y / 0.25f)) < 0.25f) {
                    // Elliptical stretch logic for the standing body
                    if (procedural_noise(px * 100.0f, py * 100.0f, time_offset) > sickness_spread) {
                        pixels[idx] = 84; pixels[idx+1] = 107; pixels[idx+2] = 45;
                    } else {
                        pixels[idx] = 91; pixels[idx+1] = 63; pixels[idx+2] = 51;
                    }
                }
            }
        }
        
        fwrite(pixels, 1, W * H * 3, stdout);
        
        if (frame % 1000 == 0) {
            fprintf(stderr, "[ACTIVE] Computed frame %d/%d (%.1f%%)\n", frame, TOTAL_FRAMES, ((float)frame/TOTAL_FRAMES)*100.0f);
        }
    }
    
    free(pixels);
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}
