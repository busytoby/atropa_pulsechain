#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_doxastic_pipeline.h"

int main(void) {
    printf("[Doxastic Pipeline] Starting unified render pipeline tests...\n");
    fflush(stdout);

    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    // Setup initial SVDAG frontier: x=0 is RELATION_TRUE, x=1 is QUESTION_PENDING
    tsfi_trie_insert(trie_root, "svdag/0/10/10", "RELATION_TRUE");
    tsfi_trie_insert(trie_root, "svdag/1/10/10", "QUESTION_PENDING");

    tsfi_dat *dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    TSFiDoxasticPipeline *pipeline = tsfi_doxastic_pipeline_create(dat);
    assert(pipeline != NULL);

    // Run pipeline update step
    int res = tsfi_doxastic_pipeline_update(pipeline, 0, 1, 10, 10, "prior/holds_token", "prior/active");
    printf("  [Pipeline Update] Result: %d, Vertex Count: %d, Dirty Flag: %d\n", res, pipeline->active_vertex_count, pipeline->is_dirty);
    fflush(stdout);
    assert(res == 0);
    assert(pipeline->active_vertex_count == 2);
    assert(pipeline->is_dirty == 1);

    // Verify vertex colors
    printf("  [Pipeline Update] Vertex 0 Color: R=%.2f, G=%.2f, B=%.2f\n", 
           pipeline->vertex_buffer[0].r, pipeline->vertex_buffer[0].g, pipeline->vertex_buffer[0].b);
    printf("  [Pipeline Update] Vertex 1 Color: R=%.2f, G=%.2f, B=%.2f\n", 
           pipeline->vertex_buffer[1].r, pipeline->vertex_buffer[1].g, pipeline->vertex_buffer[1].b);
    fflush(stdout);
    assert(pipeline->vertex_buffer[0].g > 0.8f); // Vertex 0 is Green (RELATION_TRUE)
    assert(pipeline->vertex_buffer[1].b > 0.8f); // Vertex 1 is Blue (QUESTION_PENDING)

    // Cleanup
    tsfi_doxastic_pipeline_destroy(pipeline);
    tsfi_dat_destroy(dat);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Unified doxastic render pipeline verified successfully!\n");
    fflush(stdout);
    return 0;
}
