#ifndef AUNCIENT_TIMELINE_AUTODIN_H
#define AUNCIENT_TIMELINE_AUTODIN_H

#include "../inc/auncient_sdk.h"
#include <stdint.h>
#include <stdbool.h>

// Auncient Timeline, AUTODIN, and Hogan Integration Interface

typedef struct {
    float timestamp;
    uint32_t event_id;
    uint32_t lock_token;
    char precedence; // 'F' for Fast/High, 'L' for Low
    bool triggered;
} TimelineEvent;

#ifdef __cplusplus
extern "C" {
#endif

// Processes timeline events and coordinates lock scheduling via AUTODIN precedence locks
void auncient_timeline_process(TimelineEvent *events, int count, float current_time, sdk_cics_context_t *ctx);

// Reconciles asset integrity with the Hogan transaction registry before staging to Vulkan
bool auncient_hogan_reconcile_asset(uint32_t asset_id, const uint8_t *dna_bytes, int size);

#ifdef __cplusplus
}
#endif

#endif // AUNCIENT_TIMELINE_AUTODIN_H
