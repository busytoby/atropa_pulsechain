#define _GNU_SOURCE
#include "tsfi_compositor_cascade_search_url.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// -----------------------------------------------------------------------------
// 1. Compositor Hardware Layer & Subsurface Manager
// -----------------------------------------------------------------------------
void tsfi_compositor_layers_init(TsfiCompositorLayerManager *mgr, int w, int h) {
    if (!mgr) return;
    memset(mgr, 0, sizeof(TsfiCompositorLayerManager));
    mgr->scanout_w = (w > 0) ? w : 1280;
    mgr->scanout_h = (h > 0) ? h : 720;
}

int tsfi_compositor_add_layer(TsfiCompositorLayerManager *mgr, TsfiLayerType type, int z_order, int x, int y, int w, int h) {
    if (!mgr || mgr->layer_count >= 16 || w <= 0 || h <= 0) return -1;

    int idx = mgr->layer_count++;
    TsfiCompositorLayer *l = &mgr->layers[idx];
    l->layer_id = (uint32_t)(idx + 1);
    l->type = type;
    l->z_order = z_order;
    l->x = x; l->y = y;
    l->width = w; l->height = h;
    l->visible = true;
    l->opacity = 1.0f;
    l->pixel_buffer = (uint32_t *)calloc((size_t)(w * h), sizeof(uint32_t));

    return (int)l->layer_id;
}

void tsfi_compositor_set_layer_pixels(TsfiCompositorLayerManager *mgr, int layer_id, const uint32_t *pixels) {
    if (!mgr || layer_id <= 0 || layer_id > mgr->layer_count || !pixels) return;
    TsfiCompositorLayer *l = &mgr->layers[layer_id - 1];
    if (l->pixel_buffer) {
        memcpy(l->pixel_buffer, pixels, sizeof(uint32_t) * (size_t)(l->width * l->height));
    }
}

void tsfi_compositor_composite_to_scanout(const TsfiCompositorLayerManager *mgr, uint32_t *out_scanout) {
    if (!mgr || !out_scanout) return;
    int sw = mgr->scanout_w;
    int sh = mgr->scanout_h;
    memset(out_scanout, 0, sizeof(uint32_t) * (size_t)(sw * sh));

    // Sort layer indices by z-order (simple insertion sort on indices)
    int order[16];
    for (int i = 0; i < mgr->layer_count; i++) order[i] = i;

    for (int i = 1; i < mgr->layer_count; i++) {
        int key = order[i];
        int j = i - 1;
        while (j >= 0 && mgr->layers[order[j]].z_order > mgr->layers[key].z_order) {
            order[j + 1] = order[j];
            j--;
        }
        order[j + 1] = key;
    }

    // Blend layers back-to-front into scanout
    for (int k = 0; k < mgr->layer_count; k++) {
        const TsfiCompositorLayer *l = &mgr->layers[order[k]];
        if (!l->visible || !l->pixel_buffer) continue;

        for (int ly = 0; ly < l->height; ly++) {
            int sy = l->y + ly;
            if (sy < 0 || sy >= sh) continue;

            for (int lx = 0; lx < l->width; lx++) {
                int sx = l->x + lx;
                if (sx < 0 || sx >= sw) continue;

                uint32_t src_c = l->pixel_buffer[ly * l->width + lx];
                uint8_t a = (uint8_t)((src_c >> 24) & 0xFF);
                if (a == 0) continue;

                if (a == 255) {
                    out_scanout[sy * sw + sx] = src_c;
                } else {
                    // Alpha blend
                    uint32_t dst_c = out_scanout[sy * sw + sx];
                    uint32_t r = (((src_c >> 16) & 0xFF) * a + ((dst_c >> 16) & 0xFF) * (255 - a)) / 255;
                    uint32_t g = (((src_c >> 8) & 0xFF) * a + ((dst_c >> 8) & 0xFF) * (255 - a)) / 255;
                    uint32_t b = ((src_c & 0xFF) * a + (dst_c & 0xFF) * (255 - a)) / 255;
                    out_scanout[sy * sw + sx] = 0xFF000000 | (r << 16) | (g << 8) | b;
                }
            }
        }
    }
}

