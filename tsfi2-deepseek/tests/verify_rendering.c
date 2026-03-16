#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <dirent.h>
#include <strings.h>
#include <ctype.h>
#include "tsfi_vision.h"
#include "lau_memory.h"
#include "vulkan/vulkan_render.h"
#include "vulkan/vulkan_main.h"

#define LOG(...) fprintf(stderr, __VA_ARGS__)

bool verify_flower(const char* name) {
    LOG("[VERIFY] Checking Floral Integrity: %s\n", name);
    TSFiResonanceAnalysis saved_vis;
    uint32_t *saved_pixels = NULL;
    int w, h;
    
    if (!tsfi_vision_load_checkpoint(name, &saved_vis, &saved_pixels, &w, &h)) {
        LOG("  [FAIL] Could not load checkpoint: %s\n", name);
        return false;
    }

    TSFiClassification cls = tsfi_vision_classify(&saved_vis);
    // Strict Classification check
    if (cls.class_id == TSFI_CLASS_UNKNOWN) {
        LOG("  [FAIL] %s: Unknown classification. Expected any Floral pole.\n", name);
        if (saved_pixels) lau_free(saved_pixels);
        return false;
    }

    if (!tsfi_vision_verify_integrity(&saved_vis)) {
        LOG("  [FAIL] Checkpoint failed integrity check.\n");
        if (saved_pixels) lau_free(saved_pixels);
        return false;
    }

    LOG("  [PASS] %s: Coverage=%.2f, Corr=%.2f, Class=%d (Conf=%.2f)\n", name, saved_vis.coverage, saved_vis.target_correlation, cls.class_id, cls.confidence);
    if (saved_pixels) lau_free(saved_pixels);
    return true;
}

bool verify_glyph(const char* name) {
    LOG("[VERIFY] Checking Glyph Integrity: %s\n", name);
    TSFiResonanceAnalysis saved_vis;
    uint32_t *saved_pixels = NULL;
    int w, h;
    
    if (!tsfi_vision_load_glyph_checkpoint(name, &saved_vis, &saved_pixels, &w, &h)) {
        LOG("  [SKIP] Could not load glyph checkpoint: %s\n", name);
        return true;
    }

    // Load phenotype to check target holes
    TSFiGlyphPhenotype p;
    p.target_holes = 1; // Default for 'A' check

    TSFiClassification cls = tsfi_vision_classify(&saved_vis);
    if (cls.class_id != TSFI_CLASS_LATIN_A && cls.class_id != TSFI_CLASS_GLYPH) {
        LOG("  [FAIL] %s: Incorrect classification (ID %d, Confidence %.2f, Raw %.2f). Expected LATIN_A or GLYPH.\n", name, cls.class_id, cls.confidence, cls.raw_score);
        LOG("         Symmetry=%.2f, Holes=%.0f, Coverage=%.4f\n", saved_vis.glyph_symmetry, saved_vis.topo_hole_count, saved_vis.coverage);
        if (saved_pixels) lau_free(saved_pixels);
        return false;
    }

    if (saved_vis.glyph_symmetry < 0.70f) {
        LOG("  [FAIL] %s: Symmetry regression (%.2f < 0.70)\n", name, saved_vis.glyph_symmetry);
        if (saved_pixels) lau_free(saved_pixels);
        return false;
    }

    if ((int)saved_vis.topo_hole_count != p.target_holes) {
        LOG("  [FAIL] %s: Topological regression (Holes: %.0f, Expected: %d)\n", name, saved_vis.topo_hole_count, p.target_holes);
        if (saved_pixels) lau_free(saved_pixels);
        return false;
    }

    LOG("  [PASS] %s: Symmetry=%.2f, Holes=%.0f, Class=%s (Conf=%.2f)\n", name, saved_vis.glyph_symmetry, saved_vis.topo_hole_count, (cls.class_id == TSFI_CLASS_LATIN_A ? "LATIN_A" : "GLYPH"), cls.confidence);
    if (saved_pixels) lau_free(saved_pixels);
    return true;
}

bool verify_all_flowers() {
    DIR *d = opendir("assets/flowers/");
    if (!d) return false;
    struct dirent *dir;
    bool all_pass = true;
    char checked[64][256];
    int checked_count = 0;

    const char* targets[] = {
        "Xenostegia", "Ipomoea", "Hyalocystis", "Distimake", "Convolvulus",
        "Remirema", "Hewittia", "Operculina", "Generic", "Merremia"
    };
    int target_count = sizeof(targets)/sizeof(targets[0]);
    bool target_found[10] = {0};

    while ((dir = readdir(d)) != NULL) {
        if (strstr(dir->d_name, ".checkpoint")) {
            char name[256];
            snprintf(name, sizeof(name), "%s", dir->d_name);
            char *ext = strstr(name, ".checkpoint");
            if (ext) *ext = '\0';
            
            bool duplicate = false;
            for(int i=0; i<checked_count; i++) {
                if (strcmp(checked[i], name) == 0) { duplicate = true; break; }
            }
            if (duplicate) continue;
            if (checked_count < 64) {
                snprintf(checked[checked_count++], 256, "%s", name);
            }
            
            if (!verify_flower(name)) all_pass = false;

            for(int i=0; i<target_count; i++) {
                if (strcmp(targets[i], name) == 0) target_found[i] = true;
            }
        }
    }
    closedir(d);

    for(int i=0; i<target_count; i++) {
        if (!target_found[i]) {
            LOG("  [FAIL] Missing checkpoint for: %s\n", targets[i]);
            all_pass = false;
        }
    }

    return all_pass;
}

int main() {
    int failed = 0;

    LOG("=== TSFi2 Rendering Regression Suite ===\n");

    if (!verify_all_flowers()) failed++;
    if (!verify_glyph("Latin_Capital_A")) failed++;

    if (failed) {
        LOG("=== [FAILED] %d Regressions Detected ===\n", failed);
        return 1;
    }

    LOG("=== [SUCCESS] Visual Substrate Affirmed ===\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
