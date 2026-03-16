#include "tsfi_safetensors_cache.h"
#include "tsfi_temporal_warper.h"
#include "tsfi_wire_firmware.h"
#include "lau_memory.h"
#include "lau_registry.h"
#include "tsfi_vision.h"
#include "tsfi_design_pipeline.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

/**
 * @brief Teddy Bear Channel: High-Speed Animation Loop
 */

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <prompt> [frames]\n", argv[0]);
        return 1;
    }

    const char *prompt = argv[1];
    int total_frames = (argc > 2) ? atoi(argv[2]) : 60;

    // 1. System Integration
    tsfi_wire_firmware_init();
    LauWireFirmware *fw = tsfi_wire_firmware_get();
    if (!fw) return 1;

    printf("[ANIM] Teddy Bear Channel Online. Manifold Attached.\n");

    // 2. Safetensors Warm Attachment
    TsfiSafetensorsAsset *model = tsfi_safetensors_cache_attach("assets/models/sd15.safetensors");
    TsfiSafetensorsAsset *lcm = tsfi_safetensors_cache_attach("assets/models/lcm_lora_sd15.safetensors");
    if (!model || !lcm) return 1;

    // 3. Animation Loop
    int w = 256, h = 256; // We updated SD worker to 256
    int lw = w/8, lh = h/8;
    size_t latent_sz = lw * lh * 4 * sizeof(float);
    float *last_latent = (float*)lau_malloc_wired(latent_sz);
    memset(last_latent, 0, latent_sz);

    char cmd[1024];
    for (int i = 0; i < total_frames; i++) {
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);

        // A. Warp Phase (Temporal Feedback) & Skeleton Generation
        if (i > 0) {
            tsfi_latent_warp_translate(last_latent, lw, lh, 1, 0); // Slow horizontal drift
            
            // Save for re-injection
            FILE *f = fopen("tmp/last_latent.bin", "wb");
            if (f) {
                fwrite(last_latent, 1, latent_sz, f);
                fclose(f);
            }
        }
        
        // Generate Dynamic 16-inch Skeleton (Depth + Pose)
        float t = (float)i * 0.1f;
        char skel_cmd[256];
        snprintf(skel_cmd, sizeof(skel_cmd), "bin/tsfi_teddy_skeleton %f", t);
        system(skel_cmd);

        // B. Generation Phase (Seek-and-Warp)
        int steps = (i == 0) ? 8 : 2; // Initial frame is higher quality, then high-speed seek
        snprintf(cmd, sizeof(cmd), "bin/tsfi_sd_worker \"%s\" \"tmp/frame_%04d.raw\" \"tmp/current_depth.raw\" \"tmp/current_pose.raw\" %d \"LCM\" 1.0", prompt, i, steps);
        
        int ret = system(cmd);
        if (ret != 0) {
            fprintf(stderr, "[FRACTURE] Generation failed at frame %d\n", i);
            break;
        }

        // C. Capture Latent for next frame
        FILE *lf = fopen("tmp/sd_latent_out.bin", "rb");
        if (lf) {
            fread(last_latent, 1, latent_sz, lf);
            fclose(lf);
        }

        // D. Vision Analysis & Pipeline Storage
        char frame_path[256];
        snprintf(frame_path, sizeof(frame_path), "tmp/frame_%04d.raw", i);
        FILE *rf = fopen(frame_path, "rb");
        if (rf) {
            uint8_t *rgb = (uint8_t*)malloc(w * h * 3);
            fread(rgb, 1, w * h * 3, rf);
            fclose(rf);

            uint32_t *rgba = (uint32_t*)malloc(w * h * 4);
            for (int p=0; p<w*h; p++) {
                rgba[p] = (0xFF000000) | (rgb[p*3+2] << 16) | (rgb[p*3+1] << 8) | rgb[p*3];
            }

            TSFiResonanceAnalysis vis = {0};
            TSFiFlowerPhenotype dummy_p = {0};
            tsfi_vision_analyze_staging(rgba, w, h, &dummy_p, &vis);

            // Fetch current ZMM Mass Density from firmware (simulated for now)
            float current_density = 1.0f; // Could be fw->manifold.count

            float fitness = tsfi_eval_teddy_fitness(&vis, current_density);
            
            LauTelemetryState *telem = lau_telemetry_get_state();
            if (telem) {
                telem->recip_symmetry = fitness;
                telem->zmm_val = (uint64_t)(current_density * 1000.0f);
                snprintf((char*)telem->zmm_msg, 128, "Teddy Fitness: %.4f", fitness);
            }

            // If the render is exceptional, save it to the permanent pipeline
            if (fitness > 0.85f) {
                char render_id[64];
                snprintf(render_id, sizeof(render_id), "teddy_gen_%ld_%04d", time(NULL), i);
                tsfi_pipeline_archive_candidate(render_id, rgb, w * h * 3, last_latent, latent_sz, fitness);
            }

            free(rgb);
            free(rgba);
        }

        clock_gettime(CLOCK_MONOTONIC, &end);
        double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        printf("[ANIM] Frame %04d: %.3fs (%.1f FPS)\n", i, elapsed, 1.0/elapsed);
    }

    tsfi_safetensors_cache_detach(model);
    tsfi_safetensors_cache_detach(lcm);
    lau_free(last_latent);
    
    return 0;
}
