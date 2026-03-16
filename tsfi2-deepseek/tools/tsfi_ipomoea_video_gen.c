#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <immintrin.h>
#include "tsfi_vae_firmware.h"
#include "tsfi_nand_trap_firmware.h"
#include "tsfi_vision.h"
#include "lau_memory.h"

#define FRAMES 120
#define W 512
#define H 512

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <output_dir>\n", argv[0]);
        return 1;
    }

    const char *out_dir = argv[1];
    mkdir(out_dir, 0777);

    NandTrapState nand;
    LauVaeFirmware_State vae;
    
    float *density_field = (float*)lau_malloc(W * H * sizeof(float));
    uint32_t *pixels = (uint32_t*)lau_malloc(W * H * sizeof(uint32_t));

    for (int f = 0; f < FRAMES; f++) {
        float t = (float)f / (float)(FRAMES - 1);
        
        memset(&nand, 0, sizeof(NandTrapState));
        tsfi_nand_ipomoea_lifecycle(&nand, t);
        
        tsfi_vae_firmware_boot(&vae, &nand);
        
        // 1. Decode Fibers to High-Res Density Field (VAE Upsampler)
        tsfi_vae_decode_to_field(&vae, density_field, W, H);

        // 2. Render Shading using TSFi Flower Thunk
        WaveStream density_stream = { .data = density_field, .count = W * H, .stride = 4 };
        
        // White Ipomoea (Ipomoea alba) profile
        TSFiFlowerRenderUniforms render_u = { 
            .light_x = 0.5f, .light_y = 0.5f, .light_z = 1.0f, 
            .sss_power = 0.6f, .fresnel_power = 0.4f, 
            .secret_resonance = 1.0f + 0.1f * tsfi_nand_trap_cornu_c(t),
            .core_color = {0.9f, 0.9f, 0.7f},  // Creamy throat
            .body_color = {1.0f, 1.0f, 1.0f},  // White petals
            .outer_color = {0.95f, 0.95f, 1.0f} // Slight blue tint at rim
        };
        
        tsfi_flower_render_shading(&density_stream, pixels, W, H, &render_u);
        
        // 3. Post-process (Bloom)
        tsfi_bloom_thunk(pixels, W, H, 0.8f, 0.3f);

        // 4. Export Frame
        char path[256];
        snprintf(path, sizeof(path), "%s/frame_%03d.ppm", out_dir, f);
        FILE *of = fopen(path, "wb");
        if (of) {
            fprintf(of, "P6\n%d %d\n255\n", W, H);
            for (int i = 0; i < W * H; i++) {
                uint32_t p = pixels[i];
                uint8_t rgb[3] = { (uint8_t)(p >> 16), (uint8_t)(p >> 8), (uint8_t)p };
                fwrite(rgb, 1, 3, of);
            }
            fclose(of);
        }
        
        printf("[GEN] High-Res Frame %d/120 (t=%.2f) exported\n", f, t);
    }

    lau_free(density_field);
    lau_free(pixels);
    printf("[SUCCESS] High-resolution lifecycle frames generated in %s\n", out_dir);
    return 0;
}
