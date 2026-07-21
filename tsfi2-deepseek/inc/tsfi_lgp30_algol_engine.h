#ifndef TSFI_LGP30_ALGOL_ENGINE_H
#define TSFI_LGP30_ALGOL_ENGINE_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t lgp30_id;
    uint32_t drum_words_used;           // 4096-Word Magnetic Drum Layout (31-bit words)
    uint32_t evm_gas_units;             // 280 Gas / Auncient Ether Units
    double fet_power_watts;             // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];             // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_lgp30_algol_engine_t;

/* Initialize LGP-30 Embedded ALGOL & SCALP Drum Memory Engine */
int tsfi_lgp30_algol_engine_init(
    uint32_t lgp30_id,
    tsfi_lgp30_algol_engine_t *engine
);

/* SCALP (Self-Contained ALGOL Processor) Load-and-Go Single-Pass Compiler */
int tsfi_lgp30_scalp_compile(
    tsfi_lgp30_algol_engine_t *engine,
    const char *algol_src,
    uint32_t *drum_words_out,
    size_t *words_len_out
);

/* Latency-Optimized Drum Memory Sector Alignment (16-Opcode Hardware Loop) */
int tsfi_lgp30_drum_optimize_latency(
    tsfi_lgp30_algol_engine_t *engine,
    uint32_t sector_track_mask,
    uint32_t *cycles_saved_out
);

#endif // TSFI_LGP30_ALGOL_ENGINE_H
