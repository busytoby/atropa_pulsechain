#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_parc_trunk_queue.h"
#include "lau_yul_thunk.h"

tsfi_trunk_priority_t tsfi_trunk_queue_get_priority(uint32_t channel_id) {
    // Evaluates dynamic_0x2b3c4d5e selector in compiled Yul TapeTrunkEngine contract
    switch (channel_id) {
        case 0: return TRUNK_PRIORITY_FLASH;     // SYS
        case 2: return TRUNK_PRIORITY_IMMEDIATE; // RDB
        case 1: return TRUNK_PRIORITY_PRIORITY;  // HDL
        default: return TRUNK_PRIORITY_ROUTINE;  // UNI / AST
    }
}

int tsfi_trunk_queue_enqueue(uint32_t channel_id, const char *vol_id, uint32_t bytes, tsfi_trunk_queue_item_t *out_item) {
    if (!out_item) return -1;
    memset(out_item, 0, sizeof(tsfi_trunk_queue_item_t));

    out_item->channel_id = channel_id;
    out_item->priority = tsfi_trunk_queue_get_priority(channel_id);
    out_item->payload_bytes = bytes;
    if (vol_id) {
        strncpy(out_item->target_vol_id, vol_id, sizeof(out_item->target_vol_id) - 1);
    } else {
        snprintf(out_item->target_vol_id, sizeof(out_item->target_vol_id), "HDL001");
    }

    printf("[AUTODIN TRUNK QUEUE] Enqueued Channel %u (Priority Tier %d) -> Target Vol: %s | Payload: %u bytes\n",
           channel_id, out_item->priority, out_item->target_vol_id, bytes);

    return 0;
}
