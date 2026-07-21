#ifndef TSFI_AUTODIN_CUMULATIVE_TELEMETRY_H
#define TSFI_AUTODIN_CUMULATIVE_TELEMETRY_H

#include <stdint.h>
#include <stddef.h>
#include "tsfi_autodin_cumulative_permutation.h"

typedef struct {
    uint32_t channel_identifier;
    uint64_t cumulative_merkle_root_hash;
    uint64_t total_field_effect_transistor_gas_consumed;
    uint32_t processed_transaction_count;
    float render_frame_rate_hertz;
    char tape_label_identifier[32];
    char quadtree_binary_path[128]; // Single-Header Array (.DAT.BIN - Rule 13)
} autodin_cumulative_telemetry_snapshot_t;

/* Capture live telemetry snapshot from Automated Digital Network cumulative transaction state permutation engine */
int autodin_cumulative_telemetry_capture(
    const autodin_cumulative_permutation_engine_t *engine,
    autodin_cumulative_telemetry_snapshot_t *output_snapshot
);

/* Format unabbreviated telemetry snapshot string for Vulkan terminal display */
int autodin_cumulative_telemetry_format_display(
    const autodin_cumulative_telemetry_snapshot_t *snapshot,
    char *output_display_buffer,
    size_t maximum_buffer_length
);

#endif // TSFI_AUTODIN_CUMULATIVE_TELEMETRY_H
