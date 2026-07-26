#include "tsfi_stagevariants.h"
#include <string.h>

void tsfi_stagevariants_init(TSFiStageVariants *sv, const char *path) {
    if (!sv) return;
    
    if (path) {
        strncpy(sv->prim_path, path, sizeof(sv->prim_path) - 1);
    } else {
        strcpy(sv->prim_path, "/World/Mesh");
    }
    
    sv->set_count = 0;
    memset(sv->sets, 0, sizeof(sv->sets));
}

bool tsfi_stagevariants_define_set(TSFiStageVariants *sv, const char *set_name) {
    if (!sv || !set_name || sv->set_count >= TSFI_MAX_VARIANT_SETS) return false;
    
    // Check duplicates
    for (int i = 0; i < sv->set_count; i++) {
        if (strcmp(sv->sets[i].set_name, set_name) == 0) {
            return true;
        }
    }
    
    TSFiVariantSet *set = &sv->sets[sv->set_count];
    strncpy(set->set_name, set_name, sizeof(set->set_name) - 1);
    set->option_count = 0;
    memset(set->selected_option, 0, sizeof(set->selected_option));
    
    sv->set_count++;
    return true;
}

bool tsfi_stagevariants_add_option(TSFiStageVariants *sv, const char *set_name, const char *option_name, float scale_factor) {
    if (!sv || !set_name || !option_name) return false;
    
    for (int i = 0; i < sv->set_count; i++) {
        TSFiVariantSet *set = &sv->sets[i];
        if (strcmp(set->set_name, set_name) == 0) {
            if (set->option_count >= TSFI_MAX_VARIANTS) return false;
            
            // Check duplicates
            for (int j = 0; j < set->option_count; j++) {
                if (strcmp(set->options[j].variant_name, option_name) == 0) {
                    set->options[j].scale_factor = scale_factor;
                    return true;
                }
            }
            
            TSFiVariantOption *opt = &set->options[set->option_count];
            strncpy(opt->variant_name, option_name, sizeof(opt->variant_name) - 1);
            opt->scale_factor = scale_factor;
            
            // Select first option by default
            if (set->option_count == 0) {
                strcpy(set->selected_option, option_name);
            }
            
            set->option_count++;
            return true;
        }
    }
    return false;
}

bool tsfi_stagevariants_select_option(TSFiStageVariants *sv, const char *set_name, const char *option_name) {
    if (!sv || !set_name || !option_name) return false;
    
    for (int i = 0; i < sv->set_count; i++) {
        TSFiVariantSet *set = &sv->sets[i];
        if (strcmp(set->set_name, set_name) == 0) {
            // Verify option exists
            for (int j = 0; j < set->option_count; j++) {
                if (strcmp(set->options[j].variant_name, option_name) == 0) {
                    strncpy(set->selected_option, option_name, sizeof(set->selected_option) - 1);
                    return true;
                }
            }
            return false;
        }
    }
    return false;
}

float tsfi_stagevariants_get_selected_scale(const TSFiStageVariants *sv, const char *set_name, float default_val) {
    if (!sv || !set_name) return default_val;
    
    for (int i = 0; i < sv->set_count; i++) {
        const TSFiVariantSet *set = &sv->sets[i];
        if (strcmp(set->set_name, set_name) == 0) {
            for (int j = 0; j < set->option_count; j++) {
                if (strcmp(set->options[j].variant_name, set->selected_option) == 0) {
                    return set->options[j].scale_factor;
                }
            }
        }
    }
    return default_val;
}
