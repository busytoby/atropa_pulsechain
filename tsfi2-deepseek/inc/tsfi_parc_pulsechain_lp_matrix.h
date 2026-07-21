#ifndef TSFI_PARC_PULSECHAIN_LP_MATRIX_H
#define TSFI_PARC_PULSECHAIN_LP_MATRIX_H

#include "tsfi_parc_ingerman_alg141.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define TSFI_PULSECHAIN_MAX_TOKENS 16
#define TSFI_PULSECHAIN_MAX_POOLS  32

/* PulseChain Token Node Definition */
typedef struct {
    uint32_t token_id;
    char symbol[16];
    char contract_address[64];
} tsfi_pulsechain_token_t;

/* PulseChain Liquidity Pool Edge Definition */
typedef struct {
    uint32_t pool_id;
    uint32_t token_a_id;
    uint32_t token_b_id;
    char pair_address[64];
    uint64_t reserve_a;
    uint64_t reserve_b;
} tsfi_pulsechain_lp_pool_t;

/* PulseChain LP Reachability & Path Matrix State Machine */
typedef struct {
    bool is_initialized;
    uint32_t token_count;
    tsfi_pulsechain_token_t tokens[TSFI_PULSECHAIN_MAX_TOKENS];
    uint32_t pool_count;
    tsfi_pulsechain_lp_pool_t pools[TSFI_PULSECHAIN_MAX_POOLS];
    tsfi_ingerman_alg141_t ingerman_graph;
    char dynamic_router_address[64];
} tsfi_pulsechain_lp_matrix_t;

/* Initialize PulseChain LP Path Matrix Engine */
int tsfi_pulsechain_lp_matrix_initialize(tsfi_pulsechain_lp_matrix_t *lp_matrix);

/* Register PulseChain Token Node */
int tsfi_pulsechain_lp_matrix_register_token(tsfi_pulsechain_lp_matrix_t *lp_matrix, const char *symbol, const char *address, uint32_t *token_id_out);

/* Register PulseChain Liquidity Pool Edge */
int tsfi_pulsechain_lp_matrix_register_pool(tsfi_pulsechain_lp_matrix_t *lp_matrix, uint32_t token_a, uint32_t token_b, const char *pair_address, uint64_t reserve_a, uint64_t reserve_b);

/* Compute Full PulseChain LP Reachability Matrix (Ingerman Alg 141) */
int tsfi_pulsechain_lp_matrix_compute_closure(tsfi_pulsechain_lp_matrix_t *lp_matrix);

/* Query Multihop Swap Reachability Between Tokens */
bool tsfi_pulsechain_lp_matrix_can_swap(const tsfi_pulsechain_lp_matrix_t *lp_matrix, uint32_t src_token, uint32_t dest_token);

/* Find Arbitrage Loop Cycle (Token -> ... -> Token) */
bool tsfi_pulsechain_lp_matrix_has_arbitrage_loop(const tsfi_pulsechain_lp_matrix_t *lp_matrix, uint32_t token_id);

/* Render LP Matrix Diagnostic Report */
int tsfi_pulsechain_lp_matrix_render_report(const tsfi_pulsechain_lp_matrix_t *lp_matrix, char *out_buf, size_t max_buf_len);

#endif // TSFI_PARC_PULSECHAIN_LP_MATRIX_H