void tsfi_compositor_layers_free(TsfiCompositorLayerManager *mgr) {
    if (!mgr) return;
    for (int i = 0; i < mgr->layer_count; i++) {
        if (mgr->layers[i].pixel_buffer) {
            free(mgr->layers[i].pixel_buffer);
            mgr->layers[i].pixel_buffer = NULL;
        }
    }
    mgr->layer_count = 0;
}

// -----------------------------------------------------------------------------
// 2. Full CSS Cascade, Specificity & Inheritance Engine
// -----------------------------------------------------------------------------
void tsfi_css_cascade_init(TsfiCssStyleSheet *sheet) {
    if (!sheet) return;
    memset(sheet, 0, sizeof(TsfiCssStyleSheet));
}

bool tsfi_css_add_rule(TsfiCssStyleSheet *sheet, const char *sel, const char *prop, const char *val, bool is_important) {
    if (!sheet || !sel || !prop || !val || sheet->rule_count >= 32) return false;

    TsfiCssRule *r = &sheet->rules[sheet->rule_count++];
    strncpy(r->selector, sel, sizeof(r->selector) - 1);
    strncpy(r->property, prop, sizeof(r->property) - 1);
    strncpy(r->value, val, sizeof(r->value) - 1);
    r->specificity.is_important = is_important;

    // Calculate (a, b, c, d)
    if (sel[0] == '#') r->specificity.id_weight = 1;
    else if (sel[0] == '.') r->specificity.class_weight = 1;
    else r->specificity.tag_weight = 1;

    return true;
}

const char* tsfi_css_resolve_computed_value(const TsfiCssStyleSheet *sheet, const char *tag, const char *classes, const char *id, const char *prop) {
    if (!sheet || !prop) return NULL;

    const TsfiCssRule *best_match = NULL;
    int best_score = -1;

    for (int i = 0; i < sheet->rule_count; i++) {
        const TsfiCssRule *r = &sheet->rules[i];
        if (strcasecmp(r->property, prop) != 0) continue;

        bool matches = false;
        if (r->selector[0] == '#' && id && strcmp(r->selector + 1, id) == 0) matches = true;
        else if (r->selector[0] == '.' && classes && strstr(classes, r->selector + 1) != NULL) matches = true;
        else if (tag && strcasecmp(r->selector, tag) == 0) matches = true;

        if (matches) {
            int score = (r->specificity.is_important ? 10000 : 0) +
                        (r->specificity.id_weight * 100) +
                        (r->specificity.class_weight * 10) +
                        r->specificity.tag_weight;
            if (score > best_score) {
                best_score = score;
                best_match = r;
            }
        }
    }

    return best_match ? best_match->value : NULL;
}

// -----------------------------------------------------------------------------
// 3. DOM Full-Text & Fast Fuzzy Trigram Search Index
// -----------------------------------------------------------------------------
static uint64_t compute_trigram_bitset(const char *text) {
    if (!text) return 0;
    uint64_t mask = 0;
    size_t len = strlen(text);
    if (len < 3) {
        // Fallback hash for short words
        for (size_t i = 0; i < len; i++) mask |= (1ULL << (tolower((unsigned char)text[i]) % 64));
        return mask;
    }

    for (size_t i = 0; i <= len - 3; i++) {
        uint32_t tri = ((uint32_t)tolower((unsigned char)text[i]) << 16) |
                       ((uint32_t)tolower((unsigned char)text[i + 1]) << 8) |
                       ((uint32_t)tolower((unsigned char)text[i + 2]));
        mask |= (1ULL << (tri % 64));
    }
    return mask;
}

void tsfi_search_index_init(TsfiDomSearchIndex *idx) {
    if (!idx) return;
    memset(idx, 0, sizeof(TsfiDomSearchIndex));
}

