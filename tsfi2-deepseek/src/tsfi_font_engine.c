#include "tsfi_font_vectors.h"
#include "tsfi_fourier.h"
#include "lau_memory.h"
#include "tsfi_logic.h"
#include "tsfi_wiring.h"
#include "tsfi_font_ai.h"
#include "tsfi_ottype.h"
#include "tsfi_io.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Zero-Copy Reservoir Sizes
#define RESERVOIR_REGISTRY_CAP 262144   // 256K Glyphs
#define RESERVOIR_SEGMENT_CAP  4194304  // 4M Segments (64MB)

TSFiFontSystem* tsfi_font_init() {
    extern void* lau_rebar_alloc_external(size_t size);
    TSFiFontSystem *fs = (TSFiFontSystem*)lau_malloc_wired(sizeof(TSFiFontSystem));
    if (!fs) return NULL;

    LauSystemHeader *h = (LauSystemHeader *)((char *)fs - offsetof(LauWiredHeader, payload));
    h->resonance_as_status = lau_strdup("FONT_INIT_ZC");
    lau_wire_system((WaveSystem*)fs, h, tsfi_get_default_logic());

    // Zhong-Preferred Reservoirs: enable zero-copy GPU access
    size_t map_sz = TSFI_FONT_MAP_SIZE * sizeof(uint32_t);
    fs->unicode_map = (uint32_t*)lau_rebar_alloc_external(map_sz);
    if (!fs->unicode_map) fs->unicode_map = (uint32_t*)lau_malloc(map_sz);
    memset(fs->unicode_map, 0, map_sz);

    fs->dna_indices = (uint32_t*)lau_rebar_alloc_external(map_sz);
    if (!fs->dna_indices) fs->dna_indices = (uint32_t*)lau_malloc(map_sz);
    memset(fs->dna_indices, 0, map_sz);

    fs->registry_capacity = RESERVOIR_REGISTRY_CAP;
    fs->registry_count = 1; 
    size_t reg_sz = fs->registry_capacity * sizeof(TSFiGlyphRegistryPacked);
    fs->registry = (TSFiGlyphRegistryPacked*)lau_rebar_alloc_external(reg_sz);
    if (!fs->registry) fs->registry = (TSFiGlyphRegistryPacked*)lau_malloc(reg_sz);
    memset(fs->registry, 0, sizeof(TSFiGlyphRegistryPacked)); 

    fs->segment_capacity = RESERVOIR_SEGMENT_CAP;
    fs->segment_count = 0;
    size_t seg_sz = fs->segment_capacity * sizeof(TSFiPackedSegment);
    fs->segments = (TSFiPackedSegment*)lau_rebar_alloc_external(seg_sz);
    if (!fs->segments) fs->segments = (TSFiPackedSegment*)lau_malloc(seg_sz);

    fs->spectrum_capacity = RESERVOIR_REGISTRY_CAP;
    size_t spec_sz = fs->spectrum_capacity * sizeof(TSFiFourierGlyph);
    fs->spectrum = lau_rebar_alloc_external(spec_sz);
    if (!fs->spectrum) fs->spectrum = lau_malloc(spec_sz);
    memset(fs->spectrum, 0, spec_sz);

    fs->wave_capacity = RESERVOIR_SEGMENT_CAP; 
    fs->wave_count = 1; 
    size_t wave_sz = fs->wave_capacity * sizeof(WAVE15);
    fs->wave_elements = (WAVE15*)lau_rebar_alloc_external(wave_sz);
    if (!fs->wave_elements) fs->wave_elements = (WAVE15*)lau_malloc(wave_sz);

    fs->dirty_map = true;
    fs->is_device_coherent = false;
    fs->current_style.thickness = 400.0f;
    fs->current_style.softness = 200.0f;
    
    tsfi_font_ai_init(fs);
    return fs;
}

static tsfi_ottype_t g_ot_ref;
static bool g_ot_loaded = false;

