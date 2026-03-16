#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>

// DeepSeek's specific procedural hash noise
float fract(float x) {
    return x - floorf(x);
}

float procedural_noise(float px, float py, float time_offset) {
    float dot_val = (px * 12.9898f) + (py * 78.233f) + time_offset;
    return fract(sinf(dot_val) * 43758.5453f);
}

// True Branching Shadow Physics (Bypassing AI Hallucination)
float compute_bifurcated_shadow(float x, float y) {
    float dist_left = hypotf(x - 0.400f, y - 0.780f);
    float dist_right = hypotf(x - 0.624f, y - 0.780f);
    float k = 50.0f; 
    float res = expf(-k * dist_left) + expf(-k * dist_right);
    return -logf(res) / k;
}

// The exact DNA structural layout established by DeepSeek
#pragma pack(push, 1)
typedef struct {
    float g_x;
    float g_y;
    float body_y_stretch;
    float pulse;
    float dynamic_sick;
    float light_intensity;
    uint8_t fur_r;
    uint8_t fur_g;
    uint8_t fur_b;
    uint8_t eye_r;
    uint8_t eye_g;
    uint8_t eye_b;
    uint8_t eye_count;
} TsfiDNAFrame;
#pragma pack(pop)

int main() {
    int W = 1024;
    int H = 1536;
    int TOTAL_FRAMES = 720; // 12 seconds @ 60 FPS
    
    // 1. Physically map the AOT DNA Vessel
    int fd = open("assets/atropa.dna", O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "[FRACTURE] atropa.dna vessel missing or corrupted.\\n");
        return 1;
    }
    
    struct stat sb;
    fstat(fd, &sb);
    uint8_t *dna_map = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    
    // Verify Magic Header
    if (memcmp(dna_map, "TSFI", 4) != 0) {
        fprintf(stderr, "[FRACTURE] Invalid DNA Header.\\n");
        return 1;
    }
    
    uint32_t compiled_frames = *(uint32_t*)(dna_map + 4);
    TsfiDNAFrame *dna_frames = (TsfiDNAFrame*)(dna_map + 8);
    
    if (compiled_frames < TOTAL_FRAMES) {
        TOTAL_FRAMES = compiled_frames;
    }
    
    uint8_t *pixels = (uint8_t*)malloc(W * H * 3);
    if (!pixels) return 1;

    // 2. High-Speed execution loop
    for (int f = 0; f < TOTAL_FRAMES; f++) {
        memset(pixels, 0, W * H * 3);
        
        TsfiDNAFrame dna = dna_frames[f];
        float time_offset = f * 0.1f;
        
        // Deep Volumetric Constraints (Phase 0 and 3.5)
        float head_radius = 200.0f + (dna.pulse * 5.0f);
        float pupil_radius = 20.0f + (fabsf(dna.pulse) * 15.0f);
        
        for (int y = 100; y < 1400; y++) {
            int row_idx = y * W * 3;
            float py = (float)y / H;
            
            // Bounding box optimization for 60 FPS speed
            if (py < (0.1f + dna.g_y) || py > (0.95f + dna.g_y)) continue;
            
            for (int x = 200; x < 800; x++) {
                float px = (float)x / W;
                int idx = row_idx + (x * 3);
                
                float ax = px - dna.g_x;
                float ay = py - dna.g_y;
                
                // VLM Interrogation Extracted Offsets
                float dist_eye1 = hypotf(ax - 0.46f, ay - 0.4f);
                float dist_eye2 = hypotf(ax - 0.54f, ay - 0.4f); 
                float pulse_radius = 0.02f + (dna.pulse * 0.002f);
                
                // 1. The Branching Shadow Physics
                float shadow_dist = compute_bifurcated_shadow(ax, ay);
                if (shadow_dist < 0.08f) {
                    pixels[idx] = 19; pixels[idx+1] = 19; pixels[idx+2] = 19; // Shadow floor
                }
                
                // 2. The Eyes (With Sickness Suppressor logic)
                if (dist_eye1 < pulse_radius) {
                    pixels[idx] = dna.eye_r; pixels[idx+1] = dna.eye_g; pixels[idx+2] = dna.eye_b;
                }
                else if (dna.eye_count == 2 && dist_eye2 < pulse_radius) {
                    pixels[idx] = dna.eye_r; pixels[idx+1] = dna.eye_g; pixels[idx+2] = dna.eye_b;
                }
                else if (dna.eye_count == 1 && dist_eye2 < pulse_radius + 0.01f) {
                    // Suppressed eye cavity (Fracture/Shadow)
                    pixels[idx] = 30; pixels[idx+1] = 0; pixels[idx+2] = 0; 
                }
                // 3. The Ears
                else if (hypotf(ax - 0.35f, ay - 0.25f) < 0.075f || hypotf(ax - 0.65f, ay - 0.25f) < 0.075f) {
                    pixels[idx] = dna.fur_r; pixels[idx+1] = dna.fur_g; pixels[idx+2] = dna.fur_b;
                }
                // 4. The Cranium
                else if (hypotf(ax - 0.5f, ay - 0.5f) < (0.2f + (dna.pulse * 0.01f))) {
                    // Specular Rim Light overlay
                    if (hypotf(ax - 0.45f, ay - 0.45f) < 0.08f) {
                        pixels[idx] = 160 + (92 * dna.light_intensity); // Add highlight
                        pixels[idx+1] = dna.fur_g; pixels[idx+2] = dna.fur_b;
                    } else {
                        pixels[idx] = dna.fur_r; pixels[idx+1] = dna.fur_g; pixels[idx+2] = dna.fur_b;
                    }
                }
                // 5. The Sick Body
                else if (hypotf(ax - 0.5f, (ay - 0.75f) / (dna.body_y_stretch / 0.25f)) < 0.25f) {
                    if (procedural_noise(px * 100.0f, py * 100.0f, time_offset) > dna.dynamic_sick) {
                        pixels[idx] = 51; pixels[idx+1] = 65; pixels[idx+2] = 0; // Rotting green
                    } else {
                        pixels[idx] = dna.fur_r; pixels[idx+1] = dna.fur_g; pixels[idx+2] = dna.fur_b;
                    }
                }
            }
        }
        
        // Push raw geometry to stdout for FFmpeg to ingest instantly
        fwrite(pixels, 1, W * H * 3, stdout);
        
        if (f % 60 == 0) {
            fprintf(stderr, "[ACTIVE] Decoded DNA frame %d/%d\\n", f, TOTAL_FRAMES);
        }
    }
    
    free(pixels);
    munmap(dna_map, sb.st_size);
    close(fd);
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}
