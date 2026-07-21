#ifndef TSFI_PARC_TRUNK_QUEUE_H
#define TSFI_PARC_TRUNK_QUEUE_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    TRUNK_PRIORITY_FLASH     = 0, // SYS
    TRUNK_PRIORITY_IMMEDIATE = 1, // RDB
    TRUNK_PRIORITY_PRIORITY  = 2, // HDL
    TRUNK_PRIORITY_ROUTINE   = 3  // UNI / AST
} tsfi_trunk_priority_t;

typedef struct {
    uint32_t channel_id;
    tsfi_trunk_priority_t priority;
    char target_vol_id[7];
    uint32_t payload_bytes;
} tsfi_trunk_queue_item_t;

/* Enqueue a transaction packet into the AUTODIN multi-channel priority trunk queue */
int tsfi_trunk_queue_enqueue(uint32_t channel_id, const char *vol_id, uint32_t bytes, tsfi_trunk_queue_item_t *out_item);

/* Query priority tier for a channel ID directly from compiled Yul contract selector */
tsfi_trunk_priority_t tsfi_trunk_queue_get_priority(uint32_t channel_id);

#endif // TSFI_PARC_TRUNK_QUEUE_H
