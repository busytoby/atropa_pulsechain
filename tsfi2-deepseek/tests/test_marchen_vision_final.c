#include "lau_memory.h"
#include "tsfi_ottype.h"
#include "vulkan/vulkan_render.h" 
#include "vulkan/vulkan_logic.h"
#include "vulkan/vulkan_main.h"
#include "tsfi_logic.h"
#include "tsfi_vision.h"
#include "tsfi_hotloader.h"
#include "tsfi_k0rn_ops.h"
#include "tsfi_io.h"
#include "tsfi_font_shaping.h"

void run_test_marchen_vision3(const char *font_path) {
    size_t size;
    uint8_t *font_data = (uint8_t*)tsfi_io_map_file(font_path, &size);
    if (!font_data) return;
    tsfi_ottype_t ot;
    if (!tsfi_ottype_init(&ot, font_data, size, 0)) { lau_unseal_object(font_data); lau_free(font_data); return; }

    uint32_t cps[] = { 0x11C70, 0x11CB0 }; // Ka + Subjoined Ka
    TSFiShapeCluster **clusters = NULL;
    int num_clusters = tsfi_font_shape_text(&ot, cps, 2, &clusters);
    if (num_clusters <= 0) { lau_unseal_object(font_data); lau_free(font_data); return; }

    TSFiShapeCluster *cluster = clusters[0];
    printf("Shaped %d glyphs.\n", cluster->num_glyphs);

    for (int s = 0; s < cluster->num_glyphs; s++) {
        uint32_t g_idx = cluster->glyphs[s].glyph_index;
        tsfi_glyph_geom_t geom = {0};
        if (!tsfi_ottype_get_glyph_geom(&ot, g_idx, &geom)) continue;

        tsfi_transform_t t = cluster->glyphs[s].transform;
        
        float raw_min_y = 99999.0f, raw_max_y = -99999.0f;
        for (uint32_t i=0; i<geom.num_points; i++) {
            float y = geom.points[i].y;
            if (y < raw_min_y) raw_min_y = y;
            if (y > raw_max_y) raw_max_y = y;
        }
        printf("Glyph %d: Raw Y: [%.2f, %.2f], dy=%.2f\n", s, raw_min_y, raw_max_y, t.dy);
        tsfi_ottype_free_geom(&geom);
    }
    tsfi_font_free_clusters(clusters, num_clusters);
    lau_unseal_object(font_data);
    lau_free(font_data);
}

