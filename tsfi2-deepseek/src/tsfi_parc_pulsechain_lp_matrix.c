#include "tsfi_parc_pulsechain_lp_matrix.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int tsfi_pulsechain_lp_matrix_initialize(tsfi_pulsechain_lp_matrix_t *lp_matrix) {
    if (!lp_matrix) return -1;
    memset(lp_matrix, 0, sizeof(tsfi_pulsechain_lp_matrix_t));

    lp_matrix->is_initialized = true;
    snprintf(lp_matrix->dynamic_router_address, sizeof(lp_matrix->dynamic_router_address),
             "dynamic_0x981325d7e5dcf4a7ff95c6439c099391ab1a1290");

    return 0;
}

int tsfi_pulsechain_lp_matrix_register_token(tsfi_pulsechain_lp_matrix_t *lp_matrix, const char *symbol, const char *address, uint32_t *token_id_out) {
    if (!lp_matrix || !symbol || !address) return -1;
    if (lp_matrix->token_count >= TSFI_PULSECHAIN_MAX_TOKENS) return -2;

    uint32_t id = lp_matrix->token_count++;
    tsfi_pulsechain_token_t *tok = &lp_matrix->tokens[id];
    tok->token_id = id;
    snprintf(tok->symbol, sizeof(tok->symbol), "%s", symbol);
    snprintf(tok->contract_address, sizeof(tok->contract_address), "%s", address);

    if (token_id_out) *token_id_out = id;
    return 0;
}

int tsfi_pulsechain_lp_matrix_register_pool(tsfi_pulsechain_lp_matrix_t *lp_matrix, uint32_t token_a, uint32_t token_b, const char *pair_address, uint64_t reserve_a, uint64_t reserve_b) {
    if (!lp_matrix || token_a >= lp_matrix->token_count || token_b >= lp_matrix->token_count) return -1;
    if (lp_matrix->pool_count >= TSFI_PULSECHAIN_MAX_POOLS) return -2;

    uint32_t pid = lp_matrix->pool_count++;
    tsfi_pulsechain_lp_pool_t *pool = &lp_matrix->pools[pid];
    pool->pool_id = pid;
    pool->token_a_id = token_a;
    pool->token_b_id = token_b;
    pool->reserve_a = reserve_a;
    pool->reserve_b = reserve_b;
    snprintf(pool->pair_address, sizeof(pool->pair_address), "%s", pair_address ? pair_address : "dynamic_0x00");

    return 0;
}

int tsfi_pulsechain_lp_matrix_compute_closure(tsfi_pulsechain_lp_matrix_t *lp_matrix) {
    if (!lp_matrix) return -1;

    // Initialize Ingerman Graph with token count
    if (tsfi_ingerman_alg141_initialize(&lp_matrix->ingerman_graph, lp_matrix->token_count) != 0) {
        return -2;
    }

    // Populate directed edges for bidirectional LP swaps
    for (uint32_t i = 0; i < lp_matrix->pool_count; i++) {
        uint32_t a = lp_matrix->pools[i].token_a_id;
        uint32_t b = lp_matrix->pools[i].token_b_id;
        tsfi_ingerman_alg141_add_edge(&lp_matrix->ingerman_graph, a, b);
        tsfi_ingerman_alg141_add_edge(&lp_matrix->ingerman_graph, b, a);
    }

    // Compute Transitive Closure via Ingerman Algorithm 141 (CACM 1962)
    return tsfi_ingerman_alg141_compute_path_matrix(&lp_matrix->ingerman_graph);
}

bool tsfi_pulsechain_lp_matrix_can_swap(const tsfi_pulsechain_lp_matrix_t *lp_matrix, uint32_t src_token, uint32_t dest_token) {
    if (!lp_matrix) return false;
    return tsfi_ingerman_alg141_is_reachable(&lp_matrix->ingerman_graph, src_token, dest_token);
}

bool tsfi_pulsechain_lp_matrix_has_arbitrage_loop(const tsfi_pulsechain_lp_matrix_t *lp_matrix, uint32_t token_id) {
    if (!lp_matrix || token_id >= lp_matrix->token_count) return false;
    return tsfi_ingerman_alg141_is_reachable(&lp_matrix->ingerman_graph, token_id, token_id);
}

int tsfi_pulsechain_lp_matrix_render_report(const tsfi_pulsechain_lp_matrix_t *lp_matrix, char *out_buf, size_t max_buf_len) {
    if (!lp_matrix || !out_buf || max_buf_len < 512) return -1;

    size_t written = 0;
    written += snprintf(out_buf + written, max_buf_len - written,
                         "=======================================================\n"
                         " PULSECHAIN LP POOL INGERMAN PATH MATRIX REPORT        \n"
                         " Router Address: %s                                     \n"
                         " Total Tokens: %u | Total LP Pools: %u                  \n"
                         "=======================================================\n",
                         lp_matrix->dynamic_router_address, lp_matrix->token_count, lp_matrix->pool_count);

    for (uint32_t i = 0; i < lp_matrix->token_count; i++) {
        written += snprintf(out_buf + written, max_buf_len - written, "[Token %u: %s] Reachable targets: ",
                             i, lp_matrix->tokens[i].symbol);
        for (uint32_t j = 0; j < lp_matrix->token_count; j++) {
            if (tsfi_ingerman_alg141_is_reachable(&lp_matrix->ingerman_graph, i, j)) {
                written += snprintf(out_buf + written, max_buf_len - written, "%s ", lp_matrix->tokens[j].symbol);
            }
        }
        written += snprintf(out_buf + written, max_buf_len - written, "\n");
    }

    return 0;
}
