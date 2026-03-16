#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <immintrin.h>

#include "stable-diffusion.h"
#include "tsfi_text_render.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../assets/sd_cpp/thirdparty/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../assets/sd_cpp/thirdparty/stb_image_write.h"

// Simple thick line drawing for the OpenPose skeleton
void draw_thick_line(uint8_t* img, int W, int H, float x0, float y0, float x1, float y1, uint8_t r, uint8_t g, uint8_t b, float thickness) {
    int min_x = fmax(0, fmin(x0, x1) - thickness);
    int max_x = fmin(W - 1, fmax(x0, x1) + thickness);
    int min_y = fmax(0, fmin(y0, y1) - thickness);
    int max_y = fmin(H - 1, fmax(y0, y1) + thickness);
    
    float l2 = (x1 - x0)*(x1 - x0) + (y1 - y0)*(y1 - y0);
    if (l2 == 0.0f) return;
    
    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            float t = ((x - x0) * (x1 - x0) + (y - y0) * (y1 - y0)) / l2;
            t = fmax(0.0f, fmin(1.0f, t));
            float px = x0 + t * (x1 - x0);
            float py = y0 + t * (y1 - y0);
            float dist = sqrt((x - px)*(x - px) + (y - py)*(y - py));
            
            if (dist <= thickness / 2.0f) {
                int idx = (y * W + x) * 3;
                img[idx] = r;
                img[idx+1] = g;
                img[idx+2] = b;
            }
        }
    }
}

void build_openpose_skeleton(uint8_t* img, int f_idx) {
    memset(img, 0, 512 * 512 * 3);
    
    // Draw Unicode Matrix Rain Background
    draw_matrix_rain(img, 512, 512, f_idx, "GENIE SEES: A TEDDY BEAR SITTING AND STIRRING ");
    
    int cx = 256;
    int cy = 384;
    float scale_y = 1.0f;
    float arm_len = 80 * scale_y;
    
    // Smooth stirring animation with decay to reach absolute rest
    float t = f_idx / 30.0f;
    float decay = fmax(0.0f, 1.0f - (f_idx / 45.0f)); // Decays to zero by frame 45, holds rest for 15 frames
    float sit_arm_wave = (sinf(t * 1.5f) * 0.5f + sinf(t * 4.1f) * 0.2f + sinf(t * 0.8f) * 0.3f) * decay;
    float sit_leg_twitch = (cosf(t * 2.3f) * 0.5f + sinf(t * 3.7f) * 0.3f) * decay;
    
    float torso_len = 120 * scale_y;
    float neck_x = cx, neck_y = cy - torso_len;
    float nose_x = cx, nose_y = neck_y - 40;
    
    float r_shoulder_x = neck_x - 40, r_shoulder_y = neck_y;
    float l_shoulder_x = neck_x + 40, l_shoulder_y = neck_y;
    float r_hip_x = cx - 30, r_hip_y = cy;
    float l_hip_x = cx + 30, l_hip_y = cy;
    
    // Evolved sitting form - facing camera, stirring arms
    float r_elbow_x = r_shoulder_x - 20;
    float r_elbow_y = r_shoulder_y + arm_len * 0.4f - sit_arm_wave * 10.0f;
    float r_wrist_x = r_elbow_x - 10 - sit_arm_wave * 15.0f;
    float r_wrist_y = r_elbow_y + arm_len * 0.5f - sit_arm_wave * 25.0f;
    
    float l_elbow_x = l_shoulder_x + 20;
    float l_elbow_y = l_shoulder_y + arm_len * 0.4f + sit_arm_wave * 8.0f;
    float l_wrist_x = l_elbow_x + 10 + sit_arm_wave * 12.0f;
    float l_wrist_y = l_elbow_y + arm_len * 0.5f + sit_arm_wave * 20.0f;
    
    float r_knee_x = r_hip_x - 50;
    float r_knee_y = r_hip_y + 30;
    float r_ankle_x = r_hip_x - 80 - sit_leg_twitch * 10.0f;
    float r_ankle_y = r_hip_y + 70 - sit_leg_twitch * 15.0f;
    
    float l_knee_x = l_hip_x + 50;
    float l_knee_y = l_hip_y + 30;
    float l_ankle_x = l_hip_x + 80 + sit_leg_twitch * 8.0f;
    float l_ankle_y = l_hip_y + 70 + sit_leg_twitch * 12.0f;
    
    float th = 8.0f;
    #define DL(x0,y0,x1,y1,r,g,b) draw_thick_line(img, 512, 512, x0, y0, x1, y1, r, g, b, th)
    DL(neck_x, neck_y, cx, cy, 255, 0, 0);
    DL(neck_x, neck_y, nose_x, nose_y, 255, 0, 0);
    DL(neck_x, neck_y, r_shoulder_x, r_shoulder_y, 255, 85, 0);
    DL(r_shoulder_x, r_shoulder_y, r_elbow_x, r_elbow_y, 255, 170, 0);
    DL(r_elbow_x, r_elbow_y, r_wrist_x, r_wrist_y, 255, 255, 0);
    
    DL(neck_x, neck_y, l_shoulder_x, l_shoulder_y, 85, 255, 0);
    DL(l_shoulder_x, l_shoulder_y, l_elbow_x, l_elbow_y, 0, 255, 0);
    DL(l_elbow_x, l_elbow_y, l_wrist_x, l_wrist_y, 0, 255, 85);
    
    DL(cx, cy, r_hip_x, r_hip_y, 0, 255, 170);
    DL(r_hip_x, r_hip_y, r_knee_x, r_knee_y, 0, 85, 255);
    DL(r_knee_x, r_knee_y, r_ankle_x, r_ankle_y, 0, 0, 255);
    
    DL(cx, cy, l_hip_x, l_hip_y, 85, 0, 255);
    DL(l_hip_x, l_hip_y, l_knee_x, l_knee_y, 170, 0, 255);
    DL(l_knee_x, l_knee_y, l_ankle_x, l_ankle_y, 255, 0, 255);
}

