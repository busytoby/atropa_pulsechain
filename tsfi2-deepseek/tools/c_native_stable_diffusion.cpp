#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../assets/sd_cpp/thirdparty/stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../assets/sd_cpp/thirdparty/stb_image.h"

// Include the stable diffusion C API directly
#include "stable-diffusion.h"

int main(int argc, char **argv) {
    printf("[INFO] Booting Native C-Kernel Stable Diffusion Matrix...\n");
    
    // 1. Initialize Context
    sd_ctx_params_t ctx_params;
    sd_ctx_params_init(&ctx_params);
    ctx_params.model_path = "assets/models/sd15.safetensors";
    ctx_params.vae_path = "assets/models/taesd.safetensors";
    ctx_params.control_net_path = "assets/models/control_openpose.safetensors";
    
    printf("  -> Loading Context (This happens exactly ONCE, no Python overhead)...\n");
    sd_ctx_t* ctx = new_sd_ctx(&ctx_params);
    if (!ctx) {
        printf("[FRACTURE] Failed to initialize SD Context.\n");
        return 1;
    }
    
    // 2. Prepare Generation Parameters
    sd_img_gen_params_t gen_params;
    sd_img_gen_params_init(&gen_params);
    gen_params.width = 512;
    gen_params.height = 512;
    gen_params.sample_params.sample_steps = 4; // High speed iteration
    gen_params.sample_params.guidance.txt_cfg = 1.2f; // Correct CFG binding
    gen_params.seed = 42; 
    
    // Bind to the exact prompt strings that guarantee 16-inch scale
    gen_params.prompt = "A photorealistic overstuffed plush 16 inch tall brown teddy bear sitting still like a companion pet, masterpiece, 8k resolution, cinematic lighting, highly detailed anatomy";
    gen_params.negative_prompt = "muscular, human-like, abstract, deformed, bad anatomy, missing limbs, blurry";

    printf("[INFO] Generation Loop Engaged.\n");
    
    // Load pre-generated skeleton for proof of concept
    int w, h, c;
    uint8_t *control_pixels = stbi_load("assets/mpx_skel.png", &w, &h, &c, 3);
    if (control_pixels) {
        gen_params.control_image.data = control_pixels;
        gen_params.control_image.width = w;
        gen_params.control_image.height = h;
        gen_params.control_image.channel = 3;
        gen_params.control_strength = 1.0f;
    } else {
        printf("[WARN] ControlNet skeleton missing. Continuing without it.\n");
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // 3. GENERATE NATIVELY
    sd_image_t* result = generate_image(ctx, &gen_params);
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    if (result && result->data) {
        printf("[SUCCESS] Rendered frame in %.2f seconds (C-Native).\n", elapsed);
        stbi_write_png("native_sd_test.png", result->width, result->height, 3, result->data, result->width * 3);
    } else {
        printf("[FRACTURE] Generation failed.\n");
    }

    if (control_pixels) stbi_image_free(control_pixels);
    
    printf("[INFO] Teardown...\n");
    free_sd_ctx(ctx);
    
    return 0;
}
