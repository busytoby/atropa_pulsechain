#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <linux/input-event-codes.h>

#include "lau_memory.h"
#include "tsfi_ottype.h"
#include "vulkan/vulkan_render.h" 
#include "vulkan/vulkan_logic.h"
#include "vulkan/vulkan_main.h"
#include "vulkan/vulkan_input.h"
#include "tsfi_logic.h"
#include "tsfi_vision.h"
#include "tsfi_k0rn_ops.h"
#include "tsfi_io.h"
#include "tsfi_font_vectors.h"
#include "stb_truetype.h"
#include "window_inc/tsfi_input.h"

#include "tsfi_quan.h"
#include "tsfi_font_registry.h"

// --- Global State ---
#define MAX_TEXT 4096
static uint32_t g_text_buffer[MAX_TEXT];
static int g_text_len = 0;
static int g_status_timer = 0;
static char g_status_msg[64] = "";
static TSFiFontSystem *g_fs = NULL;
static TSFiFontRegistry *g_registry = NULL;
static QuanState *g_quan = NULL;
static bool g_force_quit = false;
static bool g_ctrl_held = false;
static volatile bool g_show_reference = true;
static volatile bool g_dirty = true;

static void handle_sig(int sig) {
    (void)sig;
    g_force_quit = true;
}

