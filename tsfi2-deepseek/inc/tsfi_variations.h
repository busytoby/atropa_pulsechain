#ifndef TSFI_VARIATIONS_H
#define TSFI_VARIATIONS_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float start;
    float peak;
    float end;
} tsfi_region_axis_t;

typedef struct {
    tsfi_region_axis_t *axes; // [axis_count]
} tsfi_variation_region_t;

typedef struct {
    uint16_t region_count;
    uint16_t *region_indices; // [region_count]
    const uint8_t *delta_data;
    uint16_t item_count;
    uint16_t short_delta_count;
} tsfi_item_variation_data_t;

typedef struct {
    uint16_t axis_count;
    uint16_t region_count;
    tsfi_variation_region_t *regions; // [region_count]
    uint16_t data_count;
    tsfi_item_variation_data_t *data_sets; // [data_count]
} tsfi_item_variation_store_t;

float tsfi_variation_evaluate_region(const tsfi_variation_region_t *region, const float *coords, int axis_count);
float tsfi_variation_store_evaluate(const uint8_t *store_data, const float *coords, int axis_count, uint16_t outer_idx, uint16_t inner_idx);
bool tsfi_delta_set_index_map_lookup(const uint8_t *map_data, uint32_t logical_index, uint16_t *outer_idx, uint16_t *inner_idx);

#endif
