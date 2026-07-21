#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_speroni_lynch_stream.h"
#include "tsfi_lowpower_fet.h"

int tsfi_speroni_lynch_stream_init(
    uint32_t stream_id,
    tsfi_speroni_lynch_stream_t *stream
) {
    if (!stream) return -1;
    memset(stream, 0, sizeof(tsfi_speroni_lynch_stream_t));

    stream->stream_id = stream_id;
    stream->evm_gas_units = 230; // 230 Gas / Auncient Ether Units per stream write slot

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    stream->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(stream->tape_dat_bin, sizeof(stream->tape_dat_bin), "SPERONI_LYNCH_STREAM_%08X.DAT.BIN", stream_id);
    return 0;
}

int tsfi_speroni_lynch_stream_write(
    tsfi_speroni_lynch_stream_t *stream,
    const uint8_t *data,
    size_t len
) {
    if (!stream || !data || stream->write_pos + len > sizeof(stream->stream_buffer)) return -1;

    memcpy(&stream->stream_buffer[stream->write_pos], data, len);
    stream->write_pos += len;

    printf("[SPERONI-LYNCH STREAM] Stream: %u | Wrote: %zu Bytes | Total Pos: %zu/720 | Gas: %u | File: %s\n",
           stream->stream_id, len, stream->write_pos, stream->evm_gas_units, stream->tape_dat_bin);

    return 0; // Stream write successful
}
