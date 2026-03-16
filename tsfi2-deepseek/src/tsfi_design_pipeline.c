#include "tsfi_vision_internal.h"
#include "tsfi_opt_zmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/**
 * @brief Multi-Generational Intentional Design Pipeline
 */

typedef struct {
    char render_id[64];
    float fitness_score;
    int generation;
} TsfiDesignMeta;

// Evaluates the "Teddy Fitness" based on Visual Symmetry and ZMM Mass Density
float tsfi_eval_teddy_fitness(const TSFiResonanceAnalysis *vision, float mass_density) {
    if (!vision) return 0.0f;
    
    // Weighted fitness: 40% Symmetry, 30% Complexity, 30% ZMM Stability
    float symmetry_score = vision->glyph_symmetry;
    float complexity_score = vision->complexity;
    float density_score = (mass_density > 0) ? (1.0f / (1.0f + 1.0f/mass_density)) : 0.0f;

    return (symmetry_score * 0.4f) + (complexity_score * 0.3f) + (density_score * 0.3f);
}

// Archives a "best" candidate into long-term storage
bool tsfi_pipeline_archive_candidate(const char *id, void *pixels, size_t pix_size, void *latents, size_t lat_size, float score) {
    char dir[256];
    snprintf(dir, sizeof(dir), "assets/long_term_storage/%s", id);
    mkdir("assets/long_term_storage", 0777);
    mkdir(dir, 0777);

    char path[512];
    
    // 1. Save Render
    snprintf(path, sizeof(path), "%s/render.raw", dir);
    FILE *f1 = fopen(path, "wb");
    if (f1) { fwrite(pixels, 1, pix_size, f1); fclose(f1); }

    // 2. Save Latents (The Genetic DNA)
    snprintf(path, sizeof(path), "%s/latent.bin", dir);
    FILE *f2 = fopen(path, "wb");
    if (f2) { fwrite(latents, 1, lat_size, f2); fclose(f2); }

    // 3. Save Meta
    snprintf(path, sizeof(path), "%s/meta.txt", dir);
    FILE *f3 = fopen(path, "w");
    if (f3) {
        fprintf(f3, "id=%s\n", id);
        fprintf(f3, "fitness=%.4f\n", score);
        fprintf(f3, "status=archived\n");
        fclose(f3);
    }

    printf("[PIPELINE] Archived High-Quality Candidate: %s (Score: %.4f)\n", id, score);
    return true;
}
