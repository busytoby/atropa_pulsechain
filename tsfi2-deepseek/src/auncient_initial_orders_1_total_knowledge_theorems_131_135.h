#ifndef AUNCIENT_INITIAL_ORDERS_1_TOTAL_KNOWLEDGE_THEOREMS_131_135_H
#define AUNCIENT_INITIAL_ORDERS_1_TOTAL_KNOWLEDGE_THEOREMS_131_135_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define INITIAL_ORDERS_1_WORD_COUNT 31
#define INITIAL_ORDERS_1_TPA_BASE   0x0100

typedef struct {
    uint32_t memory_words[INITIAL_ORDERS_1_WORD_COUNT];
    uint32_t total_words_loaded;
    uint32_t tk_master_witness_required;
    uint32_t tk_witness_provided;
    bool     tk_precondition_verified;
    bool     loader_word_limit_verified;
    bool     relocatable_arithmetic_verified;
    bool     prefetch_opcode_audit_verified;
    bool     initial_orders_parity_verified;
    uint32_t rule18_parity_checksum;
} InitialOrders1TkState;

void auncient_initial_orders_1_init(InitialOrders1TkState *state, uint32_t tk_witness);
bool auncient_initial_orders_1_load_bootstrap(InitialOrders1TkState *state);
bool auncient_initial_orders_1_relocate(InitialOrders1TkState *state, uint32_t base_address, uint32_t offset, uint32_t *out_target);
bool auncient_initial_orders_1_verify_theorems_131_135(InitialOrders1TkState *state);
uint32_t auncient_initial_orders_1_compute_rule18(const InitialOrders1TkState *state);

#endif /* AUNCIENT_INITIAL_ORDERS_1_TOTAL_KNOWLEDGE_THEOREMS_131_135_H */
