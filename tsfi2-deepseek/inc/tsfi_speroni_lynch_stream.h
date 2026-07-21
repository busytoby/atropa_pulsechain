#ifndef TSFI_SPERONI_LYNCH_STREAM_H
#define TSFI_SPERONI_LYNCH_STREAM_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t stream_id;                 // Speroni-Lynch Stream ID
    uint8_t stream_buffer[720];         // 720-byte 8-block Yul Tape Header Buffer
    size_t write_pos;                   // Current Write Position
    uint32_t evm_gas_units;             // 230 Gas / Auncient Ether Units
    double fet_power_watts;             // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];             // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_speroni_lynch_stream_t;

/* Initialize Speroni-Lynch Case ALGOL Stream I/O Channel */
int tsfi_speroni_lynch_stream_init(
    uint32_t stream_id,
    tsfi_speroni_lynch_stream_t *stream
);

/* Write bytes into Speroni-Lynch stream channel */
int tsfi_speroni_lynch_stream_write(
    tsfi_speroni_lynch_stream_t *stream,
    const uint8_t *data,
    size_t len
);

#endif // TSFI_SPERONI_LYNCH_STREAM_H
