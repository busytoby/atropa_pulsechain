#include "tsfi_hotloader.h"
#include "tsfi_font_shaping.h"
#include "tsfi_ottype.h"
#include "tsfi_ottype_internal.h"
#include "lau_memory.h"
#include "tsfi_wiring.h"
#include "tsfi_logic.h"
#include <string.h>

// A basic classification function for Marchen characters.
// In a full implementation, this would use a standard Unicode property table.
static bool is_marchen(uint32_t cp) {
    return (cp >= 0x11C70 && cp <= 0x11CB6);
}

// Forward declaration of the Marchen thunk interface.
// In TSFi, this is typically dynamically loaded or dispatched via logic tables.
extern void tsfi_shape_marchen_thunk(TSFiShapeCluster *cluster, tsfi_ottype_t *ot) __attribute__((weak));

static TSFiShapeCluster* create_wired_cluster(uint32_t script_id, const uint32_t *cps, int num_cps) {
    TSFiShapeCluster *cluster = lau_memalign_wired(512, sizeof(TSFiShapeCluster));
    if (!cluster) return NULL;

    LauSystemHeader *h = (LauSystemHeader *)((char *)cluster - offsetof(LauSystemHeader, payload));
    lau_mem_scramble(&h->regs, sizeof(LauRegisterBank), 0);
    h->resonance_as_status = lau_strdup("SHAPE_CLUSTER");
    
    lau_wire_system((WaveSystem*)cluster, h, tsfi_get_default_logic());

    cluster->script_id = script_id;
    cluster->num_codepoints = num_cps;
    cluster->codepoints = lau_malloc(num_cps * sizeof(uint32_t));
    memcpy(cluster->codepoints, cps, num_cps * sizeof(uint32_t));
    
    cluster->glyphs = NULL;
    cluster->num_glyphs = 0;
    
    cluster->cluster_transform = (tsfi_transform_t){ 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };
    cluster->advance_dx = 0.0f;
    cluster->advance_dy = 0.0f;
    
    return cluster;
}

int tsfi_font_shape_text(void *ot_ptr, const uint32_t *codepoints, int count, TSFiShapeCluster ***out_clusters) {
    tsfi_ottype_t *ot = (tsfi_ottype_t*)ot_ptr;
    if (count <= 0) return 0;

    // Temporary array to hold clusters before returning
    TSFiShapeCluster **clusters = lau_calloc(count, sizeof(TSFiShapeCluster*));
    int num_clusters = 0;

    // Simple greedy parser: groups adjacent characters of the same script.
    // Marchen is grouped into molecules (Syllables).
    int i = 0;
    while (i < count) {
        if (is_marchen(codepoints[i])) {
            int start = i;
            while (i < count && is_marchen(codepoints[i])) i++;
            
            TSFiShapeCluster *cluster = create_wired_cluster(TAG('m','a','r','c'), &codepoints[start], i - start);
            if (cluster) {
                // Dispatch to the Marchen shaping thunk
                
void (*marchen_func)(TSFiShapeCluster*, tsfi_ottype_t*) = (void (*)(TSFiShapeCluster*, tsfi_ottype_t*))tsfi_hotload_thunk("thunks/font_shaping/shape_marchen.c", "tsfi_shape_marchen_thunk");
if (marchen_func) marchen_func(cluster, ot);

                clusters[num_clusters++] = cluster;
            }
        } else {
            // Basic Latin or unhandled scripts - grouped one by one for now
            TSFiShapeCluster *cluster = create_wired_cluster(TAG('l','a','t','n'), &codepoints[i], 1);
            if (cluster) {
                // Fallback basic layout
                cluster->num_glyphs = 1;
                cluster->glyphs = lau_memalign_wired(512, sizeof(TSFiShapedGlyph));
                
                LauSystemHeader *h = (LauSystemHeader *)((char *)cluster->glyphs - offsetof(LauSystemHeader, payload));
                lau_mem_scramble(&h->regs, sizeof(LauRegisterBank), 0);
                h->resonance_as_status = lau_strdup("SHAPED_GLYPH");
                lau_wire_system((WaveSystem*)cluster->glyphs, h, tsfi_get_default_logic());

                cluster->glyphs[0].glyph_index = tsfi_ottype_get_glyph_index(ot, codepoints[i], 0);
                cluster->glyphs[0].cluster_index = i;
                cluster->glyphs[0].transform = (tsfi_transform_t){ 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };

                int adv, lsb;
                if (tsfi_ottype_get_glyph_metrics(ot, cluster->glyphs[0].glyph_index, &adv, &lsb)) {
                    cluster->advance_dx = (float)adv;
                } else {
                    cluster->advance_dx = 0.0f;
                }
                cluster->advance_dy = 0.0f;
                
                clusters[num_clusters++] = cluster;
            }
            i++;
        }
    }

    *out_clusters = clusters;
    return num_clusters;
}

void tsfi_font_free_clusters(TSFiShapeCluster **clusters, int num_clusters) {
    if (!clusters) return;
    for (int i = 0; i < num_clusters; i++) {
        if (clusters[i]) {
            if (clusters[i]->codepoints) lau_free(clusters[i]->codepoints);
            if (clusters[i]->glyphs) {
                LauSystemHeader *hg = (LauSystemHeader *)((char *)clusters[i]->glyphs - offsetof(LauSystemHeader, payload));
                if (hg->resonance_as_status) { lau_free(hg->resonance_as_status); hg->resonance_as_status = NULL; }
                lau_unseal_object(clusters[i]->glyphs);
                lau_free(clusters[i]->glyphs); // This frees the entire wired block
            }
            LauSystemHeader *hc = (LauSystemHeader *)((char *)clusters[i] - offsetof(LauSystemHeader, payload));
            if (hc->resonance_as_status) { lau_free(hc->resonance_as_status); hc->resonance_as_status = NULL; }
            lau_unseal_object(clusters[i]);
            lau_free(clusters[i]);
        }
    }
    lau_free(clusters);
}
