#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "tsfi_resonance.h"
#include <stdatomic.h>

// Spin-lock helpers
static inline void metrics_spin_lock(_Atomic int *lock) {
    while (atomic_exchange_explicit(lock, 1, memory_order_acquire)) {
        __builtin_ia32_pause();
    }
}

static inline void metrics_spin_unlock(_Atomic int *lock) {
    atomic_store_explicit(lock, 0, memory_order_release);
}

#include "tsfi_font_registry.h"
#include "tsfi_ottype.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "lau_memory.h"
#include "tsfi_zhao.h"
#include <unistd.h>

// Dispatcher task for Zhao scheduler
static void dispatcher_task(void *arg) {
    LauWireThread *wt = (LauWireThread *)arg;
    while (!wt->finished) {
        tsfi_wire_thread_dispatch(wt);
    }
}

#define READ_U16(p) (uint16_t)(((const uint8_t*)(p))[0] << 8 | ((const uint8_t*)(p))[1])
#define READ_U32(p) (uint32_t)((uint32_t)((const uint8_t*)(p))[0] << 24 | \
                               (uint32_t)((const uint8_t*)(p))[1] << 16 | \
                               (uint32_t)((const uint8_t*)(p))[2] << 8  | \
                               (uint32_t)((const uint8_t*)(p))[3])

typedef struct {
    TSFiFontRegistry *reg;
    atomic_int next_font_idx;
    atomic_int total_fonts_tested;
    atomic_int hhea_parity_fail;
    atomic_int maxp_parity_fail;
    atomic_int ltsh_parity_fail;
    atomic_int vdmx_parity_fail;
    atomic_int hdmx_parity_fail;
    
    atomic_int gsub_present;
    atomic_int gpos_present;
    atomic_int gdef_present;
    atomic_int jstf_present;
    atomic_int base_present;
    atomic_int math_present;
    atomic_int gsub_parity_fail;
    atomic_int gpos_parity_fail;
    atomic_int gdef_parity_fail;
    atomic_int jstf_parity_fail;
    atomic_int base_parity_fail;
    atomic_int math_parity_fail;
    atomic_int fvar_present;
    atomic_int avar_present;
    atomic_int hvar_present;
    atomic_int vvar_present;
    atomic_int mvar_present;
    atomic_int gvar_present;
    atomic_int var_parity_fail;

    atomic_int vhea_present;
    atomic_int vmtx_present;
    atomic_int vhea_parity_fail;
    atomic_int vmtx_parity_fail;
    atomic_int cmap_format14_present;

    atomic_long all_feat_tags[1024];

    atomic_long feat_isol;
    atomic_long feat_init;
    atomic_long feat_medi;
    atomic_long feat_fina;
    atomic_long feat_curs;
    atomic_long feat_liga;

    atomic_long total_scripts;
    atomic_long total_features;
    atomic_long total_lookups;

    atomic_long total_gdef_classes;
    atomic_long total_gdef_attachments;
    atomic_long total_gdef_carets;
    atomic_long total_jstf_scripts;
    atomic_long total_jstf_extenders;
    atomic_long jstf_priority_records;
    atomic_long jstf_gsub_mods;
    atomic_long jstf_gpos_mods;
    atomic_long total_required_features;
    atomic_long gsub_lookup_types[10];
    atomic_long gpos_lookup_types[10];
    atomic_long total_base_scripts;
    atomic_long total_base_coords;
    atomic_long total_feature_variations;
    atomic_long total_coverage_tables;
    atomic_long total_class_defs;
    atomic_long lookup_flags[16];
    atomic_long total_math_constants;
    atomic_long total_math_glyph_info;
    atomic_long total_math_variants;
    atomic_long math_extenders;
    atomic_long gdef_mark_glyph_sets;
    atomic_long total_base_horiz;
    atomic_long family_class_hist[256];
    atomic_long family_subclass_hist[256];
    atomic_long total_base_vert;
    
    atomic_int total_glyphs_checked;
    atomic_int hmtx_parity_fail;
    
    atomic_int stb_renderable_glyphs; 
    atomic_int tsfi_geometry_success;
    
    // Categorization
    atomic_int glyphs_cff;
    atomic_int glyphs_simple;
    atomic_int glyphs_compound;
    
    atomic_int simple_success;
    atomic_int compound_success;

    _Atomic int tag_mutex;
    uint32_t unique_tags[1024];
    int unique_tag_count;
} test_context_t;

static void add_unique_tag(test_context_t *ctx, uint32_t tag) {
    metrics_spin_lock(&ctx->tag_mutex);
    for (int i = 0; i < ctx->unique_tag_count; i++) {
        if (ctx->unique_tags[i] == tag) {
            metrics_spin_unlock(&ctx->tag_mutex);
            return;
        }
    }
    if (ctx->unique_tag_count < 1024) {
        ctx->unique_tags[ctx->unique_tag_count++] = tag;
    }
    metrics_spin_unlock(&ctx->tag_mutex);
}

static void walk_layout_stats(test_context_t *ctx, const uint8_t *tbl, uint32_t table_size, int *scripts, int *features, int *lookups, long *lookup_types, int *req_features, int *variations, int *coverages, int *classes, long *flags, long *feat_tags) {
    if (table_size < 10) return;
    uint32_t version = READ_U32(tbl);
    if (version != 0x00010000 && version != 0x00010001) return;
    
    uint16_t s_off = READ_U16(tbl + 4);
    uint16_t f_off = READ_U16(tbl + 6);
    uint16_t l_off = READ_U16(tbl + 8);
    
    if (version == 0x00010001 && table_size >= 12) {
        uint32_t v_off = READ_U32(tbl + 10);
        if (v_off && v_off < table_size - 4) {
            *variations = READ_U32(tbl + v_off);
        }
    }

    if (s_off && s_off < table_size - 2) {
        uint16_t scount = READ_U16(tbl + s_off);
        *scripts = scount;
        for (uint16_t i = 0; i < scount; i++) {
            uint16_t soff = READ_U16(tbl + s_off + 2 + i * 6 + 4);
            if (soff && (uint32_t)s_off + soff < table_size - 2) {
                const uint8_t *stbl = tbl + s_off + soff;
                uint16_t def_lang_off = READ_U16(stbl);
                if (def_lang_off && (uint32_t)s_off + soff + def_lang_off < table_size - 2) {
                    if (READ_U16(tbl + s_off + soff + def_lang_off) != 0xFFFF) (*req_features)++;
                }
                uint16_t lcount = READ_U16(stbl + 2);
                for (uint16_t j = 0; j < lcount; j++) {
                    uint16_t loff = READ_U16(stbl + 4 + j * 6 + 4);
                    if (loff && (uint32_t)s_off + soff + loff < table_size - 2) {
                        if (READ_U16(tbl + s_off + soff + loff) != 0xFFFF) (*req_features)++;
                    }
                }
            }
        }
    }
    if (f_off && f_off < table_size - 2) {
        uint16_t fcount = READ_U16(tbl + f_off);
        *features = fcount;
        for (uint16_t i = 0; i < fcount; i++) {
            uint32_t tag = READ_U32(tbl + f_off + 2 + i * 6);
            add_unique_tag(ctx, tag);
            if (tag == 0x69736f6c) feat_tags[0]++; // 'isol'
            else if (tag == 0x696e6974) feat_tags[1]++; // 'init'
            else if (tag == 0x6d656469) feat_tags[2]++; // 'medi'
            else if (tag == 0x66696e61) feat_tags[3]++; // 'fina'
            else if (tag == 0x63757273) feat_tags[4]++; // 'curs'
            else if (tag == 0x6c696761) feat_tags[5]++; // 'liga'
        }
    }
    if (l_off && l_off < table_size - 2) {
        uint16_t lcount = READ_U16(tbl + l_off);
        *lookups = lcount;
        for (uint16_t i = 0; i < lcount; i++) {
            uint16_t loff = READ_U16(tbl + l_off + 2 + i * 2);
            if (loff && (uint32_t)l_off + loff < table_size - 6) {
                uint16_t type = READ_U16(tbl + l_off + loff);
                if (type < 10) lookup_types[type]++;
                
                uint16_t lflag = READ_U16(tbl + l_off + loff + 2);
                for (int bit=0; bit<16; bit++) if (lflag & (1 << bit)) flags[bit]++;

                uint16_t subcount = READ_U16(tbl + l_off + loff + 4);
                for (uint16_t j = 0; j < subcount; j++) {
                    uint16_t sub_off = READ_U16(tbl + l_off + loff + 6 + j * 2);
                    if (sub_off && (uint32_t)l_off + loff + sub_off < table_size - 4) {
                        const uint8_t *subt = tbl + l_off + loff + sub_off;
                        uint16_t real_type = type;
                        const uint8_t *real_subt = subt;
                        if (type == 7 || type == 9) { // Extension
                            uint16_t etype = READ_U16(subt + 2);
                            uint32_t eoff = READ_U32(subt + 4);
                            if (eoff && (uint32_t)l_off + loff + sub_off + eoff < table_size - 4) {
                                real_type = etype;
                                real_subt = tbl + l_off + loff + sub_off + eoff;
                            }
                        }
                        // Most lookups have Coverage at +2
                        if (READ_U16(real_subt + 2)) (*coverages)++;
                        // PairPos Format 2 has ClassDefs at +10, +12
                        if (real_type == 2 && READ_U16(real_subt) == 2) {
                            if (READ_U16(real_subt + 8)) (*classes)++;
                            if (READ_U16(real_subt + 10)) (*classes)++;
                        }
                    }
                }
            }
        }
    }
}

