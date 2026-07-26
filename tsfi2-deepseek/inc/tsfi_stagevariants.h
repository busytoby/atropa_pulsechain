#ifndef TSFI_STAGEVARIANTS_H
#define TSFI_STAGEVARIANTS_H

#include <stdint.h>
#include <stdbool.h>

#define TSFI_MAX_VARIANTS 4
#define TSFI_MAX_VARIANT_SETS 4

typedef struct {
    char variant_name[32];
    float scale_factor;
} TSFiVariantOption;

typedef struct {
    char set_name[32];
    TSFiVariantOption options[TSFI_MAX_VARIANTS];
    int option_count;
    char selected_option[32];
} TSFiVariantSet;

typedef struct {
    char prim_path[64];
    TSFiVariantSet sets[TSFI_MAX_VARIANT_SETS];
    int set_count;
} TSFiStageVariants;

// Initialize the StageVariants context
void tsfi_stagevariants_init(TSFiStageVariants *sv, const char *path);

// Defines a new variant set on the target primitive path
bool tsfi_stagevariants_define_set(TSFiStageVariants *sv, const char *set_name);

// Adds a variant choice option to an existing set
bool tsfi_stagevariants_add_option(TSFiStageVariants *sv, const char *set_name, const char *option_name, float scale_factor);

// Selects an active variant option within a variant set
bool tsfi_stagevariants_select_option(TSFiStageVariants *sv, const char *set_name, const char *option_name);

// Evaluates and returns the scale factor of the selected variant option
float tsfi_stagevariants_get_selected_scale(const TSFiStageVariants *sv, const char *set_name, float default_val);

#endif // TSFI_STAGEVARIANTS_H
