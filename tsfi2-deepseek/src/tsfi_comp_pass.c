#include "tsfi_comp_pass.h"
#include <string.h>

void tsfi_comp_pass_init(TSFiCompPass *pass) {
    if (!pass) return;
    pass->layer_count = 0;
    memset(pass->layers, 0, sizeof(pass->layers));
}

void tsfi_comp_pass_add_layer(TSFiCompPass *pass, float r, float g, float b, float alpha) {
    if (!pass || pass->layer_count >= MAX_COMP_LAYERS) return;
    int idx = pass->layer_count++;
    pass->layers[idx].color[0] = r;
    pass->layers[idx].color[1] = g;
    pass->layers[idx].color[2] = b;
    pass->layers[idx].alpha = alpha;
}

void tsfi_comp_pass_resolve(const TSFiCompPass *pass, float *out_color) {
    if (!pass || !out_color) return;
    out_color[0] = 0.0f;
    out_color[1] = 0.0f;
    out_color[2] = 0.0f;
    
    int n = pass->layer_count;
    for (int l = 0; l < n; l++) {
        float product = 1.0f;
        for (int j = l + 1; j < n; j++) {
            product *= pass->layers[j].alpha;
        }
        float weight = 1.0f - product;
        
        // Accumulate composited colors
        out_color[0] += pass->layers[l].color[0] * weight;
        out_color[1] += pass->layers[l].color[1] * weight;
        out_color[2] += pass->layers[l].color[2] * weight;
    }
}

void tsfi_comp_pass_load_cactus_density(TSFiCompPass *pass, const usd_auncient_cactus_schema_t *cactus) {
    if (!pass || !cactus) return;
    tsfi_comp_pass_add_layer(pass, 0.1f, 0.8f, 0.2f, cactus->density);
}
