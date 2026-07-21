#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_speroni_sort_merge.h"
#include "tsfi_lowpower_fet.h"

int tsfi_speroni_sort_merge_init(
    uint32_t spool_id,
    tsfi_speroni_sort_merge_spooler_t *spooler
) {
    if (!spooler) return -1;
    memset(spooler, 0, sizeof(tsfi_speroni_sort_merge_spooler_t));

    spooler->spool_id = spool_id;
    spooler->evm_gas_units = 220; // 220 Gas / Auncient Ether Units per sort/merge cycle

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    spooler->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(spooler->tape_dat_bin, sizeof(spooler->tape_dat_bin), "SPERONI_SORT_MERGE_%08X.DAT.BIN", spool_id);
    return 0;
}

int tsfi_speroni_sort_merge_insert(
    tsfi_speroni_sort_merge_spooler_t *spooler,
    uint32_t key,
    uint32_t val
) {
    if (!spooler || spooler->count >= 16) return -1;

    spooler->items[spooler->count].key = key;
    spooler->items[spooler->count].val = val;
    spooler->count++;

    return 0;
}

static int compare_items(const void *a, const void *b) {
    const tsfi_speroni_sort_item_t *ia = (const tsfi_speroni_sort_item_t *)a;
    const tsfi_speroni_sort_item_t *ib = (const tsfi_speroni_sort_item_t *)b;
    if (ia->key < ib->key) return -1;
    if (ia->key > ib->key) return 1;
    return 0;
}

int tsfi_speroni_sort_merge_execute(
    tsfi_speroni_sort_merge_spooler_t *spooler
) {
    if (!spooler || spooler->count == 0) return -1;

    // Execute Speroni balanced merge sort pass
    qsort(spooler->items, spooler->count, sizeof(tsfi_speroni_sort_item_t), compare_items);

    printf("[SPERONI SORT/MERGE] Spooler: %u | Sorted Items: %zu | Gas: %u | File: %s\n",
           spooler->spool_id, spooler->count, spooler->evm_gas_units, spooler->tape_dat_bin);

    for (size_t i = 0; i < spooler->count; i++) {
        printf("  Sorted [%zu]: Key 0x%08X -> Val 0x%08X\n", i, spooler->items[i].key, spooler->items[i].val);
    }

    return 0; // Sort/Merge pass successful
}
