#ifndef TSFI_ALGOL60_BLOCK_SCOPE_H
#define TSFI_ALGOL60_BLOCK_SCOPE_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t active_block_depth; // Current nested block depth (begin..end)
    uint32_t allocated_vars_cnt; // Stack-allocated variable count
    uint32_t thunk_evaluations;  // Call-by-name thunk evaluation count
    uint32_t evm_gas_cost;       // 450 Gas Cost per Block Frame
    double fet_power_watts;      // Power in Watts under 78.2% FET Drop
    char tape_dat_bin[128];      // Rule 13 Filename (.DAT.BIN)
} tsfi_algol60_block_summary_t;

/* Execute ALGOL 60 Block Entry, Call-by-Name Thunk, and Exit */
int tsfi_algol60_execute_block_frame(
    uint32_t target_depth,
    uint32_t var_declarations,
    tsfi_algol60_block_summary_t *out_summary
);

#endif // TSFI_ALGOL60_BLOCK_SCOPE_H