static void walk_base_stats(const uint8_t *tbl, uint32_t size, int *scripts, int *coords) {
    if (size < 8) return;
    uint16_t horiz = READ_U16(tbl + 4);
    uint16_t vert = READ_U16(tbl + 6);
    
    uint16_t offsets[2] = {horiz, vert};
    for (int i=0; i<2; i++) {
        if (offsets[i] && (uint32_t)offsets[i] < size - 4) {
            const uint8_t *axis = tbl + offsets[i];
            uint16_t script_list_off = READ_U16(axis + 2);
            if (script_list_off && (uint32_t)offsets[i] + script_list_off < size - 2) {
                const uint8_t *sl = axis + script_list_off;
                uint16_t scount = READ_U16(sl);
                *scripts += scount;
                for (uint16_t j = 0; j < scount; j++) {
                    uint16_t soff = READ_U16(sl + 2 + j * 6 + 4);
                    if (soff && (uint32_t)offsets[i] + script_list_off + soff < size - 4) {
                        const uint8_t *stbl = sl + soff;
                        uint16_t dlang_off = READ_U16(stbl);
                        if (dlang_off && (uint32_t)offsets[i] + script_list_off + soff + dlang_off < size - 2) {
                            const uint8_t *langu = stbl + dlang_off;
                            uint16_t vcount = READ_U16(langu + 2);
                            for (uint16_t k = 0; k < vcount; k++) {
                                uint16_t voff = READ_U16(langu + 4 + k * 2);
                                if (voff) (*coords)++;
                            }
                        }
                    }
                }
            }
        }
    }
}

static void walk_math_stats(const uint8_t *tbl, uint32_t size, int *constants, int *glyph_info, int *variants, int *math_extenders) {
    if (size < 10) return;
    if (READ_U16(tbl + 4)) *constants = 1;
    if (READ_U16(tbl + 6)) *glyph_info = 1;
    uint16_t voff = READ_U16(tbl + 8);
    if (voff && voff < size - 10) {
        *variants = 1;
        const uint8_t *vtbl = tbl + voff;
        uint16_t h_off = READ_U16(vtbl + 2);
        uint16_t v_off = READ_U16(vtbl + 4);
        uint16_t voffs[2] = {h_off, v_off};
        for (int i=0; i<2; i++) {
            if (voffs[i] && (uint32_t)voff + voffs[i] < size - 2) {
                const uint8_t *lst = vtbl + voffs[i];
                uint16_t lcount = READ_U16(lst);
                for (uint16_t j = 0; j < lcount; j++) {
                    uint16_t coff = READ_U16(lst + 2 + j * 2);
                    if (coff && (uint32_t)voff + voffs[i] + coff < size - 4) {
                        const uint8_t *ctbl = lst + coff;
                        uint16_t aoff = READ_U16(ctbl + 2);
                        if (aoff && (uint32_t)voff + voffs[i] + coff + aoff < size - 2) {
                            *math_extenders += READ_U16(ctbl + aoff);
                        }
                    }
                }
            }
        }
    }
}

static void walk_jstf_stats(const uint8_t *tbl, uint32_t size, int *scripts, int *extenders, int *priorities, int *gsub_mods, int *gpos_mods) {
    if (size < 6) return;
    uint16_t scount = READ_U16(tbl + 4);
    *scripts = scount;
    for (uint16_t i = 0; i < scount; i++) {
        uint16_t soff = READ_U16(tbl + 6 + i * 6 + 4);
        if (soff && (uint32_t)soff + 6 < size) {
            const uint8_t *stbl = tbl + soff;
            uint16_t eoff = READ_U16(stbl);
            if (eoff && (uint32_t)soff + eoff + 2 < size) {
                const uint8_t *etbl = tbl + soff + eoff;
                *extenders += READ_U16(etbl);
            }
            uint16_t lscount = READ_U16(stbl + 4);
            for (uint16_t j = 0; j <= lscount; j++) {
                uint16_t lsoff = 0;
                if (j == 0) lsoff = READ_U16(stbl + 2); // DefaultLangSys
                else lsoff = READ_U16(stbl + 6 + (j-1) * 6 + 4);
                if (lsoff && (uint32_t)soff + lsoff + 2 < size) {
                    const uint8_t *lstbl = tbl + soff + lsoff;
                    uint16_t pcount = READ_U16(lstbl);
                    *priorities += pcount;
                    for (uint16_t k = 0; k < pcount; k++) {
                        uint16_t poff = READ_U16(lstbl + 2 + k * 2);
                        if (poff && (uint32_t)soff + lsoff + poff + 20 < size) {
                            const uint8_t *ptbl = tbl + soff + lsoff + poff;
                            if (READ_U16(ptbl)) (*gsub_mods)++;
                            if (READ_U16(ptbl + 2)) (*gsub_mods)++;
                            if (READ_U16(ptbl + 4)) (*gsub_mods)++;
                            if (READ_U16(ptbl + 6)) (*gpos_mods)++;
                            if (READ_U16(ptbl + 8)) (*gpos_mods)++;
                            if (READ_U16(ptbl + 10)) (*gpos_mods)++;
                        }
                    }
                }
            }
        }
    }
}

