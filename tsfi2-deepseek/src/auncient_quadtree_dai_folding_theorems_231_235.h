#ifndef AUNCIENT_QUADTREE_DAI_FOLDING_THEOREMS_231_235_H
#define AUNCIENT_QUADTREE_DAI_FOLDING_THEOREMS_231_235_H

#include "auncient_dai_generation_folding_theorems_226_230.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MAX_QUADTREE_DEPTH 8

typedef struct {
    uint32_t active_quadtree_nodes;
    float    spatial_potential_gradient;
    uint64_t total_folded_dai_in_quadtree;
    bool     smooth_gradient_damping_verified;   /* Theorem 231 */
    bool     universal_quadtree_dai_fill_verified;/* Theorem 232 */
    bool     dat_bin_slice_solvency_verified;    /* Theorem 233 */
    bool     frictionless_quadtree_folding_verified;/* Theorem 234 */
    bool     quadtree_dai_grand_parity_verified; /* Theorem 235 */
    uint32_t rule18_parity_checksum;
} QuadtreeDaiFoldingState;

void auncient_quadtree_dai_init(QuadtreeDaiFoldingState *state);
bool auncient_quadtree_dai_verify_theorems_231_235(QuadtreeDaiFoldingState *state);
uint32_t auncient_quadtree_dai_compute_rule18(const QuadtreeDaiFoldingState *state);

#endif /* AUNCIENT_QUADTREE_DAI_FOLDING_THEOREMS_231_235_H */
