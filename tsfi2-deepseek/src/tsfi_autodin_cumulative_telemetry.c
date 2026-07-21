#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_autodin_cumulative_telemetry.h"

int autodin_cumulative_telemetry_capture(
    const autodin_cumulative_permutation_engine_t *engine,
    autodin_cumulative_telemetry_snapshot_t *output_snapshot
) {
    if (!engine || !output_snapshot) return -1;

    memset(output_snapshot, 0, sizeof(autodin_cumulative_telemetry_snapshot_t));

    output_snapshot->channel_identifier = engine->channel_identifier;
    output_snapshot->cumulative_merkle_root_hash = engine->root_cumulative_hash;
    output_snapshot->total_field_effect_transistor_gas_consumed = engine->total_field_effect_transistor_gas;
    output_snapshot->processed_transaction_count = (uint32_t)engine->batch_count;
    output_snapshot->render_frame_rate_hertz = 60.0f;

    snprintf(output_snapshot->tape_label_identifier, sizeof(output_snapshot->tape_label_identifier),
             "ATD%06u", engine->channel_identifier);

    // Rule 13: Quadtree single-header storage layout strictly (.DAT.BIN)
    snprintf(output_snapshot->quadtree_binary_path, sizeof(output_snapshot->quadtree_binary_path),
             "%s", engine->quadtree_binary);

    return 0;
}

int autodin_cumulative_telemetry_format_display(
    const autodin_cumulative_telemetry_snapshot_t *snapshot,
    char *output_display_buffer,
    size_t maximum_buffer_length
) {
    if (!snapshot || !output_display_buffer || maximum_buffer_length == 0) return -1;

    int written_characters = snprintf(
        output_display_buffer,
        maximum_buffer_length,
        "================================================================================\n"
        "AUTOMATED DIGITAL NETWORK CUMULATIVE TRANSACTION STATE TELEMETRY\n"
        "================================================================================\n"
        "Channel Identifier                   : %u\n"
        "Tape Label Identifier                : %s\n"
        "Cumulative Merkle Root Hash          : 0x%016llX\n"
        "Processed Transaction Count          : %u\n"
        "Total Field-Effect Transistor Gas    : %llu\n"
        "Quadtree Storage File                : %s\n"
        "Auncient Virtual Hardware Frame Rate : %.2f Hertz\n"
        "================================================================================\n",
        snapshot->channel_identifier,
        snapshot->tape_label_identifier,
        (unsigned long long)snapshot->cumulative_merkle_root_hash,
        snapshot->processed_transaction_count,
        (unsigned long long)snapshot->total_field_effect_transistor_gas_consumed,
        snapshot->quadtree_binary_path,
        snapshot->render_frame_rate_hertz
    );

    return (written_characters > 0 && (size_t)written_characters < maximum_buffer_length) ? 0 : -1;
}
