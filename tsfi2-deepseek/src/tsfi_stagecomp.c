#include "tsfi_stagecomp.h"
#include <string.h>

void tsfi_stagecomp_init(TSFiStageComp *comp) {
    if (!comp) return;
    for (int i = 0; i < MAX_LIVRPS_LAYERS; i++) {
        comp->layers[i].has_value = false;
        memset(comp->layers[i].val, 0, sizeof(comp->layers[i].val));
    }
}

void tsfi_stagecomp_set_opinion(TSFiStageComp *comp, LivrpsLayerType layer, const char *value) {
    if (!comp || layer >= MAX_LIVRPS_LAYERS) return;
    if (value) {
        strncpy(comp->layers[layer].val, value, sizeof(comp->layers[layer].val) - 1);
        comp->layers[layer].has_value = true;
    } else {
        comp->layers[layer].has_value = false;
    }
}

bool tsfi_stagecomp_resolve(const TSFiStageComp *comp, char *out_val) {
    if (!comp || !out_val) return false;
    
    // Evaluate layers from highest priority (LIVRPS_LOCAL) to lowest (LIVRPS_SPECIALIZES)
    // local > inherits > variantsets > references > payloads > specializes
    for (int i = MAX_LIVRPS_LAYERS - 1; i >= 0; i--) {
        if (comp->layers[i].has_value) {
            strcpy(out_val, comp->layers[i].val);
            return true;
        }
    }
    return false;
}

void tsfi_stagecomp_tune_pll(const TSFiStageComp *comp, tsfi_sage_pll_fabric *pll) {
    if (!comp || !pll) return;
    char resolved[64];
    if (tsfi_stagecomp_resolve(comp, resolved)) {
        if (strcmp(resolved, "high_frequency") == 0) {
            pll->reference_phase = 1000;
        } else if (strcmp(resolved, "low_frequency") == 0) {
            pll->reference_phase = 100;
        } else {
            pll->reference_phase = 500;
        }
    }
}
