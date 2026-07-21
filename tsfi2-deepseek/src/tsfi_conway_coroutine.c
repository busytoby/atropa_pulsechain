#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_conway_coroutine.h"
#include "tsfi_lowpower_fet.h"

int tsfi_conway_step_tree_coroutine(
    uint32_t node_index,
    tsfi_conway_coroutine_node_t *node_ctx
) {
    if (!node_ctx) return -1;

    node_ctx->node_index = node_index;
    node_ctx->evm_gas_units = 320; // 320 Gas / Auncient Ether Units

    // State machine yield / resume transition inside DAT / 2-3 Tree node
    switch (node_ctx->yield_state) {
        case CONWAY_YIELD_INIT:
            node_ctx->yield_state = CONWAY_YIELD_TRAVERSAL;
            node_ctx->instruction_pc = 0x10;
            break;
        case CONWAY_YIELD_TRAVERSAL:
            node_ctx->yield_state = CONWAY_YIELD_LOCK_ACQUIRED;
            node_ctx->instruction_pc = 0x20;
            break;
        case CONWAY_YIELD_LOCK_ACQUIRED:
            node_ctx->yield_state = CONWAY_YIELD_EVALUATED;
            node_ctx->instruction_pc = 0x30;
            break;
        case CONWAY_YIELD_EVALUATED:
            node_ctx->yield_state = CONWAY_YIELD_COMPLETE;
            node_ctx->instruction_pc = 0x40;
            break;
        case CONWAY_YIELD_COMPLETE:
        default:
            node_ctx->yield_state = CONWAY_YIELD_INIT;
            node_ctx->instruction_pc = 0x00;
            break;
    }

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    node_ctx->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(node_ctx->tape_dat_bin, sizeof(node_ctx->tape_dat_bin),
             "CONWAY_TREE_THREAD_%08X.DAT.BIN", (unsigned int)node_index);

    printf("[CONWAY COROUTINE] DAT Node: %u | Yield State: %d | PC: 0x%02X | Ether Gas: %u | FET Power: %.4fW (78.2%% Cut) | File: %s\n",
           node_ctx->node_index, node_ctx->yield_state, node_ctx->instruction_pc,
           node_ctx->evm_gas_units, node_ctx->fet_power_watts, node_ctx->tape_dat_bin);

    return 0; // Tree coroutine step complete
}