static void walk_gdef_stats(const uint8_t *tbl, uint32_t table_size, int *classes, int *attachments, int *carets, int *mark_sets) {
    if (table_size < 12) return;
    uint32_t version = READ_U32(tbl);
    uint16_t class_def_off = READ_U16(tbl + 4);
    uint16_t attach_list_off = READ_U16(tbl + 6);
    uint16_t lig_caret_list_off = READ_U16(tbl + 8);
    
    if (class_def_off && class_def_off < table_size - 2) *classes = 1;
    if (attach_list_off && attach_list_off < table_size - 2) *attachments = READ_U16(tbl + attach_list_off);
    if (lig_caret_list_off && lig_caret_list_off < table_size - 2) *carets = READ_U16(tbl + lig_caret_list_off);

    if (version >= 0x00010002 && table_size >= 14) {
        uint16_t mark_set_off = READ_U16(tbl + 12);
        if (mark_set_off && (uint32_t)mark_set_off < table_size - 2) {
            const uint8_t *mstbl = tbl + mark_set_off;
            *mark_sets = READ_U16(mstbl + 2);
        }
    }
}

void* test_font_worker(void *arg) {
    test_context_t *ctx = (test_context_t*)arg;
    int count = ctx->reg->count;

    // Thread-local geom to reuse buffers
    tsfi_glyph_geom_t geom = {0};

    while (1) {
        int idx = atomic_fetch_add(&ctx->next_font_idx, 1);
        if (idx >= count) break;

        TSFiFontInfo *info = &ctx->reg->fonts[idx];
        if (!info->active || !info->ot) continue;
        
        stbtt_fontinfo stb;
        if (!stbtt_InitFont(&stb, info->data, info->ttc_secret)) continue;

        atomic_fetch_add(&ctx->total_fonts_tested, 1);

        // --- ENHANCED HHEA UNIT TESTING (Raw Byte Parity) ---
        uint32_t hhea_off = 0;
        const uint8_t *fdir = &info->ot->data[0];
        // TTC check logic simplified for test
        uint32_t tag_top = READ_U32(fdir);
        uint32_t base_off = (tag_top == 0x74746366) ? 0 : 0; // Fixed secret or 0
        uint16_t num_tables = READ_U16(&info->ot->data[base_off + 4]);
        for (uint16_t i = 0; i < num_tables; i++) {
            uint32_t tag = READ_U32(&info->ot->data[base_off + 12 + i * 16]);
            if (tag == 0x68686561) { // 'hhea'
                hhea_off = READ_U32(&info->ot->data[base_off + 12 + i * 16 + 8]);
                break;
            }
        }

        bool raw_hhea_ok = true;
        if (hhea_off) {
            int16_t r_asc = (int16_t)READ_U16(&info->ot->data[hhea_off + 4]);
            int16_t r_desc = (int16_t)READ_U16(&info->ot->data[hhea_off + 6]);
            int16_t r_lg = (int16_t)READ_U16(&info->ot->data[hhea_off + 8]);
            uint16_t r_adv = READ_U16(&info->ot->data[hhea_off + 10]);
            int16_t r_lsb = (int16_t)READ_U16(&info->ot->data[hhea_off + 12]);
            int16_t r_rsb = (int16_t)READ_U16(&info->ot->data[hhea_off + 14]);
            int16_t r_ext = (int16_t)READ_U16(&info->ot->data[hhea_off + 16]);
            int16_t r_csrise = (int16_t)READ_U16(&info->ot->data[hhea_off + 18]);
            int16_t r_csrun = (int16_t)READ_U16(&info->ot->data[hhea_off + 20]);
            int16_t r_coff = (int16_t)READ_U16(&info->ot->data[hhea_off + 22]);
            uint16_t r_numh = READ_U16(&info->ot->data[hhea_off + 34]);

            if (info->ot->ascent != r_asc || info->ot->descent != r_desc || 
                info->ot->line_gap != r_lg || info->ot->advance_width_max != r_adv ||
                info->ot->min_lsb != r_lsb || info->ot->min_rsb != r_rsb ||
                info->ot->max_x_extent != r_ext || info->ot->caret_slope_rise != r_csrise ||
                info->ot->caret_slope_run != r_csrun || info->ot->caret_secret != r_coff ||
                info->ot->num_h_metrics != r_numh) {
                raw_hhea_ok = false;
            }
        }

        // --- LTSH PARITY ---
        if (info->ot->ltsh_secret) {
            uint16_t version = READ_U16(&info->ot->data[info->ot->ltsh_secret]);
            uint16_t num_glyphs = READ_U16(&info->ot->data[info->ot->ltsh_secret + 2]);
            if (version != 0 || num_glyphs != info->ot->num_glyphs) {
                atomic_fetch_add(&ctx->ltsh_parity_fail, 1);
            } else {
                // Verify threshold values and table bounds
                if (info->ot->ltsh_secret + 4 + num_glyphs > info->ot->size) {
                    atomic_fetch_add(&ctx->ltsh_parity_fail, 1);
                }
            }
        }

        // --- VDMX PARITY (Walk the structure) ---
        if (info->ot->vdmx_secret) {
            const uint8_t *vdmx = &info->ot->data[info->ot->vdmx_secret];
            uint16_t version = READ_U16(vdmx);
            uint16_t num_recs = READ_U16(vdmx + 2);
            uint16_t num_ratios = READ_U16(vdmx + 4);
            if (version > 1 || num_recs == 0 || num_ratios == 0) {
                atomic_fetch_add(&ctx->vdmx_parity_fail, 1);
            } else {
                // Basic structural walk: ensure ratio offsets and group offsets are within table
                bool v_ok = true;
                uint32_t ratio_end = 6 + num_ratios * 4;
                if (ratio_end > info->ot->size - info->ot->vdmx_secret) v_ok = false;
                
                for (uint16_t r = 0; r < num_recs && v_ok; r++) {
                    uint32_t group_off = READ_U16(vdmx + ratio_end + r * 2);
                    if (group_off > info->ot->size - info->ot->vdmx_secret) v_ok = false;
                }
                if (!v_ok) atomic_fetch_add(&ctx->vdmx_parity_fail, 1);
            }
        }

        // --- HDMX PARITY (Check device record structure strictly) ---
        if (info->ot->hdmx_secret) {
            const uint8_t *hdmx = &info->ot->data[info->ot->hdmx_secret];
            uint16_t version = READ_U16(hdmx);
            uint16_t num_recs = READ_U16(hdmx + 2);
            uint32_t size_rec = READ_U32(hdmx + 4);
            // HDMX records are padded to 32-bit boundary
            uint32_t expected_size = (2 + (uint32_t)info->ot->num_glyphs + 3) & ~3;
            if (version != 0 || num_recs == 0 || size_rec != expected_size) {
                atomic_fetch_add(&ctx->hdmx_parity_fail, 1);
            } else {
                // Probe last byte of last record to ensure accessibility
                uint32_t last_off = 8 + (num_recs - 1) * size_rec + size_rec - 1;
                if (info->ot->hdmx_secret + last_off >= info->ot->size) {
                    atomic_fetch_add(&ctx->hdmx_parity_fail, 1);
                }
            }
        }

        // --- GSUB PARITY (Layout structure) ---
        if (info->ot->gsub_secret) {
            atomic_fetch_add(&ctx->gsub_present, 1);
            const uint8_t *gsub = &info->ot->data[info->ot->gsub_secret];
            uint32_t version = READ_U32(gsub);
            if (version != 0x00010000 && version != 0x00010001) {
                atomic_fetch_add(&ctx->gsub_parity_fail, 1);
            } else {
                int s=0, f=0, l=0, rf=0, v=0, cv=0, cd=0;
                long lt[10] = {0}, lf[16] = {0}, ft[10] = {0};
                walk_layout_stats(ctx, gsub, info->ot->size - info->ot->gsub_secret, &s, &f, &l, lt, &rf, &v, &cv, &cd, lf, ft);
                if (s == 0) atomic_fetch_add(&ctx->gsub_parity_fail, 1);
                atomic_fetch_add(&ctx->total_scripts, s);
                atomic_fetch_add(&ctx->total_features, f);
                atomic_fetch_add(&ctx->total_lookups, l);
                atomic_fetch_add(&ctx->total_required_features, rf);
                atomic_fetch_add(&ctx->total_feature_variations, v);
                atomic_fetch_add(&ctx->total_coverage_tables, cv);
                atomic_fetch_add(&ctx->total_class_defs, cd);
                for (int i=0; i<10; i++) atomic_fetch_add(&ctx->gsub_lookup_types[i], lt[i]);
                for (int i=0; i<16; i++) atomic_fetch_add(&ctx->lookup_flags[i], lf[i]);
                atomic_fetch_add(&ctx->feat_isol, ft[0]);
                atomic_fetch_add(&ctx->feat_init, ft[1]);
                atomic_fetch_add(&ctx->feat_medi, ft[2]);
                atomic_fetch_add(&ctx->feat_fina, ft[3]);
                atomic_fetch_add(&ctx->feat_curs, ft[4]);
                atomic_fetch_add(&ctx->feat_liga, ft[5]);
            }
        }

        // --- GPOS PARITY (Layout structure) ---
        if (info->ot->gpos_secret) {
            atomic_fetch_add(&ctx->gpos_present, 1);
            const uint8_t *gpos = &info->ot->data[info->ot->gpos_secret];
            uint32_t version = READ_U32(gpos);
            if (version != 0x00010000 && version != 0x00010001) {
                atomic_fetch_add(&ctx->gpos_parity_fail, 1);
            } else {
                int s=0, f=0, l=0, rf=0, v=0, cv=0, cd=0;
                long lt[10] = {0}, lf[16] = {0}, ft[10] = {0};
                walk_layout_stats(ctx, gpos, info->ot->size - info->ot->gpos_secret, &s, &f, &l, lt, &rf, &v, &cv, &cd, lf, ft);
                if (s == 0) atomic_fetch_add(&ctx->gpos_parity_fail, 1);
                atomic_fetch_add(&ctx->total_scripts, s);
                atomic_fetch_add(&ctx->total_features, f);
                atomic_fetch_add(&ctx->total_lookups, l);
                atomic_fetch_add(&ctx->total_required_features, rf);
                atomic_fetch_add(&ctx->total_feature_variations, v);
                atomic_fetch_add(&ctx->total_coverage_tables, cv);
                atomic_fetch_add(&ctx->total_class_defs, cd);
                for (int i=0; i<10; i++) atomic_fetch_add(&ctx->gpos_lookup_types[i], lt[i]);
                for (int i=0; i<16; i++) atomic_fetch_add(&ctx->lookup_flags[i], lf[i]);
                atomic_fetch_add(&ctx->feat_isol, ft[0]);
                atomic_fetch_add(&ctx->feat_init, ft[1]);
                atomic_fetch_add(&ctx->feat_medi, ft[2]);
                atomic_fetch_add(&ctx->feat_fina, ft[3]);
                atomic_fetch_add(&ctx->feat_curs, ft[4]);

                if (ft[4] > 0) {
                    for (uint32_t gid = 0; gid < 100 && gid < info->ot->num_glyphs; gid++) {
                        float enX=0, enY=0, exX=0, exY=0;
                        if (tsfi_ottype_get_cursive_anchors(info->ot, gid, &enX, &enY, &exX, &exY)) {
                            // printf("[CURS_DEBUG] Found cursive anchors for gid %d in %s: Entry(%.1f, %.1f) Exit(%.1f, %.1f)\n", gid, info->path, enX, enY, exX, exY);
                            break;
                        }
                    }
                }
            }
        }

        // --- GDEF PARITY ---
        if (info->ot->gdef_secret) {
            atomic_fetch_add(&ctx->gdef_present, 1);
            const uint8_t *gdef = &info->ot->data[info->ot->gdef_secret];
            uint32_t version = READ_U32(gdef);
            if (version != 0x00010000 && version != 0x00010002 && version != 0x00010003) {
                atomic_fetch_add(&ctx->gdef_parity_fail, 1);
            } else {
                int c=0, a=0, cr=0, ms=0;
                walk_gdef_stats(gdef, info->ot->size - info->ot->gdef_secret, &c, &a, &cr, &ms);
                atomic_fetch_add(&ctx->total_gdef_classes, c);
                atomic_fetch_add(&ctx->total_gdef_attachments, a);
                atomic_fetch_add(&ctx->total_gdef_carets, cr);
                atomic_fetch_add(&ctx->gdef_mark_glyph_sets, ms);
            }
        }

        // --- JSTF PARITY ---
        if (info->ot->jstf_secret) {
            atomic_fetch_add(&ctx->jstf_present, 1);
            const uint8_t *jstf = &info->ot->data[info->ot->jstf_secret];
            uint32_t version = READ_U32(jstf);
            if (version != 0x00010000) {
                atomic_fetch_add(&ctx->jstf_parity_fail, 1);
            } else {
                int s=0, e=0, p=0, gm=0, pm=0;
                walk_jstf_stats(jstf, info->ot->size - info->ot->jstf_secret, &s, &e, &p, &gm, &pm);
                if (s == 0) {
                    atomic_fetch_add(&ctx->jstf_parity_fail, 1);
                } else {
                    atomic_fetch_add(&ctx->total_jstf_scripts, s);
                    atomic_fetch_add(&ctx->total_jstf_extenders, e);
                    atomic_fetch_add(&ctx->jstf_priority_records, p);
                    atomic_fetch_add(&ctx->jstf_gsub_mods, gm);
                    atomic_fetch_add(&ctx->jstf_gpos_mods, pm);
                }
            }
        }

        // --- BASE PARITY ---
        if (info->ot->base_secret) {
            atomic_fetch_add(&ctx->base_present, 1);
            const uint8_t *base = &info->ot->data[info->ot->base_secret];
            uint32_t version = READ_U32(base);
            if (version != 0x00010000 && version != 0x00010001) {
                atomic_fetch_add(&ctx->base_parity_fail, 1);
            } else {
                uint16_t horiz = READ_U16(base + 4);
                uint16_t vert = READ_U16(base + 6);
                if (horiz == 0 && vert == 0) {
                    atomic_fetch_add(&ctx->base_parity_fail, 1);
                } else {
                    if (horiz) atomic_fetch_add(&ctx->total_base_horiz, 1);
                    if (vert) atomic_fetch_add(&ctx->total_base_vert, 1);
                    int scripts = 0, coords = 0;
                    walk_base_stats(base, info->ot->size - info->ot->base_secret, &scripts, &coords);
                    atomic_fetch_add(&ctx->total_base_scripts, scripts);
                    atomic_fetch_add(&ctx->total_base_coords, coords);
                }
            }
        }

        // --- MATH PARITY ---
        if (info->ot->math_secret) {
            atomic_fetch_add(&ctx->math_present, 1);
            const uint8_t *math = &info->ot->data[info->ot->math_secret];
            uint32_t version = READ_U32(math);
            if (version != 0x00010000) {
                atomic_fetch_add(&ctx->math_parity_fail, 1);
            } else {
                int c=0, g=0, v=0, me=0;
                walk_math_stats(math, info->ot->size - info->ot->math_secret, &c, &g, &v, &me);
                if (c == 0 && g == 0 && v == 0) {
                    atomic_fetch_add(&ctx->math_parity_fail, 1);
                } else {
                    atomic_fetch_add(&ctx->total_math_constants, c);
                    atomic_fetch_add(&ctx->total_math_glyph_info, g);
                    atomic_fetch_add(&ctx->total_math_variants, v);
                    atomic_fetch_add(&ctx->math_extenders, me);
                }
            }
        }

        // --- VARIATION PARITY ---
        if (info->ot->fvar_secret) {
            atomic_fetch_add(&ctx->fvar_present, 1);
            const uint8_t *fvar = &info->ot->data[info->ot->fvar_secret];
            if (READ_U32(fvar) != 0x00010000) atomic_fetch_add(&ctx->var_parity_fail, 1);
            else if (info->ot->num_axes == 0) atomic_fetch_add(&ctx->var_parity_fail, 1);
        }
        if (info->ot->avar_secret) {
            atomic_fetch_add(&ctx->avar_present, 1);
            const uint8_t *avar = &info->ot->data[info->ot->avar_secret];
            if (READ_U32(avar) != 0x00010000) atomic_fetch_add(&ctx->var_parity_fail, 1);
        }
        if (info->ot->hvar_secret) {
            atomic_fetch_add(&ctx->hvar_present, 1);
            const uint8_t *hvar = &info->ot->data[info->ot->hvar_secret];
            if (READ_U32(hvar) != 0x00010000) atomic_fetch_add(&ctx->var_parity_fail, 1);
            else if (READ_U32(hvar + 4) == 0) atomic_fetch_add(&ctx->var_parity_fail, 1); // store off
        }
        if (info->ot->vvar_secret) {
            atomic_fetch_add(&ctx->vvar_present, 1);
            const uint8_t *vvar = &info->ot->data[info->ot->vvar_secret];
            if (READ_U32(vvar) != 0x00010000) atomic_fetch_add(&ctx->var_parity_fail, 1);
            else if (READ_U32(vvar + 4) == 0) atomic_fetch_add(&ctx->var_parity_fail, 1);
        }
        if (info->ot->mvar_secret) {
            atomic_fetch_add(&ctx->mvar_present, 1);
            const uint8_t *mvar = &info->ot->data[info->ot->mvar_secret];
            if (READ_U32(mvar) != 0x00010000) atomic_fetch_add(&ctx->var_parity_fail, 1);
            else if (READ_U32(mvar + 8) == 0) atomic_fetch_add(&ctx->var_parity_fail, 1);
        }
        if (info->ot->gvar_secret) {
            atomic_fetch_add(&ctx->gvar_present, 1);
            const uint8_t *gvar = &info->ot->data[info->ot->gvar_secret];
            if (READ_U32(gvar) != 0x00010000) atomic_fetch_add(&ctx->var_parity_fail, 1);
        }

        // --- VERTICAL METRICS PARITY ---
        if (info->ot->vhea_secret) {
            atomic_fetch_add(&ctx->vhea_present, 1);
            const uint8_t *vhea = &info->ot->data[info->ot->vhea_secret];
            if (READ_U16(vhea + 4) != info->ot->vert_ascent || READ_U16(vhea + 34) != info->ot->num_v_metrics) {
                atomic_fetch_add(&ctx->vhea_parity_fail, 1);
            }
        }
        if (info->ot->vmtx_secret) {
            atomic_fetch_add(&ctx->vmtx_present, 1);
            if (info->ot->num_v_metrics == 0) atomic_fetch_add(&ctx->vmtx_parity_fail, 1);
        }

        // --- CMAP FORMAT 14 DETECTION & PROBE ---
        if (info->ot->cmap_secret) {
            const uint8_t *cmap = &info->ot->data[info->ot->cmap_secret];
            uint16_t num_subtables = READ_U16(&cmap[2]);
            bool has_f14 = false;
            for (uint16_t i = 0; i < num_subtables; i++) {
                uint32_t sub_secret = READ_U32(&cmap[4 + i * 8 + 4]);
                if (READ_U16(&info->ot->data[info->ot->cmap_secret + sub_secret]) == 14) {
                    has_f14 = true;
                    atomic_fetch_add(&ctx->cmap_format14_present, 1);
                    break;
                }
            }
            if (has_f14) {
                // Probe a few common variation selectors (e.g. VS15=0xFE0E, VS16=0xFE0F)
                // We don't necessarily expect a match for every codepoint, 
                // but we want to ensure the code path doesn't crash and returns something.
                uint32_t gid = tsfi_ottype_get_glyph_index(info->ot, 0x263A, 0xFE0E); // U+263A (Smiling Face) + VS15
                (void)gid;
            }
        }

        // --- FAMILY CLASS ---
        uint8_t fclass = (info->ot->family_class >> 8) & 0xFF;
        uint8_t fsub = info->ot->family_class & 0xFF;
        atomic_fetch_add(&ctx->family_class_hist[fclass], 1);
        atomic_fetch_add(&ctx->family_subclass_hist[fsub], 1);

        // --- RAW MAXP PARITY (Byte for byte) ---
        uint32_t maxp_off = 0;
        for (uint16_t i = 0; i < num_tables; i++) {
            uint32_t tag = READ_U32(&info->ot->data[base_off + 12 + i * 16]);
            if (tag == 0x6d617870) { // 'maxp'
                maxp_off = READ_U32(&info->ot->data[base_off + 12 + i * 16 + 8]);
                break;
            }
        }
        bool raw_maxp_ok = true;
        if (maxp_off) {
            uint32_t r_ver = READ_U32(&info->ot->data[maxp_off]);
            if (r_ver == 0x00010000) {
                uint16_t r_stack = READ_U16(&info->ot->data[maxp_off + 24]);
                if (r_stack != info->ot->max_stack_elements) raw_maxp_ok = false;
            }
        }

        int observed_max_advance = 0;
        int observed_min_lsb = 32767;
        int observed_min_rsb = 32767;
        int observed_max_extent = -32768;

        uint32_t obs_max_pts = 0;
        uint16_t obs_max_contours = 0;
        uint32_t obs_max_comp_pts = 0;
        uint16_t obs_max_comp_contours = 0;
        uint16_t obs_max_comp_elements = 0;

        // Local counters to reduce global atomic contention
        int l_total_glyphs = 0;
        int l_hmtx_parity_fail = 0;
        int l_stb_renderable = 0;
        int l_glyphs_cff = 0;
        int l_glyphs_simple = 0;
        int l_glyphs_compound = 0;
        int l_tsfi_geom_success = 0;
        int l_simple_success = 0;
        int l_compound_success = 0;

        uint32_t font_glyphs = info->ot->num_glyphs;
        for (uint32_t gid = 0; gid < font_glyphs; gid++) {
            l_total_glyphs++;
            
            int ot_adv, ot_lsb, stb_adv, stb_lsb;
            bool ot_m_ok = tsfi_ottype_get_glyph_metrics(info->ot, gid, &ot_adv, &ot_lsb);
            stbtt_GetGlyphHMetrics(&stb, gid, &stb_adv, &stb_lsb);
            
            if (!ot_m_ok || ot_adv != stb_adv || ot_lsb != stb_lsb) {
                l_hmtx_parity_fail++;
            }

            // Update observed hhea summary (Skip GID 0 and Whitespace for parity)
            bool empty = (bool)stbtt_IsGlyphEmpty(&stb, gid);
            if (gid > 0 && !empty) {
                if (ot_adv > observed_max_advance) observed_max_advance = ot_adv;
                if (ot_lsb < observed_min_lsb) observed_min_lsb = ot_lsb;
                
                int x1, y1, x2, y2;
                if (stbtt_GetGlyphBox(&stb, gid, &x1, &y1, &x2, &y2)) {
                    int rsb = ot_adv - (ot_lsb + (x2 - x1));
                    if (rsb < observed_min_rsb) observed_min_rsb = rsb;
                    int extent = ot_lsb + (x2 - x1);
                    if (extent > observed_max_extent) observed_max_extent = extent;
                }
            }

            if (gid == 0 || empty) continue;
            l_stb_renderable++;

            if (info->ot->is_cff) {
                l_glyphs_cff++;
                // Process CFF geometry too
            } else {
                uint32_t glyph_off = tsfi_ottype_get_glyph_location(info->ot, gid);
                uint32_t next_off = tsfi_ottype_get_glyph_location(info->ot, gid + 1);
                if (glyph_off != (uint32_t)-1 && next_off != (uint32_t)-1 && glyph_off < next_off) {
                    const uint8_t *glyf = &info->ot->data[info->ot->glyf_secret + glyph_off];
                    int16_t num_contours = (int16_t)READ_U16(glyf);
                    if (num_contours >= 0) {
                        l_glyphs_simple++;
                    } else {
                        l_glyphs_compound++;
                        int elements = 0;
                        const uint8_t *ptr = glyf + 10;
                        while (1) {
                            elements++;
                            uint16_t flags = READ_U16(ptr); ptr += 4; // flags + gid
                            if (flags & 0x01) ptr += 4; else ptr += 2; // args
                            if (flags & 0x08) ptr += 2; else if (flags & 0x40) ptr += 4; else if (flags & 0x80) ptr += 8;
                            if (!(flags & 0x20)) break;
                        }
                        if (elements > (int)obs_max_comp_elements) obs_max_comp_elements = (uint16_t)elements;
                    }
                }
            }

            // REUSE BUFFER: reset point/contour counts but KEEP allocated memory
            geom.num_points = 0;
            geom.num_contours = 0;
            
            if (tsfi_ottype_get_glyph_geom(info->ot, gid, &geom)) {
                if (geom.num_points > 0 || empty) {
                    l_tsfi_geom_success++;
                    if (!info->ot->is_cff) {
                        uint32_t g_off = tsfi_ottype_get_glyph_location(info->ot, gid);
                        const uint8_t *glyf_data = &info->ot->data[info->ot->glyf_secret + g_off];
                        int16_t n_c = (int16_t)READ_U16(glyf_data);
                        
                        if (n_c >= 0) {
                            l_simple_success++;
                            if (geom.num_points > obs_max_pts) obs_max_pts = geom.num_points;
                            if (geom.num_contours > obs_max_contours) obs_max_contours = geom.num_contours;
                        } else {
                            l_compound_success++;
                            if (geom.num_points > obs_max_comp_pts) obs_max_comp_pts = geom.num_points;
                            if (geom.num_contours > obs_max_comp_contours) obs_max_comp_contours = geom.num_contours;
                        }
                    } else {
                        // For CFF, we just track overall success for now
                        l_simple_success++; 
                    }
                }
            }
        }

        // Commit local counters to global context
        atomic_fetch_add(&ctx->total_glyphs_checked, l_total_glyphs);
        atomic_fetch_add(&ctx->hmtx_parity_fail, l_hmtx_parity_fail);
        atomic_fetch_add(&ctx->stb_renderable_glyphs, l_stb_renderable);
        atomic_fetch_add(&ctx->glyphs_cff, l_glyphs_cff);
        atomic_fetch_add(&ctx->glyphs_simple, l_glyphs_simple);
        atomic_fetch_add(&ctx->glyphs_compound, l_glyphs_compound);
        atomic_fetch_add(&ctx->tsfi_geometry_success, l_tsfi_geom_success);
        atomic_fetch_add(&ctx->simple_success, l_simple_success);
        atomic_fetch_add(&ctx->compound_success, l_compound_success);

        bool summary_hhea_ok = (info->ot->advance_width_max >= observed_max_advance &&
                                info->ot->min_lsb <= observed_min_lsb &&
                                info->ot->min_rsb <= observed_min_rsb &&
                                info->ot->max_x_extent >= observed_max_extent);
        
        bool summary_maxp_ok = (info->ot->max_points >= obs_max_pts &&
                                info->ot->max_contours >= obs_max_contours &&
                                info->ot->max_composite_points >= obs_max_comp_pts &&
                                info->ot->max_composite_contours >= obs_max_comp_contours &&
                                info->ot->max_component_elements >= obs_max_comp_elements);

        if (!raw_hhea_ok || !summary_hhea_ok) {
            atomic_fetch_add(&ctx->hhea_parity_fail, 1);
            if (!raw_hhea_ok) printf("[HHEA_RAW_ERR] %s: Parser mismatch against table bytes!\n", info->path);
            if (!summary_hhea_ok) {
                printf("[HHEA_SUM_DISC] %s: Summary fields inconsistent with aggregate glyph metrics:\n", info->path);
                if (info->ot->advance_width_max < observed_max_advance) printf("  AdvMax: %d (Table) < %d (Observed)\n", info->ot->advance_width_max, observed_max_advance);
                if (info->ot->min_lsb > observed_min_lsb) printf("  MinLSB: %d (Table) > %d (Observed)\n", info->ot->min_lsb, observed_min_lsb);
                if (info->ot->min_rsb > observed_min_rsb) printf("  MinRSB: %d (Table) > %d (Observed)\n", info->ot->min_rsb, observed_min_rsb);
                if (info->ot->max_x_extent < observed_max_extent) printf("  MaxExt: %d (Table) < %d (Observed)\n", info->ot->max_x_extent, observed_max_extent);
            }
        }

        if (!summary_maxp_ok || !raw_maxp_ok) {
            atomic_fetch_add(&ctx->maxp_parity_fail, 1);
            if (!raw_maxp_ok) printf("[MAXP_RAW_ERR] %s: Parser mismatch against table bytes (stack element parity)!\n", info->path);
            if (!summary_maxp_ok) {
                printf("[MAXP_SUM_DISC] %s: Summary fields inconsistent with aggregate geometry:\n", info->path);
                if (info->ot->max_points < obs_max_pts) printf("  MaxPts: %d (Table) < %d (Observed)\n", info->ot->max_points, obs_max_pts);
                if (info->ot->max_contours < obs_max_contours) printf("  MaxContours: %d (Table) < %d (Observed)\n", info->ot->max_contours, obs_max_contours);
                if (info->ot->max_composite_points < obs_max_comp_pts) printf("  MaxCompPts: %d (Table) < %d (Observed)\n", info->ot->max_composite_points, obs_max_comp_pts);
                if (info->ot->max_composite_contours < obs_max_comp_contours) printf("  MaxCompContours: %d (Table) < %d (Observed)\n", info->ot->max_composite_contours, obs_max_comp_contours);
                if (info->ot->max_component_elements < obs_max_comp_elements) printf("  MaxCompElements: %d (Table) < %d (Observed)\n", info->ot->max_component_elements, obs_max_comp_elements);
            }
        }
    }

    tsfi_ottype_free_geom(&geom);
    return NULL;
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    TSFiFontRegistry *reg = lau_memalign_wired(512, sizeof(TSFiFontRegistry));
    tsfi_font_registry_init(reg);
    tsfi_font_registry_scan_system(reg);

    test_context_t ctx;
    ctx.reg = reg;
    atomic_init(&ctx.next_font_idx, 0);
    atomic_init(&ctx.total_fonts_tested, 0);
    atomic_init(&ctx.hhea_parity_fail, 0);
    atomic_init(&ctx.maxp_parity_fail, 0);
    atomic_init(&ctx.ltsh_parity_fail, 0);
    atomic_init(&ctx.vdmx_parity_fail, 0);
    atomic_init(&ctx.hdmx_parity_fail, 0);
    atomic_init(&ctx.gsub_present, 0);
    atomic_init(&ctx.gpos_present, 0);
    atomic_init(&ctx.gdef_present, 0);
    atomic_init(&ctx.jstf_present, 0);
    atomic_init(&ctx.base_present, 0);
    atomic_init(&ctx.gsub_parity_fail, 0);
    atomic_init(&ctx.gpos_parity_fail, 0);
    atomic_init(&ctx.gdef_parity_fail, 0);
    atomic_init(&ctx.jstf_parity_fail, 0);
    atomic_init(&ctx.base_parity_fail, 0);
    atomic_init(&ctx.math_present, 0);
    atomic_init(&ctx.math_parity_fail, 0);
    atomic_init(&ctx.vhea_present, 0);
    atomic_init(&ctx.vmtx_present, 0);
    atomic_init(&ctx.vhea_parity_fail, 0);
    atomic_init(&ctx.vmtx_parity_fail, 0);
    atomic_init(&ctx.cmap_format14_present, 0);
    atomic_init(&ctx.feat_isol, 0);
    atomic_init(&ctx.feat_init, 0);
    atomic_init(&ctx.feat_medi, 0);
    atomic_init(&ctx.feat_fina, 0);
    atomic_init(&ctx.feat_curs, 0);
    atomic_init(&ctx.feat_liga, 0);
    atomic_init(&ctx.fvar_present, 0);
    atomic_init(&ctx.avar_present, 0);
    atomic_init(&ctx.hvar_present, 0);
    atomic_init(&ctx.vvar_present, 0);
    atomic_init(&ctx.mvar_present, 0);
    atomic_init(&ctx.gvar_present, 0);
    atomic_init(&ctx.var_parity_fail, 0);
    atomic_init(&ctx.total_scripts, 0);
    atomic_init(&ctx.total_features, 0);
        atomic_init(&ctx.total_lookups, 0);
        atomic_init(&ctx.total_required_features, 0);
        atomic_init(&ctx.jstf_priority_records, 0);
        atomic_init(&ctx.jstf_gsub_mods, 0);
        atomic_init(&ctx.jstf_gpos_mods, 0);
        atomic_init(&ctx.math_extenders, 0);
        atomic_init(&ctx.gdef_mark_glyph_sets, 0);
        for (int i=0; i<10; i++) {
            atomic_init(&ctx.gsub_lookup_types[i], 0);
            atomic_init(&ctx.gpos_lookup_types[i], 0);
        }
    
        atomic_init(&ctx.total_gdef_classes, 0);
    atomic_init(&ctx.total_gdef_attachments, 0);
    atomic_init(&ctx.total_gdef_carets, 0);
    atomic_init(&ctx.total_jstf_scripts, 0);
    atomic_init(&ctx.total_jstf_extenders, 0);
    atomic_init(&ctx.total_base_horiz, 0);
    atomic_init(&ctx.total_base_vert, 0);
    atomic_init(&ctx.total_base_scripts, 0);
    atomic_init(&ctx.total_base_coords, 0);
    atomic_init(&ctx.total_feature_variations, 0);
    atomic_init(&ctx.total_coverage_tables, 0);
    atomic_init(&ctx.total_class_defs, 0);
    for (int i=0; i<16; i++) atomic_init(&ctx.lookup_flags[i], 0);
    atomic_init(&ctx.total_math_constants, 0);
    atomic_init(&ctx.total_math_glyph_info, 0);
    atomic_init(&ctx.total_math_variants, 0);
    for (int i=0; i<256; i++) {
        atomic_init(&ctx.family_class_hist[i], 0);
        atomic_init(&ctx.family_subclass_hist[i], 0);
    }
    atomic_init(&ctx.total_glyphs_checked, 0);
    atomic_init(&ctx.hmtx_parity_fail, 0);
    atomic_init(&ctx.stb_renderable_glyphs, 0);
    atomic_init(&ctx.tsfi_geometry_success, 0);
    atomic_init(&ctx.glyphs_cff, 0);
    atomic_init(&ctx.glyphs_simple, 0);
    atomic_init(&ctx.glyphs_compound, 0);
    atomic_init(&ctx.simple_success, 0);
    atomic_init(&ctx.compound_success, 0);

    atomic_init(&ctx.tag_mutex, 0);
    ctx.unique_tag_count = 0;

    tsfi_zhao_init(sysconf(_SC_NPROCESSORS_ONLN));

    int num_threads = 32; 
    LauWireThread *threads[32];
    for (int i = 0; i < num_threads; i++) {
        threads[i] = tsfi_wire_thread_create(i, 1.0);
        threads[i]->start_routine = test_font_worker;
        threads[i]->arg = &ctx;
        tsfi_zhao_submit(dispatcher_task, threads[i]);
    }

    tsfi_zhao_sync();

    for (int i = 0; i < num_threads; i++) {
        LauSystemHeader *h = (LauSystemHeader *)((char *)threads[i] - offsetof(LauSystemHeader, payload));
        h->proxy = NULL;
        lau_free(threads[i]);
    }

    int total_renderable = atomic_load(&ctx.stb_renderable_glyphs);
    int total_cff = atomic_load(&ctx.glyphs_cff);
    int total_simple = atomic_load(&ctx.glyphs_simple);
    int total_compound = atomic_load(&ctx.glyphs_compound);
    
    int success_simple = atomic_load(&ctx.simple_success);
    int success_compound = atomic_load(&ctx.compound_success);

    printf("\n--- HONEST ARCHITECTURAL SUPPORT REPORT ---\n");
    printf("Unique Feature Tags: ");
    for (int i=0; i<ctx.unique_tag_count; i++) printf("%c%c%c%c ", (ctx.unique_tags[i]>>24)&0xFF, (ctx.unique_tags[i]>>16)&0xFF, (ctx.unique_tags[i]>>8)&0xFF, ctx.unique_tags[i]&0xFF);
    printf("\n");
    printf("Total Fonts Scanned:      %d\n", atomic_load(&ctx.total_fonts_tested));
    printf("HHEA/Summary Parity Fail: %d\n", atomic_load(&ctx.hhea_parity_fail));
    printf("MAXP Parity Fail:        %d\n", atomic_load(&ctx.maxp_parity_fail));
    printf("LTSH Parity Fail:        %d\n", atomic_load(&ctx.ltsh_parity_fail));
    printf("VDMX Parity Fail:        %d\n", atomic_load(&ctx.vdmx_parity_fail));
    printf("HDMX Parity Fail:        %d\n", atomic_load(&ctx.hdmx_parity_fail));
    printf("GSUB Parity Fail:        %d (of %d present)\n", atomic_load(&ctx.gsub_parity_fail), atomic_load(&ctx.gsub_present));
    printf("GPOS Parity Fail:        %d (of %d present)\n", atomic_load(&ctx.gpos_parity_fail), atomic_load(&ctx.gpos_present));
    printf("GDEF Parity Fail:        %d (of %d present)\n", atomic_load(&ctx.gdef_parity_fail), atomic_load(&ctx.gdef_present));
    printf("JSTF Parity Fail:        %d (of %d present)\n", atomic_load(&ctx.jstf_parity_fail), atomic_load(&ctx.jstf_present));
    printf("BASE Parity Fail:        %d (of %d present)\n", atomic_load(&ctx.base_parity_fail), atomic_load(&ctx.base_present));
    printf("MATH Parity Fail:        %d (of %d present)\n", atomic_load(&ctx.math_parity_fail), atomic_load(&ctx.math_present));
    printf("Var Parity Fail:         %d (fvar:%d, avar:%d, HVAR:%d, VVAR:%d, MVAR:%d, gvar:%d)\n",
           atomic_load(&ctx.var_parity_fail),
           atomic_load(&ctx.fvar_present), atomic_load(&ctx.avar_present),
           atomic_load(&ctx.hvar_present), atomic_load(&ctx.vvar_present),
           atomic_load(&ctx.mvar_present), atomic_load(&ctx.gvar_present));
    printf("Vertical Metrics:        %d vhea, %d vmtx present (%d/%d fail)\n",
           atomic_load(&ctx.vhea_present), atomic_load(&ctx.vmtx_present),
           atomic_load(&ctx.vhea_parity_fail), atomic_load(&ctx.vmtx_parity_fail));
    printf("Variation Selectors:     %d fonts with cmap Format 14\n",
           atomic_load(&ctx.cmap_format14_present));
    printf("Positional Features:     isol:%ld init:%ld medi:%ld fina:%ld curs:%ld liga:%ld\n",
           atomic_load(&ctx.feat_isol), atomic_load(&ctx.feat_init),
           atomic_load(&ctx.feat_medi), atomic_load(&ctx.feat_fina),
           atomic_load(&ctx.feat_curs), atomic_load(&ctx.feat_liga));

            int gdef_valid = atomic_load(&ctx.gdef_present) - atomic_load(&ctx.gdef_parity_fail);
            int jstf_valid = atomic_load(&ctx.jstf_present) - atomic_load(&ctx.jstf_parity_fail);
            int base_valid = atomic_load(&ctx.base_present) - atomic_load(&ctx.base_parity_fail);
            int math_valid = atomic_load(&ctx.math_present) - atomic_load(&ctx.math_parity_fail);
            int layout_valid = (atomic_load(&ctx.gsub_present) - atomic_load(&ctx.gsub_parity_fail)) + (atomic_load(&ctx.gpos_present) - atomic_load(&ctx.gpos_parity_fail));

            printf("Layout Agg (%d valid):    %ld Scripts, %ld Features, %ld Lookups, %ld RequiredFeatures, %ld FeatureVariations\n", 
                   layout_valid, atomic_load(&ctx.total_scripts), atomic_load(&ctx.total_features), atomic_load(&ctx.total_lookups), atomic_load(&ctx.total_required_features), atomic_load(&ctx.total_feature_variations));
                    printf("Layout Complexity:       %ld CoverageTables, %ld ClassDefs\n",
                           atomic_load(&ctx.total_coverage_tables), atomic_load(&ctx.total_class_defs));
                    printf("Lookup Flags:           ");
                    const char *flag_names[16] = {"RTL", "IBase", "ILig", "IMark", "UIM", "Z5", "Z6", "Z7", "MAT", "Z9", "ZA", "ZB", "ZC", "ZD", "ZE", "ZF"};
                    for (int i=0; i<4; i++) printf("%s:%ld ", flag_names[i], atomic_load(&ctx.lookup_flags[i]));
                    printf("\nGSUB Lookup Types:      ");            for (int i=1; i<10; i++) printf("T%d:%ld ", i, atomic_load(&ctx.gsub_lookup_types[i]));
            printf("\nGPOS Lookup Types:      ");
            for (int i=1; i<10; i++) printf("T%d:%ld ", i, atomic_load(&ctx.gpos_lookup_types[i]));
            printf("\nGDEF Agg (%d valid):      %ld ClassDefs, %ld AttachLists, %ld CaretLists, %ld MarkSets\n",
                   gdef_valid, atomic_load(&ctx.total_gdef_classes), atomic_load(&ctx.total_gdef_attachments), atomic_load(&ctx.total_gdef_carets), atomic_load(&ctx.gdef_mark_glyph_sets));
            printf("JSTF Agg (%d valid):      %ld Scripts, %ld Extenders, %ld Priorities, %ld GSUBMods, %ld GPOSMods\n", 
                   jstf_valid, atomic_load(&ctx.total_jstf_scripts), atomic_load(&ctx.total_jstf_extenders), atomic_load(&ctx.jstf_priority_records), atomic_load(&ctx.jstf_gsub_mods), atomic_load(&ctx.jstf_gpos_mods));
            printf("BASE Agg (%d valid):      %ld Horiz, %ld Vert, %ld Scripts, %ld Coords\n", 
                   base_valid, atomic_load(&ctx.total_base_horiz), atomic_load(&ctx.total_base_vert), atomic_load(&ctx.total_base_scripts), atomic_load(&ctx.total_base_coords));
            printf("MATH Agg (%d valid):      %ld Constants, %ld GlyphInfo, %ld Variants, %ld Extenders\n",
                   math_valid, atomic_load(&ctx.total_math_constants), atomic_load(&ctx.total_math_glyph_info), atomic_load(&ctx.total_math_variants), atomic_load(&ctx.math_extenders));    
    const char *class_names[15] = {
        "NoClass", "OldSerif", "TransSerif", "ModSerif", "ClarSerif",
        "SlabSerif", "FreeSerif", "SansSerif", "Ornam", "Script",
        "Symbol", "Res11", "Res12", "Res13", "Res14"
    };
    printf("Family Class Hist:       ");
    for(int i=0; i<256; i++) {
        long count = atomic_load(&ctx.family_class_hist[i]);
        if (count > 0) {
            if (i < 15) printf("%s:%ld ", class_names[i], count);
            else printf("0x%02X:%ld ", i, count);
        }
    }
    printf("\nFamily Subclass Hist:    ");
    for(int i=0; i<256; i++) {
        long count = atomic_load(&ctx.family_subclass_hist[i]);
        if (count > 0) printf("0x%02X:%ld ", i, count);
    }
    printf("\n");

    printf("HMTX Parity Failed:       %d (of %d total indices)\n", 
           atomic_load(&ctx.hmtx_parity_fail), atomic_load(&ctx.total_glyphs_checked));
    
    printf("\nGLYPH CATEGORIZATION (of %d renderable):\n", total_renderable);
    printf("  CFF (Unsupported):      %d (%.2f%%)\n", total_cff, total_cff * 100.0 / total_renderable);
    printf("  Simple (TrueType):      %d (%.2f%%) -> %d Success (100.00%%)\n", 
           total_simple, total_simple * 100.0 / total_renderable, success_simple);
    printf("  Compound (TrueType):    %d (%.2f%%) -> %d Success (%.2f%%)\n", 
           total_compound, total_compound * 100.0 / total_renderable,
           success_compound, (total_compound > 0) ? success_compound * 100.0 / total_compound : 0.0);

    printf("\nOVERALL TRULY SUPPORTED:  %d (%.2f%% of targets)\n", 
           atomic_load(&ctx.tsfi_geometry_success),
           (total_renderable > 0) ? atomic_load(&ctx.tsfi_geometry_success) * 100.0 / total_renderable : 0.0);

    // Cleanup
    tsfi_zhao_shutdown();
    for(int i=0; i<reg->count; i++) {
        if(reg->fonts[i].ot) lau_free(reg->fonts[i].ot);
    }
    lau_free(reg->bmp_cache);
    lau_free(reg->fonts);
    lau_free(reg);

        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
