#include "tsfi_parc_schrack.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

/* Dilate lower 32 bits into 64 bits with alternating zeros */
static uint64_t schrack_dilate_32(uint32_t value) {
    uint64_t x = value;
    x = (x | (x << 16)) & 0x0000FFFF0000FFFFULL;
    x = (x | (x << 8))  & 0x00FF00FF00FF00FFULL;
    x = (x | (x << 4))  & 0x0F0F0F0F0F0F0F0FULL;
    x = (x | (x << 2))  & 0x3333333333333333ULL;
    x = (x | (x << 1))  & 0x5555555555555555ULL;
    return x;
}

/* Contract 64 bits back to 32 bits by extracting odd/even bits */
static uint32_t schrack_contract_64(uint64_t value) {
    uint64_t x = value & 0x5555555555555555ULL;
    x = (x | (x >> 1))  & 0x3333333333333333ULL;
    x = (x | (x >> 2))  & 0x0F0F0F0F0F0F0F0FULL;
    x = (x | (x >> 4))  & 0x00FF00FF00FF00FFULL;
    x = (x | (x >> 8))  & 0x0000FFFF0000FFFFULL;
    x = (x | (x >> 16)) & 0x00000000FFFFFFFFULL;
    return (uint32_t)x;
}

uint64_t tsfi_schrack_integer_dilation(uint32_t coordinate_x, uint32_t coordinate_y) {
    uint64_t dilated_x = schrack_dilate_32(coordinate_x);
    uint64_t dilated_y = schrack_dilate_32(coordinate_y);
    return dilated_x | (dilated_y << 1);
}

void tsfi_schrack_integer_contraction(uint64_t quadcode, uint32_t *coordinate_x_out, uint32_t *coordinate_y_out) {
    if (coordinate_x_out) {
        *coordinate_x_out = schrack_contract_64(quadcode);
    }
    if (coordinate_y_out) {
        *coordinate_y_out = schrack_contract_64(quadcode >> 1);
    }
}

int tsfi_schrack_find_equal_neighbor(
    uint64_t input_quadcode,
    uint32_t quadtree_depth,
    tsfi_schrack_direction_t direction,
    uint64_t *neighbor_quadcode_out
) {
    if (!neighbor_quadcode_out || quadtree_depth == 0 || quadtree_depth > 32) return -1;

    uint32_t cur_x = 0;
    uint32_t cur_y = 0;
    tsfi_schrack_integer_contraction(input_quadcode, &cur_x, &cur_y);

    int32_t target_x = (int32_t)cur_x;
    int32_t target_y = (int32_t)cur_y;

    switch (direction) {
        case SCHRACK_DIR_NORTH:
            target_y -= 1;
            break;
        case SCHRACK_DIR_SOUTH:
            target_y += 1;
            break;
        case SCHRACK_DIR_EAST:
            target_x += 1;
            break;
        case SCHRACK_DIR_WEST:
            target_x -= 1;
            break;
        case SCHRACK_DIR_NORTH_EAST:
            target_x += 1;
            target_y -= 1;
            break;
        case SCHRACK_DIR_NORTH_WEST:
            target_x -= 1;
            target_y -= 1;
            break;
        case SCHRACK_DIR_SOUTH_EAST:
            target_x += 1;
            target_y += 1;
            break;
        case SCHRACK_DIR_SOUTH_WEST:
            target_x -= 1;
            target_y += 1;
            break;
    }

    uint32_t max_coord = (1U << quadtree_depth) - 1;

    if (target_x < 0 || target_x > (int32_t)max_coord || target_y < 0 || target_y > (int32_t)max_coord) {
        return -2; // Boundary Out of Bounds
    }

    *neighbor_quadcode_out = tsfi_schrack_integer_dilation((uint32_t)target_x, (uint32_t)target_y);
    return 0;
}

uint64_t tsfi_schrack_morton_to_hilbert(uint64_t morton_code, uint32_t quadtree_depth) {
    if (quadtree_depth == 0) return morton_code;

    uint32_t x = 0;
    uint32_t y = 0;
    tsfi_schrack_integer_contraction(morton_code, &x, &y);

    uint64_t d = 0;
    uint32_t n = 1U << quadtree_depth;

    uint32_t rx, ry;
    for (uint32_t s = n / 2; s > 0; s /= 2) {
        rx = (x & s) > 0;
        ry = (y & s) > 0;
        d += (uint64_t)s * (uint64_t)s * ((3 * rx) ^ ry);

        if (ry == 0) {
            if (rx == 1) {
                x = s - 1 - x;
                y = s - 1 - y;
            }
            uint32_t temp = x;
            x = y;
            y = temp;
        }
    }

    return d;
}

int tsfi_schrack_verify_quadtree_slice_asset(const char *quadtree_bin_filepath) {
    if (!quadtree_bin_filepath) return -1;

    size_t len = strlen(quadtree_bin_filepath);
    if (len < 8) return -2;

    // Rule 13: Must end strictly with .dat.bin extension (case-insensitive check)
    const char *ext = quadtree_bin_filepath + len - 8;
    if (strcasecmp(ext, ".dat.bin") != 0) {
        printf("[SCHRACK RULE 13 VIOLATION] File '%s' violates Quadtree storage extension rule! Must end strictly with .dat.bin!\n", quadtree_bin_filepath);
        return -3;
    }

    FILE *file = fopen(quadtree_bin_filepath, "rb");
    if (!file) {
        return -4; // File not readable
    }
    fclose(file);

    return 0;
}
