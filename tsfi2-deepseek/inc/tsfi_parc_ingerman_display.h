#ifndef TSFI_PARC_INGERMAN_DISPLAY_H
#define TSFI_PARC_INGERMAN_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define TSFI_INGERMAN_MAX_LEXICAL_DEPTH 16
#define TSFI_INGERMAN_MAX_FRAME_SLOTS 32

/* Stack Frame Activation Record Descriptor */
typedef struct {
    uint32_t block_id;
    uint32_t lexical_level;
    uint64_t frame_slots[TSFI_INGERMAN_MAX_FRAME_SLOTS];
    uint32_t slot_count;
    char contract_address[64];
} tsfi_ingerman_stack_frame_t;

/* Peter Z. Ingerman (1961 CACM) Dynamic Display Vector Table */
typedef struct {
    bool is_initialized;
    uint32_t current_depth;
    tsfi_ingerman_stack_frame_t *display_vector[TSFI_INGERMAN_MAX_LEXICAL_DEPTH];
    char dynamic_contract_router[64];
} tsfi_ingerman_display_env_t;

/* Initialize Display Environment */
int tsfi_ingerman_display_env_init(tsfi_ingerman_display_env_t *env);

/* Push New Lexical Block Scope (Ingerman 1961 Scheme) */
int tsfi_ingerman_display_push_block(tsfi_ingerman_display_env_t *env, uint32_t block_id, tsfi_ingerman_stack_frame_t *frame);

/* Pop Top Lexical Block Scope */
int tsfi_ingerman_display_pop_block(tsfi_ingerman_display_env_t *env);

/* O(1) Lexical Variable Lookup via Display Vector */
uint64_t tsfi_ingerman_display_lookup(const tsfi_ingerman_display_env_t *env, uint32_t lexical_level, uint32_t slot_index);

/* Store Variable into Frame via Display Vector */
int tsfi_ingerman_display_store(tsfi_ingerman_display_env_t *env, uint32_t lexical_level, uint32_t slot_index, uint64_t value);

#endif // TSFI_PARC_INGERMAN_DISPLAY_H