K0RnStream* tsfi_font_get_dna(TSFiFontSystem *fs, uint32_t codepoint) {
    if (codepoint >= TSFI_FONT_MAP_SIZE) return NULL;

    uint32_t dna_idx = fs->dna_indices[codepoint];
    if (dna_idx > 0) {
        // Find how many atoms (this is tricky with just an index)
        // Let's store count in a separate map or encoded in registry.
        // For now, we'll re-load from OT if we need to edit, but for rendering:
        // We'll return a view into the reservoir.
        K0RnStream *s = (K0RnStream*)lau_malloc(sizeof(K0RnStream));
        // We need the count! Let's use the registry's num_segments temporarily 
        // if we mirrored them, but DNA atoms != segments.
        // RE-DESIGN: DNA atoms are 15-bytes. Let's use the registry's num_segments 
        // to mean "num_dna_atoms" when accessing Buffer E.
        uint32_t rid = fs->unicode_map[codepoint];
        if (rid == 0) { free(s); return NULL; }
        s->op_count = fs->registry[rid].num_segments;
        s->ops = (K0RnOp*)lau_malloc(s->op_count * sizeof(K0RnOp));
        // Decompress WAVE15 to K0RnOp for the host-side return
        for(uint32_t i=0; i<s->op_count; i++) {
            WAVE15 *atom = &fs->wave_elements[dna_idx + i];
            s->ops[i].x = (float)atom->p0_x / 16384.0f;
            s->ops[i].y = (float)atom->p0_y / 16384.0f;
            s->ops[i].z = (float)atom->p1_x / 16384.0f;
            s->ops[i].w = (float)atom->p1_y / 16384.0f;
            s->ops[i].type = atom->flags;
            s->ops[i].p1 = 0.05f;
        }
        return s;
    }

    // Lazy load from system font
    if (!g_ot_loaded) {
        size_t sz;
        void* data = tsfi_io_map_file("/usr/share/fonts/opentype/stix/STIXTwoText-Regular.otf", &sz);
        if (data && tsfi_ottype_init(&g_ot_ref, (uint8_t*)data, sz, 0)) {
            g_ot_loaded = true;
        } else return NULL;
    }

    uint32_t g_idx = tsfi_ottype_get_glyph_index(&g_ot_ref, codepoint, 0);
    if (g_idx == 0) return NULL;

    tsfi_glyph_geom_t geom = {0};
    if (!tsfi_ottype_get_glyph_geom(&g_ot_ref, g_idx, &geom)) return NULL;

    K0RnStream *dna = tsfi_ottype_compile_dna(&geom);
    if (!dna) { tsfi_ottype_free_geom(&geom); return NULL; }

    // Store in Reservoir
    uint32_t start_idx = (uint32_t)fs->wave_count;
    if (start_idx + dna->op_count > fs->wave_capacity) return dna; // Return unbuffered

    for (uint32_t i = 0; i < dna->op_count; i++) {
        K0RnOp *op = &dna->ops[i];
        fs->wave_elements[start_idx + i] = (WAVE15){
            .p0_x = (int16_t)(op->x * 16384.0f),
            .p0_y = (int16_t)(op->y * 16384.0f),
            .p1_x = (int16_t)(op->z * 16384.0f),
            .p1_y = (int16_t)(op->w * 16384.0f),
            .flags = (uint16_t)op->type,
            .secret = 0x42
        };
    }
    
    fs->dna_indices[codepoint] = start_idx;
    fs->wave_count += dna->op_count;

    // Update registry so renderers know it exists
    uint32_t rid = fs->unicode_map[codepoint];
    if (rid == 0) {
        rid = (uint32_t)fs->registry_count++;
        fs->unicode_map[codepoint] = rid;
    }
    fs->registry[rid].start_index = start_idx;
    fs->registry[rid].num_segments = (uint16_t)dna->op_count;

    tsfi_ottype_free_geom(&geom);
    return dna;
}

void tsfi_font_save_prime(TSFiFontSystem *fs, uint32_t codepoint) {
    K0RnStream *dna = tsfi_font_get_dna(fs, codepoint);
    if (dna) {
        char name[32]; snprintf(name, 31, "0x%X", codepoint);
        tsfi_k0rn_save(name, dna);
        lau_free(dna->ops);
        lau_free(dna);
    }
}

void tsfi_font_set_coherency(TSFiFontSystem *fs, bool device_coherent) {
    if (fs) fs->is_device_coherent = device_coherent;
}

void tsfi_font_set_style(TSFiFontSystem *fs, float thick, float soft, int16_t sx, int16_t sy, float sa) {
    if (!fs) return;
    fs->current_style.thickness = thick;
    fs->current_style.softness = soft;
    fs->current_style.shadow_secret_x = sx;
    fs->current_style.shadow_secret_y = sy;
    fs->current_style.shadow_alpha = sa;
}

void tsfi_font_flush_ranges(TSFiFontSystem *fs) {
    if (!fs) return;
    if (fs->is_device_coherent) return; 
    fs->dirty_map = false;
}

