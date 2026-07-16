#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_algebraic_query.h"

int main(void) {
    printf("[Query Compiler] Starting algebraic query parser and evaluation tests...\n");
    fflush(stdout);

    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    
    // Setup event weights
    tsfi_trie_insert(trie_root, "query/holds_token", "60"); // P(A) = 0.60
    tsfi_trie_insert(trie_root, "query/active", "50");      // P(B) = 0.50
    tsfi_trie_insert(trie_root, "query/signature", "70");   // P(C) = 0.70

    tsfi_dat *dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    // Build query representation: (holds_token AND active) OR signature
    // Algebraic expression: (A * B) + C - (A * B * C)
    // Expected probability:
    // (0.60 * 0.50) = 0.30
    // 0.30 + 0.70 - (0.30 * 0.70) = 1.00 - 0.21 = 0.79 (79%)
    TSFiQueryNode *var_a = tsfi_query_create_var("query/holds_token");
    TSFiQueryNode *var_b = tsfi_query_create_var("query/active");
    TSFiQueryNode *var_c = tsfi_query_create_var("query/signature");

    TSFiQueryNode *and_node = tsfi_query_create_mul(var_a, var_b);
    TSFiQueryNode *root_query = tsfi_query_create_add(and_node, var_c);

    float result = tsfi_query_evaluate(root_query, dat);
    printf("  [Evaluation] (A AND B) OR C -> Joint Measure = %.4f (Expected 0.7900)\n", result);
    fflush(stdout);
    assert(fabs(result - 0.7900f) < 0.0001f);

    // Cleanup
    tsfi_query_destroy(root_query);
    tsfi_dat_destroy(dat);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Algebraic query compiler verified successfully!\n");
    fflush(stdout);
    return 0;
}
