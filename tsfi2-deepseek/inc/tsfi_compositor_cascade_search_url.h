#ifndef TSFI_COMPOSITOR_CASCADE_SEARCH_URL_H
#define TSFI_COMPOSITOR_CASCADE_SEARCH_URL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// -----------------------------------------------------------------------------
// 1. Compositor Hardware Layer & Subsurface Manager (Compositor-Native)
// -----------------------------------------------------------------------------
typedef enum {
    LAYER_TYPE_BACKGROUND = 0,
    LAYER_TYPE_BROWSER_SURFACE,
    LAYER_TYPE_SUBSURFACE_POPUP,
    LAYER_TYPE_HARDWARE_CURSOR
} TsfiLayerType;

typedef struct {
    uint32_t layer_id;
    TsfiLayerType type;
    int z_order;
    int x, y;
    int width, height;
    uint32_t *pixel_buffer;
    bool visible;
    float opacity;
} TsfiCompositorLayer;

typedef struct {
    TsfiCompositorLayer layers[16];
    int layer_count;
    int scanout_w;
    int scanout_h;
} TsfiCompositorLayerManager;

void tsfi_compositor_layers_init(TsfiCompositorLayerManager *mgr, int w, int h);
int tsfi_compositor_add_layer(TsfiCompositorLayerManager *mgr, TsfiLayerType type, int z_order, int x, int y, int w, int h);
void tsfi_compositor_set_layer_pixels(TsfiCompositorLayerManager *mgr, int layer_id, const uint32_t *pixels);
void tsfi_compositor_composite_to_scanout(const TsfiCompositorLayerManager *mgr, uint32_t *out_scanout);
void tsfi_compositor_layers_free(TsfiCompositorLayerManager *mgr);

// -----------------------------------------------------------------------------
// 2. Full CSS Cascade, Specificity & Inheritance Engine
// -----------------------------------------------------------------------------
typedef struct {
    int inline_weight; // a
    int id_weight;     // b
    int class_weight;  // c
    int tag_weight;    // d
    bool is_important;
} TsfiCssSpecificity;

typedef struct {
    char selector[64];
    char property[32];
    char value[64];
    TsfiCssSpecificity specificity;
} TsfiCssRule;

typedef struct {
    TsfiCssRule rules[32];
    int rule_count;
} TsfiCssStyleSheet;

void tsfi_css_cascade_init(TsfiCssStyleSheet *sheet);
bool tsfi_css_add_rule(TsfiCssStyleSheet *sheet, const char *sel, const char *prop, const char *val, bool is_important);
const char* tsfi_css_resolve_computed_value(const TsfiCssStyleSheet *sheet, const char *tag, const char *classes, const char *id, const char *prop);

// -----------------------------------------------------------------------------
// 3. DOM Full-Text & Fast Fuzzy Trigram Search Index
// -----------------------------------------------------------------------------
typedef struct {
    uint64_t trigram_bitset;
    uint32_t node_id;
    char text_content[128];
} TsfiDomSearchEntry;

typedef struct {
    TsfiDomSearchEntry entries[64];
    int entry_count;
} TsfiDomSearchIndex;

void tsfi_search_index_init(TsfiDomSearchIndex *idx);
bool tsfi_search_index_add_node(TsfiDomSearchIndex *idx, uint32_t node_id, const char *text);
int tsfi_search_index_query(const TsfiDomSearchIndex *idx, const char *query_text, uint32_t *out_node_ids, int max_matches);

// -----------------------------------------------------------------------------
// 4. URLSearchParams & RFC 3986 Query Encoder/Decoder
// -----------------------------------------------------------------------------
typedef struct {
    char key[64];
    char value[128];
} TsfiQueryParam;

typedef struct {
    TsfiQueryParam params[16];
    int param_count;
} TsfiUrlSearchParams;

void tsfi_url_search_init(TsfiUrlSearchParams *usp);
bool tsfi_url_search_parse(TsfiUrlSearchParams *usp, const char *query_str);
const char* tsfi_url_search_get(const TsfiUrlSearchParams *usp, const char *key);
bool tsfi_url_search_set(TsfiUrlSearchParams *usp, const char *key, const char *val);
size_t tsfi_url_search_serialize(const TsfiUrlSearchParams *usp, char *out_buf, size_t max_len);

#endif // TSFI_COMPOSITOR_CASCADE_SEARCH_URL_H