void tsfi_font_destroy(TSFiFontSystem *fs) {
    if (!fs) return;
    
    LauSystemHeader *h = (LauSystemHeader *)((char *)fs - offsetof(LauWiredHeader, payload));
    if (h->resonance_as_status) { lau_free(h->resonance_as_status); h->resonance_as_status = NULL; }
    
    tsfi_font_ai_destroy(fs); 
    if (fs->unicode_map) lau_free(fs->unicode_map);
    if (fs->dna_indices) lau_free(fs->dna_indices);
    if (fs->registry) lau_free(fs->registry);
    if (fs->segments) lau_free(fs->segments);
    if (fs->spectrum) lau_free(fs->spectrum);
    if (fs->wave_elements) lau_free(fs->wave_elements);
    
    lau_unseal_object(fs);
    lau_free(fs);
}

// Zero-Copy Update: No reallocation logic needed for standard workloads.
// If capacity is exceeded, we simply fail (or crash in debug). 
// This forces predictable memory usage.

void tsfi_font_set_glyph(TSFiFontSystem *fs, uint32_t codepoint, 
                         TSFiPackedSegment *segs, size_t count, 
                         int16_t advance, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
    if (codepoint >= TSFI_FONT_MAP_SIZE) return;

    // Safety check against Reservoir overflow
    if (fs->segment_count + count > fs->segment_capacity) {
        tsfi_io_printf(stderr, "[TSFI_FONT] FATAL: Segment Reservoir Overflow! Need > %zu\n", fs->segment_capacity);
        return; 
    }
    if (fs->registry_count + 1 > fs->registry_capacity) {
        tsfi_io_printf(stderr, "[TSFI_FONT] FATAL: Registry Reservoir Overflow!\n");
        return;
    }

    uint32_t existing_id = fs->unicode_map[codepoint];
    uint32_t target_id = existing_id;

    if (target_id == 0) {
        target_id = fs->registry_count++;
        fs->unicode_map[codepoint] = target_id;
        fs->dirty_map = true;
    }

    uint32_t start_seg = fs->segment_count;
    
    // Copy segments into the reservoir (Linear Allocation)
    memcpy(&fs->segments[start_seg], segs, count * sizeof(TSFiPackedSegment));
    fs->segment_count += count;

    // Update Registry (Direct Access)
    TSFiGlyphRegistryPacked *reg = &fs->registry[target_id];
    reg->start_index = start_seg;
    reg->num_segments = (uint16_t)count;
    reg->advance = advance;
    reg->min_x = x1; reg->min_y = y1;
    reg->max_x = x2; reg->max_y = y2;
}

void add_line(TSFiPackedSegment *buf, int *idx, int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    buf[*idx].p0_x = x0; buf[*idx].p0_y = y0;
    buf[*idx].p1_x = 0;  buf[*idx].p1_y = 0; 
    buf[*idx].p2_x = x1; buf[*idx].p2_y = y1;
    buf[*idx].flags = 3; 
    (*idx)++;
}

void tsfi_font_generate_default(TSFiFontSystem *fs) {
    tsfi_io_printf(stdout, "[TSFI_FONT] Generating Default Procedural Font (Zero-Copy Reservoir)...\n");
    TSFiPackedSegment temp_segs[16];
    int16_t w = 12000, h = 24000;
    int16_t l = -w/2, r = w/2, t = h/2, b = -h/2, m = 0, my = 0;

    for (uint32_t c = 32; c < 127; c++) {
        int idx = 0;
        if (c == '0') {
            add_line(temp_segs, &idx, l, t, r, t);
            add_line(temp_segs, &idx, r, t, r, b);
            add_line(temp_segs, &idx, r, b, l, b);
            add_line(temp_segs, &idx, l, b, l, t);
            add_line(temp_segs, &idx, l, b, r, t); 
        }
        else if (c == '1') add_line(temp_segs, &idx, m, t, m, b);
        else if (c == 'A') {
            add_line(temp_segs, &idx, l, b, m, t);
            add_line(temp_segs, &idx, m, t, r, b);
            add_line(temp_segs, &idx, l/2, my, r/2, my);
        }
        else {
             add_line(temp_segs, &idx, l, t, r, t); 
             add_line(temp_segs, &idx, r, t, r, b); 
             add_line(temp_segs, &idx, r, b, l, b); 
             add_line(temp_segs, &idx, l, b, l, t); 
        }
        tsfi_font_set_glyph(fs, c, temp_segs, idx, w + 2000, l, b, r, t);
    }
    tsfi_io_printf(stdout, "[TSFI_FONT] Default Font Generated.\n");
}
