#ifndef TSFI_HOGAN_TAX_PERMUTATION_H
#define TSFI_HOGAN_TAX_PERMUTATION_H

#include <stdint.h>
#include <stddef.h>
#include "tsfi_hogan.h"

#define MAX_TAX_LOTS 64

typedef enum {
    HOGAN_TAX_LOT_FIFO = 0,
    HOGAN_TAX_LOT_LIFO = 1,
    HOGAN_TAX_LOT_HIFO = 2,
    HOGAN_TAX_LOT_SPECIFIC_ID = 3
} hogan_tax_method_t;

typedef struct {
    uint32_t lot_id;
    uint64_t acquisition_timestamp;
    uint64_t amount_microcents;
    uint64_t cost_basis_microcents;
    char token_address[42];             // dynamic_<address> - Rule 9
    uint8_t is_disposed;
} hogan_tax_lot_t;

typedef struct {
    uint32_t account_id;
    hogan_tax_lot_t lots[MAX_TAX_LOTS];
    size_t lot_count;
    uint64_t total_realized_gain_microcents;
    uint64_t total_realized_loss_microcents;
    uint32_t evm_gas_units;             // 280 Gas / Auncient Ether Units
    double fet_power_watts;             // 0.0109 W under 78.2% Power Cut
    char quadtree_bin[128];             // Single-Header Array (.DAT.BIN - Rule 13)
} hogan_tax_permutation_engine_t;

/* Initialize Hogan Tax Lot Permutation Engine */
int tsfi_hogan_tax_engine_init(
    uint32_t account_id,
    hogan_tax_permutation_engine_t *engine
);

/* Add acquired asset tax lot to Hogan account */
int tsfi_hogan_add_tax_lot(
    hogan_tax_permutation_engine_t *engine,
    uint64_t amount_microcents,
    uint64_t cost_basis_microcents,
    const char *token_address
);

/* Permute and execute asset disposal across selected IRS cost-basis strategy */
int tsfi_hogan_permute_disposal(
    hogan_tax_permutation_engine_t *engine,
    uint64_t dispose_amount_microcents,
    uint64_t proceeds_microcents,
    hogan_tax_method_t strategy,
    uint64_t *out_realized_gain_loss
);

/* Verify WinchesterMQ SCSI keycode register state permutations (Keycode 32 / 30 - Rule 5) */
int tsfi_hogan_verify_winchester_scsi_permutations(
    hogan_tax_permutation_engine_t *engine,
    uint8_t keycode,
    int *out_valid
);

#endif // TSFI_HOGAN_TAX_PERMUTATION_H