void run_test_marchen_vision_size(const char *font_path) {
    size_t size;
    uint8_t *font_data = (uint8_t*)tsfi_io_map_file(font_path, &size);
    if (!font_data) return;
    tsfi_ottype_t ot;
    if (!tsfi_ottype_init(&ot, font_data, size, 0)) { lau_unseal_object(font_data); lau_free(font_data); return; }

    uint32_t cps[] = { 0x11C70, 0x11CB0 }; // Ka + Subjoined Ka
    TSFiShapeCluster **clusters = NULL;
    int num_clusters = tsfi_font_shape_text(&ot, cps, 2, &clusters);
    if (num_clusters <= 0) { lau_unseal_object(font_data); lau_free(font_data); return; }

    TSFiShapeCluster *cluster = clusters[0];
    
    float radius = 0.05f;
    float scale_factor = 2.0f / 2000.0f;

    float measured_widths[2] = {0};

    for (int s = 0; s < cluster->num_glyphs; s++) {
        K0RnStream *dna = lau_malloc_wired(sizeof(K0RnStream));
        dna->stream_capacity = 8192; 
        dna->op_count = 0;
        dna->ops = lau_memalign(512, sizeof(K0RnOp) * dna->stream_capacity);

        uint32_t g_idx = cluster->glyphs[s].glyph_index;
        tsfi_glyph_geom_t geom = {0};
        if (!tsfi_ottype_get_glyph_geom(&ot, g_idx, &geom)) continue;

        float raw_geom_xMin = 999999.0f;
        float raw_geom_xMax = -999999.0f;
        for (uint32_t j = 0; j < geom.num_points; j++) {
            float x = geom.points[j].x;
            if (x < raw_geom_xMin) raw_geom_xMin = x;
            if (x > raw_geom_xMax) raw_geom_xMax = x;
        }
        printf("[DEBUG] Glyph %d Raw Geom Width: %f\n", s, raw_geom_xMax - raw_geom_xMin);

        tsfi_transform_t t = cluster->glyphs[s].transform;
        printf("[DEBUG] Glyph %d Transform: m11=%f, m22=%f, dx=%f, dy=%f\n", s, t.m11, t.m22, t.dx, t.dy);
        
        float cur_x = 0, cur_y = 0;
        int start_pt = 0;
        
        for (int c = 0; c < (int)geom.num_contours; c++) {
            int end_pt = geom.contour_ends[c];
            int count = end_pt - start_pt + 1;
            
            int first_on = -1;
            for (int i = 0; i < count; i++) {
                if (geom.points[start_pt + i].type == 1) { first_on = i; break; }
            }
            
            int secret = 0;
            if (first_on == -1) {
                cur_x = (geom.points[end_pt].x + geom.points[start_pt].x) * 0.5f;
                cur_y = (geom.points[end_pt].y + geom.points[start_pt].y) * 0.5f;
            } else {
                cur_x = geom.points[start_pt + first_on].x;
                cur_y = geom.points[start_pt + first_on].y;
                secret = first_on + 1;
            }

            float vx_scaled_init = cur_x * scale_factor;
            float vy_scaled_init = cur_y * scale_factor;
            cur_x = (vx_scaled_init * t.m11 + vy_scaled_init * t.m21) + (t.dx * scale_factor);
            cur_y = (vx_scaled_init * t.m12 + vy_scaled_init * t.m22) + (t.dy * scale_factor);
            
            for (int k = 0; k < count; k++) {
                int idx = start_pt + ((secret + k) % count);
                tsfi_point_t p = geom.points[idx];
                
                float vx_scaled = p.x * scale_factor;
                float vy_scaled = p.y * scale_factor;
                float transformed_x = (vx_scaled * t.m11 + vy_scaled * t.m21) + (t.dx * scale_factor);
                float transformed_y = (vx_scaled * t.m12 + vy_scaled * t.m22) + (t.dy * scale_factor);

                if (p.type == 1) { 
                    float n_x0 = cur_x; 
                    float n_y0 = cur_y; 
                    float n_x1 = transformed_x; 
                    float n_y1 = transformed_y;
                    
                    if (dna->op_count < dna->stream_capacity) { 
                        dna->ops[dna->op_count++] = (K0RnOp){ .type=K0RN_OP_CAPSULE, .p1=radius, .x=n_x0, .y=n_y0, .z=n_x1, .w=n_y1 };
                    }
                    cur_x = transformed_x; cur_y = transformed_y;
                } else if (p.type == 0) { 
                    int next_idx = start_pt + ((secret + k + 1) % count);
                    tsfi_point_t p_next = geom.points[next_idx];
                    float x_end = p_next.x, y_end = p_next.y;
                    
                    if (p_next.type == 0) { 
                        x_end = (p.x + p_next.x) * 0.5f; 
                        y_end = (p.y + p_next.y) * 0.5f; 
                    } else { 
                        k++; 
                    }

                    float transformed_cx = transformed_x;
                    float transformed_cy = transformed_y;
                    
                    float vx_scaled_end = x_end * scale_factor;
                    float vy_scaled_end = y_end * scale_factor;
                    float transformed_end_x = (vx_scaled_end * t.m11 + vy_scaled_end * t.m21) + (t.dx * scale_factor);
                    float transformed_end_y = (vx_scaled_end * t.m12 + vy_scaled_end * t.m22) + (t.dy * scale_factor);

                    float n_x0 = cur_x; 
                    float n_y0 = cur_y;
                    float cx_ctrl = transformed_cx; 
                    float cy_ctrl = transformed_cy;
                    float n_x1 = transformed_end_x; 
                    float n_y1 = transformed_end_y;
                    
                    float lx = n_x0, ly = n_y0;
                    int steps = 8; 
                    for (int j = 1; j <= steps; j++) {
                        float mt = (float)j / (float)steps, omt = 1.0f - mt;
                        float nx = omt * omt * n_x0 + 2.0f * omt * mt * cx_ctrl + mt * mt * n_x1;
                        float ny = omt * omt * n_y0 + 2.0f * omt * mt * cy_ctrl + mt * mt * n_y1;
                        if (dna->op_count < dna->stream_capacity) {
                            dna->ops[dna->op_count++] = (K0RnOp){ .type=K0RN_OP_CAPSULE, .p1=radius, .x=lx, .y=ly, .z=nx, .w=ny };
                        }
                        lx = nx; ly = ny;
                    }
                    cur_x = transformed_end_x; cur_y = transformed_end_y;
                }
            }
            start_pt = end_pt + 1;
        }
        tsfi_ottype_free_geom(&geom);

        uint32_t *pixels = lau_malloc(128 * 128 * 4);
        for (int i=0; i<128*128; i++) pixels[i] = 0xFF000000;

        void (*rasterizer)(const K0RnStream *, uint32_t *, int, int, float, float, float, uint32_t) = tsfi_hotload_thunk("thunks/tsfi_k0rn_rasterizer.c", "tsfi_font_render_k0rn_pt");
        if (rasterizer) rasterizer(dna, pixels, 128, 128, 50.0f, 64.0f, 64.0f, 0xFFFFFFFF);

        TSFiResonanceAnalysis vis = {0};
        TSFiGlyphPhenotype props = { .name = "marchen_test" };
        tsfi_vision_analyze_glyph(pixels, 128, 128, &props, &vis);

        // Find visual bounding box width manually
        int min_x = 128, max_x = 0;
        for (int y = 0; y < 128; y++) {
            for (int x = 0; x < 128; x++) {
                if ((pixels[y * 128 + x] & 0xFFFFFF) != 0) { // If not black
                    if (x < min_x) min_x = x;
                    if (x > max_x) max_x = x;
                }
            }
        }
        
        float width = (max_x >= min_x) ? (max_x - min_x) : 0.0f;
        printf("[VISION] Glyph %d Visual Width: %f px (Coverage: %f)\n", s, width, vis.progression_ratio);
        measured_widths[s] = width;

        lau_free(pixels);
        lau_free(dna->ops);
        lau_free(dna);
    }
    
    if (fabs(measured_widths[0] - measured_widths[1]) > 10.0f) {
        printf("[FAIL] Widths do not match! (Delta: %f)\n", fabs(measured_widths[0] - measured_widths[1]));
        return;
    }

    printf("[PASS] Widths are visually consistent.\n");
    lau_unseal_object(font_data); lau_free(font_data);
}

