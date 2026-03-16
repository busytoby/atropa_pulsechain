#include "tsfi_variations.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

#define READ_U16(p) (uint16_t)(((const uint8_t*)(p))[0] << 8 | ((const uint8_t*)(p))[1])
#define READ_U32(p) (uint32_t)((uint32_t)((const uint8_t*)(p))[0] << 24 | \
                               (uint32_t)((const uint8_t*)(p))[1] << 16 | \
                               (uint32_t)((const uint8_t*)(p))[2] << 8  | \
                               (uint32_t)((const uint8_t*)(p))[3])

static float f2dot14_to_float(int16_t val) {
    return (float)val / 16384.0f;
}

float tsfi_variation_evaluate_region(const tsfi_variation_region_t *region, const float *coords, int axis_count) {
    float scalar = 1.0f;
    for (int i = 0; i < axis_count; i++) {
        float start = region->axes[i].start;
        float peak  = region->axes[i].peak;
        float end   = region->axes[i].end;
        float coord = coords[i];

        if (start > peak || peak > end) continue; // Invalid region
        if (peak == 0 || coord == peak) continue;
        if (coord <= start || coord >= end) return 0.0f;

        if (coord < peak) {
            scalar *= (coord - start) / (peak - start);
        } else {
            scalar *= (end - coord) / (end - peak);
        }
    }
    return scalar;
}

float tsfi_variation_store_evaluate(const uint8_t *store_data, const float *coords, int axis_count, uint16_t outer_idx, uint16_t inner_idx) {
    if (!store_data) return 0.0f;

    uint16_t format = READ_U16(store_data);
    if (format != 1) return 0.0f;

    uint32_t region_list_off = READ_U32(store_data + 2);
    uint16_t data_count = READ_U16(store_data + 6);
    if (outer_idx >= data_count) return 0.0f;

    const uint8_t *region_list = store_data + region_list_off;
    uint16_t axis_count_store = READ_U16(region_list);
    uint16_t region_count = READ_U16(region_list + 2);
    const uint8_t *region_data = region_list + 4;

    // tsfi_io_printf(stdout, "[DEBUG] AxisCountStore: %d, RegionCount: %d\n", axis_count_store, region_count);

    uint32_t data_off = READ_U32(store_data + 8 + outer_idx * 4);
    const uint8_t *data_set = store_data + data_off;

    uint16_t item_count = READ_U16(data_set);
    uint16_t short_delta_count = READ_U16(data_set + 2);
    uint16_t region_idx_count = READ_U16(data_set + 4);
    
    // tsfi_io_printf(stdout, "[DEBUG] ItemCount: %d, RegionIdxCount: %d\n", item_count, region_idx_count);

    if (inner_idx >= item_count) return 0.0f;

    const uint8_t *indices_ptr = data_set + 6;
    const uint8_t *deltas_base = indices_ptr + region_idx_count * 2;
    uint32_t row_size = short_delta_count * 2 + (region_idx_count - short_delta_count);
    const uint8_t *row_ptr = deltas_base + inner_idx * row_size;

    float total_delta = 0.0f;
    for (uint16_t i = 0; i < region_idx_count; i++) {
        uint16_t ridx = READ_U16(indices_ptr + i * 2);
        if (ridx >= region_count) continue;

        // Evaluate region scalar
        float scalar = 1.0f;
        const uint8_t *this_region = region_data + ridx * (axis_count_store * 6);
        for (int a = 0; a < axis_count_store; a++) {
            if (a >= axis_count) { scalar = 0; break; }
            float start = f2dot14_to_float((int16_t)READ_U16(this_region + a * 6));
            float peak  = f2dot14_to_float((int16_t)READ_U16(this_region + a * 6 + 2));
            float end   = f2dot14_to_float((int16_t)READ_U16(this_region + a * 6 + 4));
            float coord = coords[a];

            if (start > peak || peak > end) continue;
            if (peak == 0 || coord == peak) continue;
            if (coord <= start || coord >= end) { scalar = 0; break; }
            if (coord < peak) scalar *= (coord - start) / (peak - start);
            else scalar *= (end - coord) / (end - peak);
        }

        int32_t delta;
        if (i < short_delta_count) {
            delta = (int16_t)READ_U16(row_ptr);
            row_ptr += 2;
        } else {
            delta = (int8_t)*row_ptr++;
        }

        if (scalar != 0) {
            // tsfi_io_printf(stdout, "  [DEBUG] Region %d Scalar: %.3f, Delta: %d\n", ridx, scalar, delta);
        }
        total_delta += scalar * (float)delta;
    }

    return total_delta;
}

bool tsfi_delta_set_index_map_lookup(const uint8_t *map_data, uint32_t logical_index, uint16_t *outer_idx, uint16_t *inner_idx) {
    if (!map_data) return false;

    uint8_t format = map_data[0];
    uint8_t entry_format = map_data[1];
    uint32_t map_count;
    const uint8_t *packed_data;

    if (format == 0) {
        map_count = READ_U16(map_data + 2);
        packed_data = map_data + 4;
    } else if (format == 1) {
        map_count = READ_U32(map_data + 2);
        packed_data = map_data + 6;
    } else {
        return false;
    }

    if (logical_index >= map_count) {
        // Use the last entry if index is out of bounds (per spec for mapping tables)
        logical_index = map_count - 1;
    }

    uint8_t inner_bits = (entry_format & 0x0F) + 1;
    uint8_t entry_size = ((entry_format & 0x30) >> 4) + 1;

    const uint8_t *p = packed_data + logical_index * entry_size;
    uint32_t val = 0;
    for (int i = 0; i < entry_size; i++) {
        val = (val << 8) | p[i];
    }

    *outer_idx = (uint16_t)(val >> inner_bits);
    *inner_idx = (uint16_t)(val & ((1 << inner_bits) - 1));

    return true;
}
