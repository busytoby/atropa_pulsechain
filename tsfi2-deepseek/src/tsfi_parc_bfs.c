#include <string.h>
#include "tsfi_parc_bfs.h"

uint16_t tsfi_parc_bfs_checksum(const uint8_t *buffer, int len) {
    if (!buffer || len <= 0) return 0;
    uint32_t sum = 0;
    for (int i = 0; i < len - 1; i += 2) {
        sum += (buffer[i] << 8) | buffer[i + 1];
    }
    if (len & 1) {
        sum += (buffer[len - 1] << 8);
    }
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    return (uint16_t)~sum;
}

int tsfi_parc_bfs_init_sector(tsfi_parc_bfs_sector_t *sec, uint16_t cyl, uint8_t hd, uint8_t sc) {
    if (!sec) return -1;
    if (cyl >= 203 || hd >= 2 || sc >= 12) return -2;
    sec->chs.cylinder = cyl;
    sec->chs.head = hd;
    sec->chs.sector = sc;

    sec->label.next_page_link = sc + 1;
    sec->label.prev_page_link = sc > 0 ? sc - 1 : 0xFFFF;
    
    memset(sec->data, 0, BFS_SECTOR_SIZE);
    
    sec->label.data_checksum = tsfi_parc_bfs_checksum(sec->data, BFS_SECTOR_SIZE);
    
    uint8_t temp[8];
    temp[0] = (sec->label.next_page_link >> 8) & 0xFF;
    temp[1] = sec->label.next_page_link & 0xFF;
    temp[2] = (sec->label.prev_page_link >> 8) & 0xFF;
    temp[3] = sec->label.prev_page_link & 0xFF;
    temp[4] = (sec->label.data_checksum >> 8) & 0xFF;
    temp[5] = sec->label.data_checksum & 0xFF;
    temp[6] = 0; temp[7] = 0;
    
    sec->label.label_checksum = tsfi_parc_bfs_checksum(temp, 8);
    return 0;
}