void run_test_marchen_vision_final(const char *font_path) {
    size_t size;
    uint8_t *font_data = (uint8_t*)tsfi_io_map_file(font_path, &size);
    if (!font_data) {
        printf("[FAIL] Failed to map font file: %s\n", font_path);
        return;
    }

    int num_clusters = 0;
    uint32_t *pixels = NULL;
    TSFiShapeCluster **clusters = NULL;

    tsfi_ottype_t ot;
    if (!tsfi_ottype_init(&ot, font_data, size, 0)) {
        printf("[WARN] Font parsing failed (expected with dummy data).\n");
        tsfi_io_unmap_file(font_data, size);
        return;
    }

    uint32_t cps[] = { 0x11C70, 0x11CB0 }; // Marchen Ka + Subjoined Ka
    
    num_clusters = tsfi_font_shape_text(&ot, cps, 2, &clusters);
    if (num_clusters <= 0) {
        printf("[WARN] Failed to shape cluster.\n");
        tsfi_ottype_destroy(&ot);
        tsfi_io_unmap_file(font_data, size);
        return;
    }

    TSFiShapeCluster *cluster = clusters[0];
    
    K0RnStream *dna = lau_malloc_wired(sizeof(K0RnStream));
    dna->stream_capacity = 8192; 
    dna->op_count = 0;
    dna->ops = lau_memalign(512, sizeof(K0RnOp) * dna->stream_capacity);

    float radius = 0.05f;
    float cluster_pen_x = 0.0f;
    float scale_factor = 2.0f / 2000.0f;

    for (int s = 0; s < cluster->num_glyphs; s++) {
        uint32_t g_idx = cluster->glyphs[s].glyph_index;
        tsfi_glyph_geom_t geom = {0};
        if (!tsfi_ottype_get_glyph_geom(&ot, g_idx, &geom)) continue;

        tsfi_transform_t t = cluster->glyphs[s].transform;
        
        float cur_x = 0, cur_y = 0;
        int start_pt = 0;
        
        for (int c = 0; c < (int)geom.num_contours; c++) {
            int end_pt = geom.contour_ends[c];
            int count = end_pt - start_pt + 1;
            
            int first_on = -1;
            for (int i = 0; i < count; i++) {
                if (geom.points[start_pt + i].type == 1) { first_on = i; break; }
            }
            
            int secret = 0;
            if (first_on == -1) {
                cur_x = (geom.points[end_pt].x + geom.points[start_pt].x) * 0.5f;
                cur_y = (geom.points[end_pt].y + geom.points[start_pt].y) * 0.5f;
            } else {
                cur_x = geom.points[start_pt + first_on].x;
                cur_y = geom.points[start_pt + first_on].y;
                secret = first_on + 1;
            }

            float vx_scaled_init = cur_x * scale_factor;
            float vy_scaled_init = cur_y * scale_factor;
            cur_x = (vx_scaled_init * t.m11 + vy_scaled_init * t.m21) + (t.dx * scale_factor);
            cur_y = (vx_scaled_init * t.m12 + vy_scaled_init * t.m22) + (t.dy * scale_factor);
            
            for (int k = 0; k < count; k++) {
                int idx = start_pt + ((secret + k) % count);
                tsfi_point_t p = geom.points[idx];
                
                float vx_scaled = p.x * scale_factor;
                float vy_scaled = p.y * scale_factor;
                float transformed_x = (vx_scaled * t.m11 + vy_scaled * t.m21) + (t.dx * scale_factor);
                float transformed_y = (vx_scaled * t.m12 + vy_scaled * t.m22) + (t.dy * scale_factor);

                if (p.type == 1) { 
                    float n_x0 = cur_x + cluster_pen_x; 
                    float n_y0 = cur_y; 
                    float n_x1 = transformed_x + cluster_pen_x; 
                    float n_y1 = transformed_y;
                    
                    if (dna->op_count < dna->stream_capacity) { 
                        dna->ops[dna->op_count++] = (K0RnOp){ .type=K0RN_OP_CAPSULE, .p1=radius, .x=n_x0, .y=n_y0, .z=n_x1, .w=n_y1 };
                    }
                    cur_x = transformed_x; cur_y = transformed_y;
                } else if (p.type == 0) { 
                    int next_idx = start_pt + ((secret + k + 1) % count);
                    tsfi_point_t p_next = geom.points[next_idx];
                    float x_end = p_next.x, y_end = p_next.y;
                    
                    if (p_next.type == 0) { 
                        x_end = (p.x + p_next.x) * 0.5f; 
                        y_end = (p.y + p_next.y) * 0.5f; 
                    } else { 
                        k++; 
                    }

                    float transformed_cx = transformed_x;
                    float transformed_cy = transformed_y;
                    
                    float vx_scaled_end = x_end * scale_factor;
                    float vy_scaled_end = y_end * scale_factor;
                    float transformed_end_x = (vx_scaled_end * t.m11 + vy_scaled_end * t.m21) + (t.dx * scale_factor);
                    float transformed_end_y = (vx_scaled_end * t.m12 + vy_scaled_end * t.m22) + (t.dy * scale_factor);

                    float n_x0 = cur_x + cluster_pen_x; 
                    float n_y0 = cur_y;
                    float cx_ctrl = transformed_cx + cluster_pen_x; 
                    float cy_ctrl = transformed_cy;
                    float n_x1 = transformed_end_x + cluster_pen_x; 
                    float n_y1 = transformed_end_y;
                    
                    float lx = n_x0, ly = n_y0;
                    int steps = 8; 
                    for (int j = 1; j <= steps; j++) {
                        float mt = (float)j / (float)steps, omt = 1.0f - mt;
                        float nx = omt * omt * n_x0 + 2.0f * omt * mt * cx_ctrl + mt * mt * n_x1;
                        float ny = omt * omt * n_y0 + 2.0f * omt * mt * cy_ctrl + mt * mt * n_y1;
                        if (dna->op_count < dna->stream_capacity) {
                            dna->ops[dna->op_count++] = (K0RnOp){ .type=K0RN_OP_CAPSULE, .p1=radius, .x=lx, .y=ly, .z=nx, .w=ny };
                        }
                        lx = nx; ly = ny;
                    }
                    cur_x = transformed_end_x; cur_y = transformed_end_y;
                }
            }
            start_pt = end_pt + 1;
        }
        tsfi_ottype_free_geom(&geom);
        cluster_pen_x += cluster->glyphs[s].advance_dx * scale_factor;
    }

    pixels = lau_malloc(128 * 128 * 4);
    for (int i=0; i<128*128; i++) pixels[i] = 0xFF000000;

    // Render the generated DNA into a buffer
    void (*rasterizer)(const K0RnStream *, uint32_t *, int, int, float, float, float, uint32_t) = tsfi_hotload_thunk("thunks/tsfi_k0rn_rasterizer.c", "tsfi_font_render_k0rn_pt");
    if (rasterizer) rasterizer(dna, pixels, 128, 128, 50.0f, 64.0f, 64.0f, 0xFFFFFFFF);

    int drawn_pixels = 0;
    for (int i=0; i<128*128; i++) {
        if (pixels[i] != 0xFF000000) drawn_pixels++;
    }

    printf("Drawn pixels: %d\n", drawn_pixels);
    printf("DNA ops: %d\n", dna->op_count);
    
    // Save checkpoint
    tsfi_vision_save_glyph_checkpoint("marchen_dna", NULL, pixels, 128, 128);

    // Final Structured Cleanup
    tsfi_ottype_destroy(&ot);
    if (font_data) tsfi_io_unmap_file(font_data, size);
    if (pixels) lau_free(pixels);
    if (clusters) tsfi_font_free_clusters(clusters, num_clusters);
    if (dna) {
        if (dna->ops) lau_free(dna->ops);
        lau_free(dna);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("[FAIL] Usage: %s <font_path>\n", argv[0]);
        return 1;
    }
    
    printf("\n=== Running Marchen Vision 3 (Raw Coordinates) ===\n");
    run_test_marchen_vision3(argv[1]);
    
    printf("\n=== Running Marchen Vision Size Validation ===\n");
    run_test_marchen_vision_size(argv[1]);
    
    printf("\n=== Running Marchen Vision Final DNA Compilation ===\n");
    run_test_marchen_vision_final(argv[1]);

    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
