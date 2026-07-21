#ifndef TSFI_PARC_INGERMAN_THUNK_H
#define TSFI_PARC_INGERMAN_THUNK_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define TSFI_INGERMAN_MAX_THUNKS 32
#define TSFI_INGERMAN_THUNK_NAME_LEN 64

/* Thunk Evaluation Function Prototype */
typedef uint64_t (*tsfi_ingerman_thunk_fn_t)(void *user_ctx);

/* Peter Z. Ingerman Call-by-Name Thunk Descriptor */
typedef struct {
    uint32_t thunk_id;
    char name[TSFI_INGERMAN_THUNK_NAME_LEN];
    tsfi_ingerman_thunk_fn_t eval_fn;
    void *user_ctx;
    uint64_t cached_value;
    bool is_evaluated;
    uint32_t eval_count;
    char target_contract_address[64];
} tsfi_ingerman_thunk_t;

/* Ingerman Thunk Manager State Machine */
typedef struct {
    bool is_initialized;
    uint32_t thunk_count;
    tsfi_ingerman_thunk_t thunks[TSFI_INGERMAN_MAX_THUNKS];
    char dynamic_vm_contract[64];
} tsfi_ingerman_thunk_manager_t;

/* Initialize Ingerman Thunk Manager */
int tsfi_ingerman_thunk_manager_initialize(tsfi_ingerman_thunk_manager_t *mgr);

/* Register Call-by-Name Thunk */
int tsfi_ingerman_thunk_register(tsfi_ingerman_thunk_manager_t *mgr, const char *name, tsfi_ingerman_thunk_fn_t eval_fn, void *ctx, const char *contract_addr, uint32_t *thunk_id_out);

/* Evaluate Thunk by ID (Call-by-Name Lazy Evaluation) */
uint64_t tsfi_ingerman_thunk_evaluate(tsfi_ingerman_thunk_manager_t *mgr, uint32_t thunk_id);

/* Evaluate Thunk by Symbol Name */
uint64_t tsfi_ingerman_thunk_evaluate_by_name(tsfi_ingerman_thunk_manager_t *mgr, const char *name);

/* Force Invalidate All Thunk Caches */
int tsfi_ingerman_thunk_invalidate_all(tsfi_ingerman_thunk_manager_t *mgr);

#endif // TSFI_PARC_INGERMAN_THUNK_H