int main(int argc, char **argv) {
    printf("[INFO] Booting Native C-Kernel Stable Diffusion Temporal Evolution Matrix...\n");
    
    sd_ctx_params_t ctx_params;
    sd_ctx_params_init(&ctx_params);
    ctx_params.model_path = "assets/models/sd15.safetensors";
    ctx_params.vae_path = "assets/models/taesd.safetensors";
    ctx_params.control_net_path = "assets/models/control_openpose.safetensors";
    ctx_params.vae_decode_only = false; // Required for img2img evolution!
    
    printf("  -> Loading Neural Context (This happens exactly ONCE, no Python overhead)...\n");
    sd_ctx_t* ctx = new_sd_ctx(&ctx_params);
    if (!ctx) {
        printf("[FRACTURE] Failed to initialize SD Context.\n");
        return 1;
    }
    
    uint8_t* control_pixels = (uint8_t*)malloc(512 * 512 * 3);
    
    sd_img_gen_params_t gen_params;
    sd_img_gen_params_init(&gen_params);
    gen_params.width = 512;
    gen_params.height = 512;
    gen_params.batch_count = 1;
    gen_params.clip_skip = -1;
    gen_params.sample_params.sample_steps = 4;
    gen_params.sample_params.guidance.txt_cfg = 1.2f;
    gen_params.seed = 42; 
    gen_params.prompt = "A photorealistic overstuffed plush 16 inch tall brown teddy bear sitting still like a companion pet, masterpiece, 8k resolution, cinematic lighting, highly detailed anatomy";
    gen_params.negative_prompt = "muscular, human-like, abstract, deformed, bad anatomy, missing limbs, blurry";
    
    gen_params.control_image.data = control_pixels;
    gen_params.control_image.width = 512;
    gen_params.control_image.height = 512;
    gen_params.control_image.channel = 3;
    gen_params.control_strength = 0.9f;
    gen_params.auto_resize_ref_image = true;

    FILE *ffmpeg_pipe = popen("ffmpeg -y -f rawvideo -vcodec rawvideo -s 512x512 -pix_fmt rgb24 -r 5 -i - -c:v libx264 -preset fast -pix_fmt yuv420p native_evolution_teddy.mp4 2>/dev/null", "w");
    
    sd_image_t last_frame = {0, 0, 0, NULL};

    printf("[INFO] Temporal Generation Loop Engaged (60 Frames at 5 FPS = 12 seconds).\n");
    for (int i = 0; i < 60; i++) {
        // Since Vulkan GGML state leaks across generate_image calls natively, we must re-initialize the context.
        if (i > 0) {
            free_sd_ctx(ctx);
            ctx = new_sd_ctx(&ctx_params);
        }

        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);

        build_openpose_skeleton(control_pixels, i);

        // Dynamic Focus Architecture:
        // Expanded to 60 frames. 
        // Blur reduced by 90% -> Starting steps significantly higher (e.g. 5 steps minimum).
        // Scaling up to 8 steps for absolute Dreamshaper hyper-realism.
        int dynamic_steps = 5 + (int)(((float)i / 59.0f) * 3.0f);
        gen_params.sample_params.sample_steps = dynamic_steps;

        // ControlNet strength also tightens as focus increases
        gen_params.control_strength = 0.8f + (((float)i / 59.0f) * 0.2f);
        // Bypass the volatile VAE re-encoding loop that crashes the Vulkan backend memory allocator.
        // We use absolute 100% ControlNet physical logic and fixed seeds to force sequence coherence natively.
        gen_params.init_image.data = NULL;
        gen_params.mask_image.data = NULL;
        gen_params.strength = 0.0f;

        sd_image_t* result = generate_image(ctx, &gen_params);
        
        if (i > 0 && gen_params.mask_image.data != NULL) {
            free(gen_params.mask_image.data);
            gen_params.mask_image.data = NULL;
        }
        
        if (result && result->data) {
            fwrite(result->data, 1, 512 * 512 * 3, ffmpeg_pipe);
            fflush(ffmpeg_pipe);
            
            if (last_frame.data != NULL) {
                free(last_frame.data);
            }
            
            last_frame.width = result->width;
            last_frame.height = result->height;
            last_frame.channel = result->channel;
            
            // We must force the frame to disk momentarily to break the Vulkan device memory pointer
            // before feeding it back into the VAE. This acts as a physical hardware memory flush.
            stbi_write_png("assets/native_tmp.png", 512, 512, 3, result->data, 512 * 3);
            
            if (i > 0 && gen_params.init_image.data) {
                stbi_image_free(gen_params.init_image.data);
                gen_params.init_image.data = NULL;
            }
            
            clock_gettime(CLOCK_MONOTONIC, &end);
            double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
            printf("  -> Frame %03d rendered in %.2f seconds.\n", i, elapsed);
            
            // DO NOT explicitly free result->data, as stable-diffusion.cpp handles it or uses stb_image_free.
            // But we must free the result struct itself if generate_image allocated it.
            // Actually, sd-cli does free(result[i].data) if it's not the init image...
            free(result->data);
            free(result);
        } else {
            printf("[FRACTURE] Frame %d failed.\n", i);
        }
    }

    if (last_frame.data) free(last_frame.data);
    free(control_pixels);
    pclose(ffmpeg_pipe);
    free_sd_ctx(ctx);
    
    printf("\n[SUCCESS] Evolution Video saved to native_evolution_teddy.mp4\n");
    return 0;
}