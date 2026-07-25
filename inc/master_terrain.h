#ifndef MASTER_TERRAIN_H
#define MASTER_TERRAIN_H

#include <stdint.h>
#include <stdbool.h>

/* Master Terrain Model configurations */
#define TERRAIN_GRID_SIZE 64
#define TERRAIN_MAX_HEIGHT 255

typedef struct {
    uint8_t latitude_bytes[32];
    uint8_t longitude_bytes[32];
    uint8_t height_grid[TERRAIN_GRID_SIZE][TERRAIN_GRID_SIZE];
    uint8_t physical_bank;
} terrain_cell_t;

/* Initialize terrain system */
bool master_terrain_init(void);

/* Load terrain cell from Hecke map seed */
bool master_terrain_load_cell(uint64_t waat, terrain_cell_t *out_cell);

/* Map terrain bank to HuC MMU page index */
bool master_terrain_map_mmu(uint8_t mpr_index, const terrain_cell_t *cell);

/* Stieber data reduction scan: Compresses terrain cells based on elevation contours */
bool master_terrain_stieber_reduction(const terrain_cell_t *cell, uint8_t *out_compressed, uint32_t *out_size);

#endif /* MASTER_TERRAIN_H */
