#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_doxastic_projector.h"

int main(void) {
    printf("[Projector] Starting Vulkan doxastic projector tests...\n");
    fflush(stdout);

    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    
    // Setup a mixed region: X=0 is fact (BOND), X=1 is pending (SEED/FRONTIER), X=2 is retracted (VOID/FRACTURE)
    tsfi_trie_insert(trie_root, "svdag/0/10/20", "RELATION_TRUE");
    tsfi_trie_insert(trie_root, "svdag/1/10/20", "QUESTION_PENDING");
    tsfi_trie_insert(trie_root, "svdag/2/10/20", "");

    tsfi_dat *dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    TSFiDoxasticVertex vertices[8];
    int count = tsfi_doxastic_project_region(dat, 0, 2, 10, 20, vertices, 8);
    printf("  [Projector] Projected %d vertices to Vulkan buffer.\n", count);
    fflush(stdout);
    assert(count == 3);

    // Verify Vertex 0: CLASS_BOND (Green)
    printf("  [Vertex 0] Pos: (%.1f, %.1f, %.1f), Color: (%.2f, %.2f, %.2f), Strain: %.2f\n",
           vertices[0].x, vertices[0].y, vertices[0].z, vertices[0].r, vertices[0].g, vertices[0].b, vertices[0].strain);
    fflush(stdout);
    assert(vertices[0].x == 0.0f);
    assert(vertices[0].g > 0.8f && vertices[0].r < 0.2f); // Mostly green

    // Verify Vertex 1: CLASS_SEED/FRONTIER (Blue/Yellow with strain)
    printf("  [Vertex 1] Pos: (%.1f, %.1f, %.1f), Color: (%.2f, %.2f, %.2f), Strain: %.2f\n",
           vertices[1].x, vertices[1].y, vertices[1].z, vertices[1].r, vertices[1].g, vertices[1].b, vertices[1].strain);
    fflush(stdout);
    assert(vertices[1].x == 1.0f);
    assert(vertices[1].b > 0.8f); // Mostly blue
    assert(vertices[1].strain > 0.40f); // High strain gradient detected

    // Cleanup
    tsfi_dat_destroy(dat);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Vulkan doxastic projector coordinates verified successfully!\n");
    fflush(stdout);
    return 0;
}
