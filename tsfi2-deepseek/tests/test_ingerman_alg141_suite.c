#include "tsfi_parc_ingerman_alg141.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("=======================================================\n");
    printf(" RUNNING PETER Z INGERMAN ALG 141 PATH MATRIX AUDIT   \n");
    printf("=======================================================\n");

    tsfi_ingerman_alg141_t graph;
    assert(tsfi_ingerman_alg141_initialize(&graph, 4) == 0);
    printf("[PASS] Algorithm 141 State Machine Initialization verified\n");

    // Construct Directed Graph: 0 -> 1 -> 2 -> 3
    assert(tsfi_ingerman_alg141_add_edge(&graph, 0, 1) == 0);
    assert(tsfi_ingerman_alg141_add_edge(&graph, 1, 2) == 0);
    assert(tsfi_ingerman_alg141_add_edge(&graph, 2, 3) == 0);

    // Compute Transitive Closure Path Matrix (Ingerman CACM 1962)
    assert(tsfi_ingerman_alg141_compute_path_matrix(&graph) == 0);
    printf("[PASS] Algorithm 141 Path Matrix Transitive Closure computed\n");

    // Verify Reachability Pathways
    assert(tsfi_ingerman_alg141_is_reachable(&graph, 0, 3) == true);  // Indirect path 0->1->2->3
    assert(tsfi_ingerman_alg141_is_reachable(&graph, 1, 3) == true);  // Indirect path 1->2->3
    assert(tsfi_ingerman_alg141_is_reachable(&graph, 3, 0) == false); // Unreachable backward path

    printf("[PASS] Reachability Query (0 -> 3: %s, 3 -> 0: %s) verified\n",
           tsfi_ingerman_alg141_is_reachable(&graph, 0, 3) ? "TRUE" : "FALSE",
           tsfi_ingerman_alg141_is_reachable(&graph, 3, 0) ? "TRUE" : "FALSE");

    char matrix_buf[1024];
    assert(tsfi_ingerman_alg141_render_matrix(&graph, matrix_buf, sizeof(matrix_buf)) == 0);
    printf("\n%s\n", matrix_buf);

    printf("=======================================================\n");
    printf(" ALL PETER Z INGERMAN ALG 141 TESTS PASSED (100%%)     \n");
    printf("=======================================================\n");

    return 0;
}
