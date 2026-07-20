#ifndef TSFI_PARC_BFS_H
#define TSFI_PARC_BFS_H

#include <stdint.h>

#define BFS_SECTOR_SIZE 512

typedef struct {
    uint16_t cylinder;
    uint8_t head;
    uint8_t sector;
} tsfi_parc_bfs_chs_t;

typedef struct {
    uint16_t next_page_link;
    uint16_t prev_page_link;
    uint16_t label_checksum;
    uint16_t data_checksum;
} tsfi_parc_bfs_label_t;

typedef struct {
    tsfi_parc_bfs_chs_t chs;
    tsfi_parc_bfs_label_t label;
    uint8_t data[BFS_SECTOR_SIZE];
} tsfi_parc_bfs_sector_t;

/* Initialize BFS sector layout with default linked page headers */
int tsfi_parc_bfs_init_sector(tsfi_parc_bfs_sector_t *sec, uint16_t cyl, uint8_t hd, uint8_t sc);

/* Calculate classic Alto 16-bit block checksums for sector labels and data */
uint16_t tsfi_parc_bfs_checksum(const uint8_t *buffer, int len);

#endif // TSFI_PARC_BFS_H
