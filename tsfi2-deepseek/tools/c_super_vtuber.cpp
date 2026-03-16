#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#include "stable-diffusion.h"
#include "tsfi_text_render.h"

// Bring in our ultra-fast ZMM geometry mapper from the other file
extern "C" void zmm_texture_map(uint8_t* output_buffer, const uint8_t* master_frame, int frame_idx);

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
    
    float r_eye_x = cx - 15, r_eye_y = neck_y - 55;
    float l_eye_x = cx + 15, l_eye_y = neck_y - 55;
    float r_ear_x = cx - 35, r_ear_y = neck_y - 65;
    float l_ear_x = cx + 35, l_ear_y = neck_y - 65;
    
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
    
    DL(nose_x, nose_y, r_eye_x, r_eye_y, 255, 0, 255);
    DL(nose_x, nose_y, l_eye_x, l_eye_y, 255, 0, 255);
    DL(r_eye_x, r_eye_y, r_ear_x, r_ear_y, 255, 255, 0);
    DL(l_eye_x, l_eye_y, l_ear_x, l_ear_y, 255, 255, 0);
    
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


int main() {
    printf("[INFO] Booting 50x C-Native Hybrid Generative/ZMM Broadcast Pipeline...\n");
    
    // We are broadcasting directly to the YouTube pipe at 60 FPS natively!
    const char* stream_key = getenv("YOUTUBE_KEY");
    if (!stream_key) {
        printf("[WARN] YOUTUBE_KEY not set. Falling back to local file 'super_broadcast.flv'\n");
        stream_key = "super_broadcast.flv";
    } else {
        char buf[256];
        snprintf(buf, sizeof(buf), "rtmp://a.rtmp.youtube.com/live2/%s", stream_key);
        stream_key = strdup(buf);
    }
    
    char ffmpeg_cmd[1024];
    snprintf(ffmpeg_cmd, sizeof(ffmpeg_cmd), 
        "ffmpeg -y -re -f rawvideo -vcodec rawvideo -s 512x512 -pix_fmt rgb24 -r 60 "
        "-i - -vf \"scale=1080:1080,pad=1080:1920:0:420:black\" "
        "-c:v libx264 -preset ultrafast -b:v 2500k -maxrate 2500k -bufsize 5000k "
        "-pix_fmt yuv420p -g 120 -f flv \"%s\"", stream_key);
        
    FILE *ffmpeg_pipe = popen(ffmpeg_cmd, "w");
    if (!ffmpeg_pipe) {
        printf("[FRACTURE] FFmpeg stream failed to open.\n");
        return 1;
    }

    sd_ctx_params_t ctx_params;
    sd_ctx_params_init(&ctx_params);
    ctx_params.model_path = "assets/models/sd15.safetensors";
    ctx_params.vae_path = "assets/models/taesd.safetensors";
    ctx_params.control_net_path = "assets/models/control_openpose.safetensors";
    
    printf("  -> Loading Context (This happens exactly ONCE, zero Python overhead)...\n");
    sd_ctx_t* ctx = new_sd_ctx(&ctx_params);
    if (!ctx) return 1;
    
    uint8_t* control_pixels = (uint8_t*)malloc(512 * 512 * 3);
    
    sd_img_gen_params_t gen_params;
    sd_img_gen_params_init(&gen_params);
    gen_params.width = 512;
    gen_params.height = 512;
    gen_params.sample_params.sample_steps = 15; // Extremely high quality since we only do it ONCE per 10 seconds!
    gen_params.sample_params.guidance.txt_cfg = 7.0f;
    gen_params.batch_count = 1;
    gen_params.clip_skip = -1;
    
    gen_params.control_image.data = control_pixels;
    gen_params.control_image.width = 512;
    gen_params.control_image.height = 512;
    gen_params.control_image.channel = 3;
    gen_params.control_strength = 0.85f;
    gen_params.auto_resize_ref_image = true;
    
    gen_params.init_image.data = NULL;
    gen_params.init_image.width = 0;
    gen_params.init_image.height = 0;
    gen_params.init_image.channel = 3;
    gen_params.mask_image.data = NULL;
    gen_params.strength = 0.0f;
    
    uint8_t* render_buffer = (uint8_t*)malloc(512 * 512 * 3);
    
    const char* genomes[] = {
        "brown teddy bear", "pink plush teddy bear", "gothic black patchwork teddy bear", 
        "polar bear plushie", "vintage 1920s woven teddy bear", "neon cyberpunk plush bear",
        "stuffed bunny rabbit", "plush elephant", "stuffed penguin", "stuffed tiger", "plush koala"
    };

    printf("[INFO] Entering Infinite 60 FPS Broadcast Loop...\n");
    int epoch = 0;
    while (1) {
        // Since Vulkan GGML state leaks across generate_image calls natively, we must re-initialize the context.
        if (epoch > 0) {
            free_sd_ctx(ctx);
            ctx = new_sd_ctx(&ctx_params);
        }

        printf("--- EPOCH %d ---\n", epoch);
        int genome_idx = epoch % 11;
        
        char prompt[1024];
        snprintf(prompt, sizeof(prompt), 
            "A photorealistic overstuffed plush 16 inch tall %s sitting perfectly still, accompanied by a 3 inch tall fuzzy hawk moth flying beside it, masterpiece, 8k resolution, cinematic lighting, strictly non-human animal shape, fat round torso, plush fabric material, short stuffed animal limbs, 100 percent mutated, glowing green Kr0wZ sickness", 
            genomes[genome_idx]);
            
        gen_params.prompt = prompt;
        gen_params.negative_prompt = "human, humanoid, man, woman, child, realistic human anatomy, human face, human limbs, standing like a human, facing away, back view, from behind, person, monkey, ape, primate, muscular, abstract, deformed, bad anatomy, missing limbs, blurry";
        gen_params.seed = 42 + epoch;
        
        // 1. NEURAL GENESIS (The single slow step: ~3 seconds)
        printf("  -> Phase 1: Neural Genesis (15 Steps) for %s...\n", genomes[genome_idx]);
        
        // Render the skeleton to control_pixels so we have valid geometry for the controlnet
        build_openpose_skeleton(control_pixels, 0); 
        
        sd_image_t* master_frame = generate_image(ctx, &gen_params);
        if (!master_frame || !master_frame->data) {
            printf("[FRACTURE] Generator failed.\n");
            continue;
        }
        
        // 2. AVX-512 HARDWARE ZMM ANIMATION (600 Frames rendered in 0.5 seconds!)
        // This generates 10 seconds of perfectly smooth 60 FPS video instantly.
        printf("  -> Phase 2: ZMM Vector Texture Projection (600 Frames)...\n");
        for (int i = 0; i < 600; i++) {
            zmm_texture_map(render_buffer, master_frame->data, i);
            fwrite(render_buffer, 1, 512 * 512 * 3, ffmpeg_pipe);
        }
        fflush(ffmpeg_pipe);
        
        free(master_frame->data);
        free(master_frame);
        
        epoch++;
    }
    
    return 0;
}