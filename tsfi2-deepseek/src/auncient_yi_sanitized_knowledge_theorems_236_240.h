#ifndef AUNCIENT_YI_SANITIZED_KNOWLEDGE_THEOREMS_236_240_H
#define AUNCIENT_YI_SANITIZED_KNOWLEDGE_THEOREMS_236_240_H

#include "auncient_quadtree_dai_folding_theorems_231_235.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define YI_HEXAGRAM_REGISTER_COUNT 64

typedef struct {
    uint32_t active_yi_hexagrams;
    float    knowledge_entropy_sanitization;
    uint32_t garbage_rejection_count;
    uint64_t verified_yi_dai_accumulation;
    bool     yi_canonical_hexagram_verified;     /* Theorem 236 */
    bool     strict_garbage_rejection_verified;  /* Theorem 237 */
    bool     sanitized_knowledge_basis_verified; /* Theorem 238 */
    bool     yi_quadtree_dai_folding_verified;   /* Theorem 239 */
    bool     yi_sanitized_grand_parity_verified; /* Theorem 240 */
    uint32_t rule18_parity_checksum;
} YiSanitizedKnowledgeState;

void auncient_yi_sanitized_init(YiSanitizedKnowledgeState *state);
bool auncient_yi_sanitized_verify_theorems_236_240(YiSanitizedKnowledgeState *state);
uint32_t auncient_yi_sanitized_compute_rule18(const YiSanitizedKnowledgeState *state);

#endif /* AUNCIENT_YI_SANITIZED_KNOWLEDGE_THEOREMS_236_240_H */
