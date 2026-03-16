#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <sys/stat.h>
#include <immintrin.h>

#include "tsfi_vae_firmware.h"
#include "tsfi_nand_trap_firmware.h"
#include "tsfi_vision.h"
#include "lau_memory.h"
#include "tsfi_safetensors_cache.h"
#include "tsfi_svdag.h"
#include "tsfi_pbr.h"
#include "tsfi_c_math.h"

#define FRAMES 600
#define W 512
#define H 512

void apply_taesd_modulation(uint32_t *pixels, int w, int h, TsfiSafetensorsAsset *taesd, float t) {
    if (!taesd || !pixels) return;
    uint8_t *raw_data = (uint8_t*)taesd->data;
    size_t data_size = taesd->size;
    float intensity = 0.05f * sinf(t * (float)TSFI_SECRET_CORE);
    for (int i = 0; i < w * h; i++) {
        uint32_t offset = (i * 17) % (data_size - 4);
        float noise = (float)raw_data[offset] / 255.0f;
        uint32_t p = pixels[i];
        uint8_t r = (p >> 16) & 0xFF, g = (p >> 8) & 0xFF, b = p & 0xFF;
        r = (uint8_t)fminf(255.0f, (float)r + noise * intensity * 255.0f);
        g = (uint8_t)fminf(255.0f, (float)g + noise * intensity * 255.0f);
        b = (uint8_t)fminf(255.0f, (float)b + noise * intensity * 255.0f);
        pixels[i] = (0xFF << 24) | (r << 16) | (g << 8) | b;
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <output_dir>\n", argv[0]);
        return 1;
    }

    const char *out_dir = argv[1];
    mkdir(out_dir, 0777);

    tsfi_pbr_init_luts();
    TSFiHelmholtzSVDAG *dag_flower = tsfi_svdag_create(128*128*128);
    TSFiHelmholtzSVDAG *dag_bear = tsfi_svdag_create(128*128*128);

    NandTrapState neural_bear_nand;
    memset(&neural_bear_nand, 0, sizeof(neural_bear_nand));
    FILE *bf = fopen("assets/neural_bear.dna", "rb");
    if (bf) {
        size_t n = fread(neural_bear_nand.fibers, 1, 1024 * 16, bf);
        fclose(bf);
        if (n == 1024 * 16) printf("[INIT] Neural Bear DNA loaded\n");
    }

    TsfiSafetensorsAsset *taesd = tsfi_safetensors_cache_attach("assets/models/taesd.safetensors");
    float sov_melanin = 0.5f, sov_roughness = 0.4f, sov_irid = 0.5f;
    if (taesd) {
        tsfi_sovereign_hydrate_from_taesd(taesd, &sov_melanin, &sov_roughness, &sov_irid);
    }

    uint32_t *pixels = (uint32_t*)lau_malloc(W * H * sizeof(uint32_t));
    float *depth_field = (float*)lau_malloc(W * H * sizeof(float));
    NandTrapState nand;

    printf("[MASTERPIECE] Initiating Path-Traced Matrix...\n");

    for (int f = 0; f < FRAMES; f++) {
        float t = (float)f / (float)(FRAMES - 1);
        
        tsfi_nand_ipomoea_lifecycle(&nand, t);
        tsfi_svdag_compile_nand(dag_flower, &nand);

        NandTrapState quad_bear_nand;
        memset(&quad_bear_nand, 0, sizeof(quad_bear_nand));
        float offsets[4][3] = {{-0.5, 0.5, 0}, {0.5, 0.5, 0}, {-0.5, -0.5, 0}, {0.5, -0.5, 0}};
        for(int q=0; q<4; q++) {
            for(int i=0; i<256; i++) {
                int dst = q*256 + i;
                quad_bear_nand.fibers[dst].x = neural_bear_nand.fibers[i].x * 0.4f + offsets[q][0];
                quad_bear_nand.fibers[dst].y = neural_bear_nand.fibers[i].y * 0.4f + offsets[q][1];
                quad_bear_nand.fibers[dst].z = neural_bear_nand.fibers[i].z * 0.4f + offsets[q][2];
                quad_bear_nand.fibers[dst].weight = neural_bear_nand.fibers[i].weight;
            }
        }
        tsfi_svdag_compile_nand(dag_bear, &quad_bear_nand);

        tsfi_svdag_path_trace(pixels, depth_field, dag_flower, dag_bear, W, H, t, sov_melanin, sov_roughness, sov_irid);
        apply_taesd_modulation(pixels, W, H, taesd, t);

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
        
        // Export Depth Map
        char dpath[256];
        snprintf(dpath, sizeof(dpath), "%s/depth_%03d.ppm", out_dir, f);
        FILE *df_file = fopen(dpath, "wb");
        if (df_file) {
            fprintf(df_file, "P6\n%d %d\n255\n", W, H);
            for (int i = 0; i < W * H; i++) {
                uint8_t d_val = (uint8_t)(fminf(1.0f, depth_field[i] / 5.0f) * 255);
                uint8_t d_rgb[3] = { d_val, d_val, d_val };
                fwrite(d_rgb, 1, 3, df_file);
            }
            fclose(df_file);
        }

        if (f % 50 == 0) printf("[GEN] Frame %d/%d exported (RGB+D)\n", f, FRAMES);
    }

    tsfi_svdag_destroy(dag_flower);
    tsfi_svdag_destroy(dag_bear);
    lau_free(pixels);
    return 0;
}