// --- Drawing Primitives ---
static void draw_line(uint32_t *px, int W, int H, float x0, float y0, float x1, float y1, uint32_t color) {
    int ix0 = (int)roundf(x0), iy0 = (int)roundf(y0), ix1 = (int)roundf(x1), iy1 = (int)roundf(y1);
    int dx = abs(ix1 - ix0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(iy1 - iy0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    while (1) {
        if (ix0 >= 0 && ix0 < W && iy0 >= 0 && iy0 < H) {
            uint32_t existing = px[iy0 * W + ix0];
            uint8_t a = (color >> 24) & 0xFF;
            if (a == 255) px[iy0 * W + ix0] = color;
            else if (a > 0) {
                float fa = a / 255.0f;
                uint8_t r = (uint8_t)(((color >> 16) & 0xFF) * fa + ((existing >> 16) & 0xFF) * (1.0f - fa));
                uint8_t g = (uint8_t)(((color >> 8) & 0xFF) * fa + ((existing >> 8) & 0xFF) * (1.0f - fa));
                uint8_t b = (uint8_t)((color & 0xFF) * fa + (existing & 0xFF) * (1.0f - fa));
                px[iy0 * W + ix0] = (0xFFu << 24) | (r << 16) | (g << 8) | b;
            }
        }
        if (ix0 == ix1 && iy0 == iy1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; ix0 += sx; }
        if (e2 <= dx) { err += dx; iy0 += sy; }
    }
}

static void draw_quad_bezier(uint32_t *px, int W, int H, float x0, float y0, float x1, float y1, float x2, float y2, uint32_t color) {
    int steps = 8; 
    float lx = x0, ly = y0;
    for (int i = 1; i <= steps; i++) {
        float t = (float)i / (float)steps;
        float omt = 1.0f - t;
        float nx = omt * omt * x0 + 2.0f * omt * t * x1 + t * t * x2;
        float ny = omt * omt * y0 + 2.0f * omt * t * y1 + t * t * y2;
        draw_line(px, W, H, lx, ly, nx, ny, color);
        lx = nx; ly = ny;
    }
}

static stbtt_fontinfo init_local_stb(TSFiFontInfo *finfo) {
    stbtt_fontinfo stb = {0};
    if (finfo && finfo->data) {
        if (!stbtt_InitFont(&stb, finfo->data, finfo->ttc_secret)) {
            stb.data = NULL; // Explicitly mark invalid
        }
    }
    return stb;
}

static void draw_stb_shape(stbtt_fontinfo *info, uint32_t cp, uint32_t *px, int W, int H, float pt_size, float pen_x, float pen_y, uint32_t color) {
    if (!info || !info->data) return;
    float scale = stbtt_ScaleForPixelHeight(info, pt_size);
    stbtt_vertex *vertices;
    int g_idx = stbtt_FindGlyphIndex(info, cp);
    int num_vertices = stbtt_GetGlyphShape(info, g_idx, &vertices);
    if (!num_vertices) {
        if (vertices) stbtt_FreeShape(info, vertices);
        return;
    }
    
    float cur_x = 0, cur_y = 0;
    for(int i=0; i<num_vertices; i++) {
        stbtt_vertex v = vertices[i];
        float px0 = pen_x + cur_x * scale;
        float py0 = pen_y - cur_y * scale; 
        float px1 = pen_x + v.x * scale;
        float py1 = pen_y - v.y * scale;
        
        if (v.type == 2) { // line
            draw_line(px, W, H, px0, py0, px1, py1, color);
            cur_x = v.x; cur_y = v.y;
        } else if (v.type == 3) { // curve
            float pcx = pen_x + v.cx * scale;
            float pcy = pen_y - v.cy * scale;
            draw_quad_bezier(px, W, H, px0, py0, pcx, pcy, px1, py1, color);
            cur_x = v.x; cur_y = v.y;
        } else if (v.type == 1) { // move
            cur_x = v.x; cur_y = v.y;
        }
    }
    stbtt_FreeShape(info, vertices);
}

static bool is_combining_mark(uint32_t cp) {
    // Sinhala Vowel Signs and other marks
    if (cp >= 0xDD2 && cp <= 0xDDF) return true;
    if (cp == 0xDCA) return true;
    // General Unicode Combining Diacritics
    if (cp >= 0x0300 && cp <= 0x036F) return true;
    // Marchen Subjoined Consonants, Vowels, and Marks (11C92-11CB6)
    if (cp >= 0x11C92 && cp <= 0x11CB6) return true;
    return false;
}

static TSFiGlyphWavefront **g_wf_cache = NULL;
static int g_wf_cache_count = 0;
static int g_wf_cache_cap = 0;

// --- Editor-Specific DNA Generator (Automates Composites via STB) ---
// Now supports merging multiple codepoints (clusters) into one DNA context.
static TSFiGlyphWavefront* editor_get_cluster_dna(TSFiFontSystem *fs, const uint32_t *cps, int count) {
    (void)fs;
    if (count <= 0) return NULL;

    uint32_t cluster_hash = 0;
    for (int i=0; i<count; i++) cluster_hash = (cluster_hash * 31) + cps[i];

    for (int i=0; i<g_wf_cache_count; i++) {
        if (g_wf_cache[i]->codepoint == cluster_hash) return g_wf_cache[i];
    }

    // Use primary font for shaping the cluster
    TSFiFontInfo *base_finfo = tsfi_font_registry_find_support(g_registry, cps[0]);
    if (!base_finfo) {
        const char *fallback = tsfi_font_registry_get_fallback(g_registry, cps[0]);
        if (fallback) {
            bool already_loaded = false;
            for (int i = 0; i < g_registry->count; i++) {
                if (strncmp(g_registry->fonts[i].path, fallback, strlen(fallback)) == 0) {
                    already_loaded = true;
                    break;
                }
            }
            if (!already_loaded) {
                tsfi_io_printf(stdout, "[TYPEWRITER] Dynamically loading fallback for 0x%X: %s\n", cps[0], fallback);
                tsfi_font_registry_add(g_registry, fallback);
                base_finfo = tsfi_font_registry_find_support(g_registry, cps[0]);
            }
        }
    }

    if (!base_finfo) {
        return NULL;
    }

    K0RnStream *dna = lau_malloc_wired(sizeof(K0RnStream));
    dna->stream_capacity = 2048; 
    dna->op_count = 0;
    dna->ops = lau_memalign(512, sizeof(K0RnOp) * dna->stream_capacity);

    float radius = 0.05f;
    float cluster_pen_x = 0.0f;

    TSFiShapeCluster **clusters = NULL;
    int num_clusters = tsfi_font_shape_text(base_finfo->ot, cps, count, &clusters);
    if (num_clusters <= 0) {
        lau_free(dna->ops);
        lau_free(dna);
        return NULL;
    }

    TSFiShapeCluster *cluster = clusters[0]; // Assuming one syllable cluster for this typewriter hack
    int shaped_count = cluster->num_glyphs;

    int ascent = base_finfo->ot->ascent;
    int descent = base_finfo->ot->descent;
    if (ascent - descent == 0) { ascent = 1000; descent = -1000; }
    float scale_factor = 2.0f / (float)(ascent - descent); 

    for (int s = 0; s < shaped_count; s++) {
        uint32_t g_idx = cluster->glyphs[s].glyph_index;
        if (g_idx == 0) continue;

        tsfi_glyph_geom_t geom = {0};
        if (!tsfi_ottype_get_glyph_geom(base_finfo->ot, g_idx, &geom)) {
            continue;
        }

        // Extract the arbitrary transform for this specific glyph
        tsfi_transform_t t = cluster->glyphs[s].transform;

        float cur_x = 0, cur_y = 0;
        int start_pt = 0;
        
        for (int c = 0; c < (int)geom.num_contours; c++) {
            int end_pt = geom.contour_ends[c];
            int count = end_pt - start_pt + 1;
            
            // Re-order contour if needed to find first on-curve point
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

            // Apply matrix transformation to initial point
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

                if (p.type == 1) { // On curve (Line)
                    float n_x0 = cur_x + cluster_pen_x; 
                    float n_y0 = cur_y; 
                    float n_x1 = transformed_x + cluster_pen_x; 
                    float n_y1 = transformed_y;
                    
                    if (dna->op_count < dna->stream_capacity) { // Draw lines
                        dna->ops[dna->op_count++] = (K0RnOp){ .type=K0RN_OP_CAPSULE, .p1=radius, .x=n_x0, .y=n_y0, .z=n_x1, .w=n_y1 };
                    }
                    cur_x = transformed_x; cur_y = transformed_y;
                } else if (p.type == 2) { // Off curve (Quadratic Bezier)
                    int next_idx = start_pt + ((secret + k + 1) % count);
                    tsfi_point_t p_next = geom.points[next_idx];
                    float x_end = p_next.x, y_end = p_next.y;
                    
                    if (p_next.type == 2) { // Next is off-curve, interpolate implicit point
                        x_end = (p.x + p_next.x) * 0.5f; 
                        y_end = (p.y + p_next.y) * 0.5f; 
                    } else { 
                        k++; // Skip next point since it's the actual end point and we process it now
                    }

                    // Transform control point
                    float transformed_cx = transformed_x;
                    float transformed_cy = transformed_y;
                    
                    // Transform end point
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
    }
    
    cluster_pen_x += cluster->advance_dx * scale_factor;
    
    tsfi_font_free_clusters(clusters, num_clusters);

    if (dna->op_count == 0) {
        lau_free(dna->ops);
        lau_free(dna);
        return NULL;
    }

    if (g_wf_cache_count >= g_wf_cache_cap) {
        g_wf_cache_cap = g_wf_cache_cap == 0 ? 256 : g_wf_cache_cap * 2;
        g_wf_cache = lau_realloc(g_wf_cache, g_wf_cache_cap * sizeof(TSFiGlyphWavefront*));
    }

    TSFiGlyphWavefront *wf = lau_malloc_wired(sizeof(TSFiGlyphWavefront));
    wf->codepoint = cluster_hash;
    wf->dna = dna;
    // other style attributes could go here if needed later
    
    g_wf_cache[g_wf_cache_count++] = wf;
    return wf;
}

static void application_key_hook(void *data, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    (void)serial; (void)time;
    VulkanSystem *s = (VulkanSystem*)data;
    bool pressed = (state == 1 /* WL_KEYBOARD_KEY_STATE_PRESSED */);
    
    if (key == KEY_LEFTCTRL || key == KEY_RIGHTCTRL) {
        g_ctrl_held = pressed;
        return;
    }

    if (g_ctrl_held && key == KEY_F) {
        // Activate Quan with context of the last character set (default Marchen)
        tsfi_quan_activate(g_quan, TSFI_SCRIPT_MARCHEN);
        g_dirty = true;
        return;
    }

    if (g_quan->is_active) {
        if (key == KEY_BACKSPACE) {
            if (tsfi_quan_backspace(g_quan)) {
                g_dirty = true;
                return;
            }
        } else {
            uint32_t cluster[8];
            int cluster_len = 0;
            // Map key to char for Trie walk
            char c = (char)tsfi_input_map_to_utf32(key);
            if (tsfi_quan_input(g_quan, c, cluster, &cluster_len)) {
                if (cluster_len > 0) {
                    for (int j=0; j<cluster_len; j++) {
                        if (g_text_len < MAX_TEXT) g_text_buffer[g_text_len++] = cluster[j];
                    }
                }
                g_dirty = true;
                return;
            }
        }
    }

    if (!pressed) return;

    // Use the TSFi input map to reliably resolve the key
    uint32_t cp = tsfi_input_map_to_utf32(key);

    if (g_ctrl_held && key == KEY_W) {
        tsfi_io_printf(stdout, "[TYPEWRITER] QUIT via CTRL+W\n");
        s->running = false;
        g_force_quit = true;
        return;
    }

    if (g_ctrl_held && key == KEY_R) {
        g_show_reference = !g_show_reference;
        tsfi_io_printf(stdout, "[TYPEWRITER] Reference Guide: %s\n", g_show_reference ? "ON" : "OFF");
        g_dirty = true;
        return;
    }

    if (g_ctrl_held && key == KEY_S) {
        int saved = 0;
        for (int i = 0; i < g_text_len; i++) {
            uint32_t cp = g_text_buffer[i];
            if (cp == ' ' || cp == '\b' || cp == '\n') continue;
            if (g_fs->dna_indices[cp] > 0) {
                tsfi_font_save_prime(g_fs, cp);
                saved++;
            }
        }
        tsfi_io_printf(stdout, "[TYPEWRITER] Saved %d prime DNA sequences.\n", saved);
        g_dirty = true;
        return;
    }

    if (key == KEY_TAB) {
        int mutated = 0;
        for (int i = 0; i < g_text_len; i++) {
            uint32_t cp = g_text_buffer[i];
            if (cp == ' ' || cp == '\b' || cp == '\n') continue;
            uint32_t dna_idx = g_fs->dna_indices[cp];
            if (dna_idx > 0) {
                uint32_t rid = g_fs->unicode_map[cp];
                if (rid > 0) {
                    uint32_t op_count = g_fs->registry[rid].num_segments;
                    for(uint32_t j=0; j<op_count; j++) {
                        WAVE15 *atom = &g_fs->wave_elements[dna_idx + j];
                        // Randomly perturb endpoints (-16 to +16 in K-Space units)
                        atom->p0_x += (rand() % 33) - 16;
                        atom->p0_y += (rand() % 33) - 16;
                        atom->p1_x += (rand() % 33) - 16;
                        atom->p1_y += (rand() % 33) - 16;
                    }
                    mutated++;
                }
            }
        }
        tsfi_io_printf(stdout, "[TYPEWRITER] Mutated %d characters.\n", mutated);
        g_dirty = true;
        return;
    }

    if (g_ctrl_held && key == KEY_V) {
        tsfi_io_printf(stdout, "[TYPEWRITER] Paste via CTRL+V (Internal)\n");
        tsfi_input_request_paste(s);
        if (s->clipboard_buffer) {
            const char *buffer = s->clipboard_buffer;
            int i = 0;
            while (buffer[i] && g_text_len < MAX_TEXT) {
                uint32_t cp = 0;
                unsigned char c = buffer[i];
                if (c < 0x80) { cp = c; i += 1; }
                else if ((c & 0xE0) == 0xC0) { cp = ((c & 0x1F) << 6) | (buffer[i+1] & 0x3F); i += 2; }
                else if ((c & 0xF0) == 0xE0) { cp = ((c & 0x0F) << 12) | ((buffer[i+1] & 0x3F) << 6) | (buffer[i+2] & 0x3F); i += 3; }
                else if ((c & 0xF8) == 0xF0) { cp = ((c & 0x07) << 18) | ((buffer[i+1] & 0x3F) << 12) | ((buffer[i+2] & 0x3F) << 6) | (buffer[i+3] & 0x3F); i += 4; }
                else { i++; } 
                if (cp >= 32 || cp == '\n') {
                    g_text_buffer[g_text_len++] = cp;
                }
            }
            lau_free(s->clipboard_buffer);
            s->clipboard_buffer = NULL;
            snprintf(g_status_msg, sizeof(g_status_msg), "PASTED");
            g_status_timer = 60;
        }
        g_dirty = true;
        return;
    }

    if (key == KEY_BACKSPACE) {
        if (g_text_len > 0) g_text_len--;
        g_dirty = true;
        return;
    }

    if (cp != 0 && g_text_len < MAX_TEXT) {
        g_text_buffer[g_text_len++] = cp;
    }

    g_dirty = true;
}

static void application_resize_hook(void *data, int32_t width, int32_t height) {
    (void)data; (void)width; (void)height;
    g_dirty = true;
}

int main(int argc, char **argv) {
    signal(SIGINT, handle_sig);

    g_registry = lau_memalign_wired(512, sizeof(TSFiFontRegistry));
    tsfi_font_registry_init(g_registry);

    int start_arg = 1;
    if (argc > 1 && argv[1][0] == '/') {
        const char *font_path = argv[1];
        start_arg = 2;
        tsfi_io_printf(stdout, "[TYPEWRITER] Primary font: %s\n", font_path);
        if (tsfi_font_registry_add(g_registry, font_path)) {
            g_registry->primary_index = 0;
        } else {
            tsfi_io_printf(stderr, "Failed to load primary font %s\n", font_path);
        }
    }

    tsfi_input_init();
    g_fs = tsfi_font_init();
    g_quan = tsfi_quan_init();
    tsfi_quan_load_marchen(g_quan);
    
    VulkanSystem *s = create_vulkan_system();
    if (!s) return 1;
    s->external_render_enabled = true; 
    xdg_toplevel_set_title(s->xdg_toplevel, "TSFi DNA Typewriter [CTRL+W]");

    tsfi_input_set_key_hook(application_key_hook);
    tsfi_input_set_resize_hook(application_resize_hook);

    // Initial prompt or default text
    if (argc > start_arg) {
        for (int arg = start_arg; arg < argc; arg++) {
            const char *str = argv[arg];
            int i = 0;
            while (str[i] && g_text_len < MAX_TEXT) {
                uint32_t cp = 0;
                unsigned char c = str[i];
                if (c < 0x80) { cp = c; i += 1; }
                else if ((c & 0xE0) == 0xC0) { cp = ((c & 0x1F) << 6) | (str[i+1] & 0x3F); i += 2; }
                else if ((c & 0xF0) == 0xE0) { cp = ((c & 0x0F) << 12) | ((str[i+1] & 0x3F) << 6) | (str[i+2] & 0x3F); i += 3; }
                else if ((c & 0xF8) == 0xF0) { cp = ((c & 0x07) << 18) | ((str[i+1] & 0x3F) << 12) | ((str[i+2] & 0x3F) << 6) | (str[i+3] & 0x3F); i += 4; }
                else { i++; } 
                if (cp >= 32 || cp == '\n') {
                    g_text_buffer[g_text_len++] = cp;
                }
            }
            if (arg < argc - 1 && g_text_len < MAX_TEXT) g_text_buffer[g_text_len++] = ' ';
        }
    } else {
        // Default text: 0x11C70 (Marchen Head Letter Ka) + 0x11CB0 (Marchen Subjoined Letter Ka)
        g_text_buffer[g_text_len++] = 0x11CB0;
    }

    tsfi_io_printf(stdout, "[TYPEWRITER] Session Active. Overlaid Reference Display Enabled.\n");

    uint32_t *left_buf = (uint32_t*)lau_malloc(128 * 128 * 4);
    uint32_t *right_buf = (uint32_t*)lau_malloc(128 * 128 * 4);

    // Initial render to ensure window appears
    g_dirty = true;

    while (s->running && !g_force_quit) {
        if (g_dirty) {
            uint32_t *px = (uint32_t*)s->paint_buffer->data;
            int W = s->paint_buffer->width, H = s->paint_buffer->height;
            
            for (int i = 0; i < W * H; i++) px[i] = 0xFF000000;

            float pen_x = 50.0f, pen_y = 150.0f;
            float pt_size = 50.0f;
            uint32_t prev_cp = 0;
            float prev_pen_x = 0.0f;

            for (int i = 0; i < g_text_len; ) {
                uint32_t cluster[8];
                int cluster_count = 0;
                cluster[cluster_count++] = g_text_buffer[i++];
                while (i < g_text_len && cluster_count < 8 && is_combining_mark(g_text_buffer[i])) {
                    cluster[cluster_count++] = g_text_buffer[i++];
                }

                uint32_t cp = cluster[0];
                TSFiFontInfo *finfo = tsfi_font_registry_find_support(g_registry, cp);
                if (!finfo) tsfi_io_printf(stdout, "[TYPEWRITER] NO FONT SUPPORT for CP 0x%X\n", cp);
                stbtt_fontinfo stb;
                if (finfo) stb = init_local_stb(finfo);
                float scale = finfo ? stbtt_ScaleForPixelHeight(&stb, pt_size) : 0.0f;
                
                                if (cp == ' ') {
                                    if (g_registry->count > 0 && g_registry->primary_index >= 0) {
                                        int advance = 0, lsb = 0;
                                        // Use primary or first fallback for space
                                        TSFiFontInfo *sp_font = &g_registry->fonts[g_registry->primary_index];
                                        stbtt_fontinfo sp_stb = init_local_stb(sp_font);
                                        if (sp_stb.data) {
                                            stbtt_GetCodepointHMetrics(&sp_stb, ' ', &advance, &lsb);
                                            pen_x += (float)advance * stbtt_ScaleForPixelHeight(&sp_stb, pt_size);
                                        } else {
                                            pen_x += pt_size * 0.5f; // Fallback space width
                                        }
                                    } else {
                                        pen_x += pt_size * 0.5f; // Fallback space width
                                    }
                                    prev_cp = 0;
                                    continue;
                                }                if (cp == '\b') {
                    prev_cp = 0;
                    continue;
                }
                if (cp == '\n') {
                    pen_x = 50.0f;
                    pen_y += pt_size * 1.3f;
                    prev_cp = 0;
                    continue;
                }

                TSFiGlyphWavefront *wf = editor_get_cluster_dna(g_fs, cluster, cluster_count);
                
                // Visual Kerning Calculation
                if (prev_cp != 0 && wf && wf->dna) {
                    TSFiGlyphWavefront *prev_wf = editor_get_cluster_dna(g_fs, &prev_cp, 1);
                    if (prev_wf && prev_wf->dna) {
                        memset(left_buf, 0, 128 * 128 * 4);
                        memset(right_buf, 0, 128 * 128 * 4);
                        // Render both at origin (64, 64) in local buffers
                        tsfi_font_render_k0rn_pt(prev_wf->dna, left_buf, 128, 128, pt_size, 64.0f, 64.0f, 0xFFFFFFFF);
                        tsfi_font_render_k0rn_pt(wf->dna, right_buf, 128, 128, pt_size, 64.0f, 64.0f, 0xFFFFFFFF);
                        
                        float target_gap = pt_size * 0.10f; // 10% gap rule
                        float advance = tsfi_vision_calculate_glyph_spacing(left_buf, right_buf, 128, 128, target_gap);
                        // Override the mathematical pen_x with the visually perfect distance from the previous character
                        pen_x = prev_pen_x + advance;
                    }
                }

                if (!finfo) {
                    // Re-check after editor_get_cluster_dna might have dynamically loaded a fallback
                    finfo = tsfi_font_registry_find_support(g_registry, cp);
                    if (finfo) stb = init_local_stb(finfo);
                    scale = finfo ? stbtt_ScaleForPixelHeight(&stb, pt_size) : 0.0f;
                }

                // Optionally draw TrueType Reference Outlines directly from Geometry
                if (g_show_reference && finfo) {
                    float ref_pt_size = pt_size * 0.4f;
                    float ref_pen_x = pen_x - pt_size * 0.75f;
                    float ref_pen_y = pen_y;
                    
                    TSFiShapeCluster **ref_clusters = NULL;
                    int num_ref_clusters = tsfi_font_shape_text(finfo->ot, cluster, cluster_count, &ref_clusters);
                    if (num_ref_clusters > 0) {
                        TSFiShapeCluster *ref_cluster = ref_clusters[0];
                        float ref_cluster_pen_x = 0.0f;
                        
                        int ascent = finfo->ot->ascent;
                        int descent = finfo->ot->descent;
                        if (ascent - descent == 0) { ascent = 1000; descent = -1000; }
                        float ref_scale = ref_pt_size * (2.0f / (float)(ascent - descent));
                        
                        for (int s = 0; s < ref_cluster->num_glyphs; s++) {
                            uint32_t g_idx = ref_cluster->glyphs[s].glyph_index;
                            if (g_idx == 0) continue;

                            tsfi_glyph_geom_t geom = {0};
                            if (!tsfi_ottype_get_glyph_geom(finfo->ot, g_idx, &geom)) continue;

                            tsfi_transform_t t = ref_cluster->glyphs[s].transform;
                            
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

                                float vx_scaled_init = cur_x * ref_scale;
                                float vy_scaled_init = cur_y * ref_scale;
                                cur_x = (vx_scaled_init * t.m11 + vy_scaled_init * t.m21) + (t.dx * ref_scale);
                                cur_y = (vx_scaled_init * t.m12 + vy_scaled_init * t.m22) + (t.dy * ref_scale);
                                
                                for (int k = 0; k < count; k++) {
                                    int idx = start_pt + ((secret + k) % count);
                                    tsfi_point_t p = geom.points[idx];
                                    
                                    float vx_scaled = p.x * ref_scale;
                                    float vy_scaled = p.y * ref_scale;
                                    float transformed_x = (vx_scaled * t.m11 + vy_scaled * t.m21) + (t.dx * ref_scale);
                                    float transformed_y = (vx_scaled * t.m12 + vy_scaled * t.m22) + (t.dy * ref_scale);

                                    if (p.type == 1) { 
                                        float px0 = cur_x + ref_pen_x + ref_cluster_pen_x; 
                                        float py0 = ref_pen_y - cur_y; 
                                        float px1 = transformed_x + ref_pen_x + ref_cluster_pen_x; 
                                        float py1 = ref_pen_y - transformed_y;
                                        
                                        draw_line(px, W, H, px0, py0, px1, py1, 0xFFFFFFFF);
                                        cur_x = transformed_x; cur_y = transformed_y;
                                    } else if (p.type == 0) { 
                                        int next_idx = start_pt + ((secret + k + 1) % count);
                                        tsfi_point_t p_next = geom.points[next_idx];
                                        float x_end = p_next.x, y_end = p_next.y;
                                        if (p_next.type == 0) { 
                                            x_end = (p.x + p_next.x) * 0.5f; 
                                            y_end = (p.y + p_next.y) * 0.5f; 
                                        } else { k++; }

                                        float vx_scaled_end = x_end * ref_scale;
                                        float vy_scaled_end = y_end * ref_scale;
                                        float transformed_end_x = (vx_scaled_end * t.m11 + vy_scaled_end * t.m21) + (t.dx * ref_scale);
                                        float transformed_end_y = (vx_scaled_end * t.m12 + vy_scaled_end * t.m22) + (t.dy * ref_scale);

                                        float px0 = cur_x + ref_pen_x + ref_cluster_pen_x; 
                                        float py0 = ref_pen_y - cur_y;
                                        float px_ctrl = transformed_x + ref_pen_x + ref_cluster_pen_x; 
                                        float py_ctrl = ref_pen_y - transformed_y;
                                        float px1 = transformed_end_x + ref_pen_x + ref_cluster_pen_x; 
                                        float py1 = ref_pen_y - transformed_end_y;
                                        
                                        draw_quad_bezier(px, W, H, px0, py0, px_ctrl, py_ctrl, px1, py1, 0xFFFFFFFF);
                                        cur_x = transformed_end_x; cur_y = transformed_end_y;
                                    }
                                }
                                start_pt = end_pt + 1;
                            }
                            tsfi_ottype_free_geom(&geom);
                        }
                        tsfi_font_free_clusters(ref_clusters, num_ref_clusters);
                    }
                } else if (g_show_reference && !finfo) {
                    // Draw missing glyph box
                    for (int j=0; j<20; j++) {
                        px[(int)(pen_y - 20)*W + (int)(pen_x - pt_size * 0.75f) + j] = 0xFFFFFFFF;
                        px[(int)(pen_y)*W + (int)(pen_x - pt_size * 0.75f) + j] = 0xFFFFFFFF;
                    }
                }

                                if (wf && wf->dna) {
                                    prev_pen_x = pen_x;
                                    tsfi_font_render_k0rn_pt(wf->dna, px, W, H, pt_size, pen_x, pen_y, 0xFF00FFFF);
                
                                    int advance = 0, lsb = 0;
                                    if (stb.data) {
                                        stbtt_GetCodepointHMetrics(&stb, (int)cp, &advance, &lsb);
                                        pen_x += (float)advance * scale;
                                    } else {
                                        pen_x += pt_size * 0.6f;
                                    }
                
                                    prev_cp = cp;
                                } else {
                                    int advance = 0, lsb = 0;
                                    if (stb.data) {
                                        stbtt_GetCodepointHMetrics(&stb, (int)cp, &advance, &lsb);
                                        pen_x += (float)advance * scale;
                                    } else {
                                        pen_x += pt_size * 0.6f;
                                    }
                                    prev_cp = 0;
                                }                
                if (pen_x > W - 100) { pen_x = 50.0f; pen_y += pt_size * 1.22f; prev_cp = 0; }
            }

            // Draw cursor
            for (int i = 0; i < (int)(pt_size * 1.22f); i++) {
                int cx = (int)pen_x, cy = (int)(pen_y - pt_size * 0.875f) + i;
                if (cx >= 0 && cx < W && cy >= 0 && cy < H) px[cy * W + cx] = 0xFFFFFFFF;
            }

            // Draw status message
            if (g_status_timer > 0) {
                float sx = (float)W - 150.0f, sy = 40.0f;
                for (int i=0; g_status_msg[i]; i++) {
                    TSFiFontInfo *f_msg = tsfi_font_registry_find_support(g_registry, g_status_msg[i]);
                    if (f_msg) {
                        stbtt_fontinfo mstb = init_local_stb(f_msg);
                        if (mstb.data) {
                            draw_stb_shape(&mstb, g_status_msg[i], px, W, H, 20.0f, sx, sy, 0xFF00FF00);
                            int advance, lsb;
                            stbtt_GetCodepointHMetrics(&mstb, g_status_msg[i], &advance, &lsb);
                            sx += (float)advance * stbtt_ScaleForPixelHeight(&mstb, 20.0f);
                        }
                    }
                }
                g_status_timer--;
                g_dirty = (g_status_timer > 0); // Keep redrawing until timer expires
            }

            // Draw Quan Status
            if (g_quan->is_active) {
                float qx = 20.0f, qy = 40.0f;
                // Render [QUAN] prefix
                const char *prefix = "[QUAN] ";
                for(int j=0; prefix[j]; j++) {
                    TSFiFontInfo *f_pref = tsfi_font_registry_find_support(g_registry, prefix[j]);
                    if (f_pref) {
                        stbtt_fontinfo pstb = init_local_stb(f_pref);
                        if (pstb.data) {
                            draw_stb_shape(&pstb, prefix[j], px, W, H, 20.0f, qx, qy, 0xFF00FF00);
                            int advance, lsb;
                            stbtt_GetCodepointHMetrics(&pstb, prefix[j], &advance, &lsb);
                            qx += (float)advance * stbtt_ScaleForPixelHeight(&pstb, 20.0f);
                        }
                    }
                }
                // Render input buffer
                for(int j=0; j<g_quan->input_len; j++) {
                    TSFiFontInfo *f_buf = tsfi_font_registry_find_support(g_registry, g_quan->input_buffer[j]);
                    if (f_buf) {
                        stbtt_fontinfo bstb = init_local_stb(f_buf);
                        if (bstb.data) {
                            draw_stb_shape(&bstb, g_quan->input_buffer[j], px, W, H, 20.0f, qx, qy, 0xFFFFFFFF);
                            int advance, lsb;
                            stbtt_GetCodepointHMetrics(&bstb, g_quan->input_buffer[j], &advance, &lsb);
                            qx += (float)advance * stbtt_ScaleForPixelHeight(&bstb, 20.0f);
                        }
                    }
                }
                // Render candidates
                if (g_quan->current->candidate_count > 0) {
                    qx += 20.0f;
                    int start_idx = g_quan->candidate_page * 9;
                    int end_idx = start_idx + 9;
                    if (end_idx > g_quan->current->candidate_count) end_idx = g_quan->current->candidate_count;
                    
                    for (int j = start_idx; j < end_idx; j++) {
                        QuanCandidate *cand = &g_quan->current->candidates[j];
                        // Render candidate index
                        char idx_str[16]; snprintf(idx_str, sizeof(idx_str), "%d.", (j - start_idx) + 1);
                        for(int k=0; idx_str[k]; k++) {
                            TSFiFontInfo *f_idx = tsfi_font_registry_find_support(g_registry, idx_str[k]);
                            if (f_idx) {
                                stbtt_fontinfo istb = init_local_stb(f_idx);
                                if (istb.data) {
                                    draw_stb_shape(&istb, idx_str[k], px, W, H, 15.0f, qx, qy, 0xFF00FF00);
                                    int advance, lsb;
                                    stbtt_GetCodepointHMetrics(&istb, idx_str[k], &advance, &lsb);
                                    qx += (float)advance * stbtt_ScaleForPixelHeight(&istb, 15.0f);
                                }
                            }
                        }
                        // Render cluster
                        for (uint32_t k=0; k<cand->cluster_len; k++) {
                            TSFiFontInfo *f_cand = tsfi_font_registry_find_support(g_registry, cand->cluster[k]);
                            if (f_cand) {
                                stbtt_fontinfo cstb = init_local_stb(f_cand);
                                if (cstb.data) {
                                    draw_stb_shape(&cstb, cand->cluster[k], px, W, H, 25.0f, qx, qy, 0xFF00FFFF);
                                    int advance, lsb;
                                    stbtt_GetCodepointHMetrics(&cstb, (int)cand->cluster[k], &advance, &lsb);
                                    qx += (float)advance * stbtt_ScaleForPixelHeight(&cstb, 25.0f);
                                }
                            }
                        }
                        qx += 15.0f;
                    }
                }
            }

            draw_frame(s);
            g_dirty = false;
        }

        // Wait for next event
        if (wl_display_dispatch(s->display) == -1) break;
    }

    lau_free(left_buf);
    lau_free(right_buf);

    if (g_wf_cache) {
        for (int i = 0; i < g_wf_cache_count; i++) {
            if (g_wf_cache[i]) {
                if (g_wf_cache[i]->dna) {
                    lau_free(g_wf_cache[i]->dna->ops);
                    lau_free(g_wf_cache[i]->dna);
                }
                lau_free(g_wf_cache[i]);
            }
        }
        lau_free(g_wf_cache);
    }

    destroy_vulkan_system(s);
    return 0;
}
