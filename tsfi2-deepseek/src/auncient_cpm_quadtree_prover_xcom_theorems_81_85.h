#ifndef AUNCIENT_CPM_QUADTREE_PROVER_XCOM_THEOREMS_81_85_H
#define AUNCIENT_CPM_QUADTREE_PROVER_XCOM_THEOREMS_81_85_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define CPM_QUADTREE_TPA_BASE 0x0100
#define MAX_QUADTREE_LEAF_AST_BYTES 512

typedef struct __attribute__((packed)) {
    uint32_t morton_code;
    float    spatial_bbox[6]; /* minX, minY, minZ, maxX, maxY, maxZ */
    uint16_t strategy_token_id;
    uint16_t algol61_prover_id;
    uint32_t witness_target;
    uint16_t ast_payload_len;
    uint8_t  ast_payload[MAX_QUADTREE_LEAF_AST_BYTES];
    uint16_t rule18_crc16;
} QuadtreeDatBinProofNode;

typedef struct {
    QuadtreeDatBinProofNode node;
    uint32_t xcom_xdc_register_val;
    float query_latency_ns;
    bool quadtree_ast_packing_verified;
    bool logarithmic_lookup_verified;
    bool in_memory_ast_eval_verified;
    bool xcom_zero_copy_verified;
    bool quadtree_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} CpmQuadtreeProverXcomState;

void auncient_cpm_quadtree_prover_init(CpmQuadtreeProverXcomState *state, uint32_t morton, uint16_t strat_id, uint16_t prover_id);
bool auncient_cpm_quadtree_prover_evaluate(CpmQuadtreeProverXcomState *state, float x, float y, float z);
bool auncient_cpm_quadtree_prover_verify_theorems_81_85(CpmQuadtreeProverXcomState *state);
uint32_t auncient_cpm_quadtree_prover_compute_rule18(const CpmQuadtreeProverXcomState *state);

#endif /* AUNCIENT_CPM_QUADTREE_PROVER_XCOM_THEOREMS_81_85_H */