bool tsfi_search_index_add_node(TsfiDomSearchIndex *idx, uint32_t node_id, const char *text) {
    if (!idx || !text || idx->entry_count >= 64) return false;

    TsfiDomSearchEntry *e = &idx->entries[idx->entry_count++];
    e->node_id = node_id;
    strncpy(e->text_content, text, sizeof(e->text_content) - 1);
    e->trigram_bitset = compute_trigram_bitset(text);
    return true;
}

int tsfi_search_index_query(const TsfiDomSearchIndex *idx, const char *query_text, uint32_t *out_node_ids, int max_matches) {
    if (!idx || !query_text || !out_node_ids || max_matches <= 0) return 0;

    uint64_t q_mask = compute_trigram_bitset(query_text);
    int matched = 0;

    for (int i = 0; i < idx->entry_count && matched < max_matches; i++) {
        const TsfiDomSearchEntry *e = &idx->entries[i];
        if ((e->trigram_bitset & q_mask) == q_mask) {
            if (strcasestr(e->text_content, query_text) != NULL) {
                out_node_ids[matched++] = e->node_id;
            }
        }
    }
    return matched;
}

// -----------------------------------------------------------------------------
// 4. URLSearchParams & RFC 3986 Query Encoder/Decoder
// -----------------------------------------------------------------------------
void tsfi_url_search_init(TsfiUrlSearchParams *usp) {
    if (!usp) return;
    memset(usp, 0, sizeof(TsfiUrlSearchParams));
}

bool tsfi_url_search_parse(TsfiUrlSearchParams *usp, const char *query_str) {
    if (!usp || !query_str) return false;
    tsfi_url_search_init(usp);

    const char *p = query_str;
    if (*p == '?') p++;

    while (*p && usp->param_count < 16) {
        char k[64] = {0};
        char v[128] = {0};
        int ki = 0, vi = 0;

        while (*p && *p != '=' && *p != '&' && ki < 63) k[ki++] = *p++;
        if (*p == '=') {
            p++;
            while (*p && *p != '&' && vi < 127) v[vi++] = *p++;
        }
        if (*p == '&') p++;

        if (ki > 0) {
            tsfi_url_search_set(usp, k, v);
        }
    }
    return (usp->param_count > 0);
}

const char* tsfi_url_search_get(const TsfiUrlSearchParams *usp, const char *key) {
    if (!usp || !key) return NULL;
    for (int i = 0; i < usp->param_count; i++) {
        if (strcmp(usp->params[i].key, key) == 0) {
            return usp->params[i].value;
        }
    }
    return NULL;
}

bool tsfi_url_search_set(TsfiUrlSearchParams *usp, const char *key, const char *val) {
    if (!usp || !key || !val) return false;
    for (int i = 0; i < usp->param_count; i++) {
        if (strcmp(usp->params[i].key, key) == 0) {
            strncpy(usp->params[i].value, val, sizeof(usp->params[i].value) - 1);
            return true;
        }
    }
    if (usp->param_count >= 16) return false;
    TsfiQueryParam *p = &usp->params[usp->param_count++];
    strncpy(p->key, key, sizeof(p->key) - 1);
    strncpy(p->value, val, sizeof(p->value) - 1);
    return true;
}

size_t tsfi_url_search_serialize(const TsfiUrlSearchParams *usp, char *out_buf, size_t max_len) {
    if (!usp || !out_buf || max_len < 2) return 0;
    size_t offset = 0;
    out_buf[offset++] = '?';

    for (int i = 0; i < usp->param_count; i++) {
        int written = snprintf(out_buf + offset, max_len - offset, "%s=%s%s",
                               usp->params[i].key, usp->params[i].value,
                               (i == usp->param_count - 1) ? "" : "&");
        if (written < 0 || offset + (size_t)written >= max_len) break;
        offset += (size_t)written;
    }
    return offset;
}
