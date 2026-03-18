#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <chrono>
#include <vector>
#include "stable-diffusion.h"

extern "C" {
#include "tsfi_controlnet_shm.h"
}

typedef enum {
    MODEL_SD15,
    MODEL_TURBO,
    MODEL_DREAMSHAPER
} TsfiModelProfile;

void sd_log_cb(enum sd_log_level_t level, const char* text, void* data) {
    (void)data;
    if (level >= SD_LOG_INFO) printf("[SD] %s", text);
}

int main(int argc, char** argv) {
    sd_set_log_callback(sd_log_cb, NULL);
    if (argc < 7) {
        printf("Usage: %s <prompt> <output.raw> <use_shm> <profile: sd15|turbo|dream> <steps> <method> [cfg] [strength]\n", argv[0]);
        return 1;
    }

    const char* prompt = argv[1];
    const char* output_path = argv[2];
    bool use_shm = atoi(argv[3]) != 0;
    const char* profile_str = argv[4];
    int steps = atoi(argv[5]);
    const char* method_str = argv[6];
    float cfg = (argc > 7) ? atof(argv[7]) : 1.0f;
    float strength = (argc > 8) ? atof(argv[8]) : 0.75f;

    TsfiModelProfile profile = MODEL_SD15;
    if (strcmp(profile_str, "turbo") == 0) profile = MODEL_TURBO;
    if (strcmp(profile_str, "dream") == 0) profile = MODEL_DREAMSHAPER;

    const TsfiControlNetMap *shm_depth = use_shm ? tsfi_cn_shm_attach(TSFI_CN_SHM_DEPTH) : NULL;
    const TsfiControlNetMap *shm_pose  = use_shm ? tsfi_cn_shm_attach(TSFI_CN_SHM_POSE) : NULL;
    const TsfiControlNetMap *shm_init  = use_shm ? tsfi_cn_shm_attach(TSFI_CN_SHM_INIT) : NULL;
    const TsfiDynamicGuidance *dgui    = tsfi_dgui_shm_attach();

    sd_ctx_params_t params;
    sd_ctx_params_init(&params);
    
    // Hardened path initialization
    params.clip_l_path = ""; params.clip_g_path = ""; params.t5xxl_path = "";
    params.clip_vision_path = ""; params.llm_path = ""; params.llm_vision_path = "";
    params.diffusion_model_path = ""; params.high_noise_diffusion_model_path = "";
    
    if (profile == MODEL_TURBO) {
        params.model_path = "assets/models/sd15.safetensors"; // Fallback to SD15 for ControlNet
        steps = 8; cfg = 1.5f; 
    } else if (profile == MODEL_DREAMSHAPER) {
        params.model_path = "assets/models/LCM_Dreamshaper_v7.safetensors";
    } else {
        params.model_path = "assets/models/sd15.safetensors";
    }

    params.vae_path = ""; params.taesd_path = ""; 
    params.control_net_path = "assets/models/control_depth.safetensors"; 
    params.control_net_path_2 = "assets/models/control_openpose.safetensors"; 
    
    // Explicitly disable TAESD preview
    params.tae_preview_only = false;
    params.wtype = SD_TYPE_F32;
    params.rng_type = CUDA_RNG;
    params.sampler_rng_type = CUDA_RNG;
    params.use_teddy_vae = false; params.n_threads = 16; 

    sd_ctx_t* ctx = new_sd_ctx(&params);
    if (!ctx) {
        printf("[FRACTURE] Failed to create Stable Diffusion Context (GPU/Model Error)\n");
        return 1;
    }

    sd_img_gen_params_t gen_params;
    memset(&gen_params, 0, sizeof(sd_img_gen_params_t));
    sd_img_gen_params_init(&gen_params);
    
    gen_params.prompt = prompt;
    gen_params.negative_prompt = "";
    gen_params.clip_skip = -1;
    gen_params.width = 1280;
    gen_params.height = 720;
    gen_params.batch_count = 1;
    gen_params.seed = -1;
    gen_params.strength = strength;
    gen_params.control_strength = 0.9f;
    gen_params.control_guidance_end = 1.0f;

    gen_params.sample_params.sample_steps = steps; 
    gen_params.sample_params.guidance.txt_cfg = cfg;
    gen_params.sample_params.sample_method = (strcmp(method_str, "euler_a") == 0) ? EULER_A_SAMPLE_METHOD : EULER_SAMPLE_METHOD;
    gen_params.sample_params.scheduler = (profile == MODEL_TURBO) ? DISCRETE_SCHEDULER : KARRAS_SCHEDULER;

    if (shm_depth) {
        gen_params.control_image.width = shm_depth->width; gen_params.control_image.height = shm_depth->height;
        gen_params.control_image.channel = 3; gen_params.control_image.data = (uint8_t*)shm_depth->data;
        gen_params.control_strength = dgui ? dgui->depth_strength : 0.8f;
    }

    if (shm_pose) {
        gen_params.control_image_2.width = shm_pose->width; gen_params.control_image_2.height = shm_pose->height;
        gen_params.control_image_2.channel = 3; gen_params.control_image_2.data = (uint8_t*)shm_pose->data;
        gen_params.control_strength_2 = dgui ? dgui->pose_strength : 0.6f;
    }

    if (shm_init) {
        gen_params.init_image.width = shm_init->width; gen_params.init_image.height = shm_init->height;
        gen_params.init_image.channel = 3; gen_params.init_image.data = (uint8_t*)shm_init->data;
        printf("[INFO] Temporal Feedback Enabled (Strength: %.2f)\n", gen_params.strength);
    }

    printf("[INFO] Generating 720p Masterpiece (%dx%d)...\n", 
           shm_depth ? (int)shm_depth->width : 1280, 
           shm_depth ? (int)shm_depth->height : 720);
    auto start = std::chrono::high_resolution_clock::now();
    
    sd_image_t* res = generate_image(ctx, &gen_params);
    if (!res) {
        printf("[FRACTURE] Image generation failed (UNet Error)\n");
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count();

    if (res && res->data) {
        FILE* f = fopen(output_path, "wb");
        if (f) { fwrite(res->data, 1, res->width * res->height * 3, f); fclose(f); }
        printf("[BENCH] Render complete in %.3fs\n", elapsed);
    }

    if (shm_depth) tsfi_cn_shm_detach(shm_depth);
    if (shm_pose)  tsfi_cn_shm_detach(shm_pose);
    if (shm_init)  tsfi_cn_shm_detach(shm_init);
    if (dgui)      tsfi_dgui_shm_detach(dgui);
    free_sd_ctx(ctx);
    return 0;
}
