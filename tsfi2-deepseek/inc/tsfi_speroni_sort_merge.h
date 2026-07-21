#ifndef TSFI_SPERONI_SORT_MERGE_H
#define TSFI_SPERONI_SORT_MERGE_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t key;                      // Sort Key (2-3 Tree Node Key)
    uint32_t val;                      // Value Payload
} tsfi_speroni_sort_item_t;

typedef struct {
    uint32_t spool_id;                 // Spooler ID
    tsfi_speroni_sort_item_t items[16]; // Sort Array
    size_t count;                       // Item Count
    uint32_t evm_gas_units;             // 220 Gas / Auncient Ether Units
    double fet_power_watts;             // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];             // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_speroni_sort_merge_spooler_t;

/* Initialize Speroni Sort/Merge & Backing Store Spooler */
int tsfi_speroni_sort_merge_init(
    uint32_t spool_id,
    tsfi_speroni_sort_merge_spooler_t *spooler
);

/* Insert sort item into Speroni spooler array */
int tsfi_speroni_sort_merge_insert(
    tsfi_speroni_sort_merge_spooler_t *spooler,
    uint32_t key,
    uint32_t val
);

/* Execute Speroni inline sort/merge pass */
int tsfi_speroni_sort_merge_execute(
    tsfi_speroni_sort_merge_spooler_t *spooler
);

#endif // TSFI_SPERONI_SORT_MERGE_H
