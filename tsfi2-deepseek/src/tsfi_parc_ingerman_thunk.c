#include "tsfi_parc_ingerman_thunk.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int tsfi_ingerman_thunk_manager_initialize(tsfi_ingerman_thunk_manager_t *mgr) {
    if (!mgr) return -1;
    memset(mgr, 0, sizeof(tsfi_ingerman_thunk_manager_t));

    mgr->is_initialized = true;
    snprintf(mgr->dynamic_vm_contract, sizeof(mgr->dynamic_vm_contract),
             "dynamic_0x5468756e6b496e6765726d616e6e31393633564d");

    return 0;
}

int tsfi_ingerman_thunk_register(tsfi_ingerman_thunk_manager_t *mgr, const char *name, tsfi_ingerman_thunk_fn_t eval_fn, void *ctx, const char *contract_addr, uint32_t *thunk_id_out) {
    if (!mgr || !name || !eval_fn) return -1;
    if (mgr->thunk_count >= TSFI_INGERMAN_MAX_THUNKS) return -2;

    uint32_t id = mgr->thunk_count++;
    tsfi_ingerman_thunk_t *t = &mgr->thunks[id];
    t->thunk_id = id;
    snprintf(t->name, sizeof(t->name), "%s", name);
    t->eval_fn = eval_fn;
    t->user_ctx = ctx;
    t->is_evaluated = false;
    t->eval_count = 0;
    snprintf(t->target_contract_address, sizeof(t->target_contract_address), "%s",
             contract_addr ? contract_addr : "dynamic_0x00");

    if (thunk_id_out) *thunk_id_out = id;
    return 0;
}

uint64_t tsfi_ingerman_thunk_evaluate(tsfi_ingerman_thunk_manager_t *mgr, uint32_t thunk_id) {
    if (!mgr || thunk_id >= mgr->thunk_count) return 0;

    tsfi_ingerman_thunk_t *t = &mgr->thunks[thunk_id];
    if (!t->eval_fn) return 0;

    // Call-by-Name lazy evaluation in environment context
    uint64_t val = t->eval_fn(t->user_ctx);
    t->cached_value = val;
    t->is_evaluated = true;
    t->eval_count++;

    return val;
}

uint64_t tsfi_ingerman_thunk_evaluate_by_name(tsfi_ingerman_thunk_manager_t *mgr, const char *name) {
    if (!mgr || !name) return 0;

    for (uint32_t i = 0; i < mgr->thunk_count; i++) {
        if (strcmp(mgr->thunks[i].name, name) == 0) {
            return tsfi_ingerman_thunk_evaluate(mgr, i);
        }
    }

    return 0;
}

int tsfi_ingerman_thunk_invalidate_all(tsfi_ingerman_thunk_manager_t *mgr) {
    if (!mgr) return -1;

    for (uint32_t i = 0; i < mgr->thunk_count; i++) {
        mgr->thunks[i].is_evaluated = false;
    }

    return 0;
}
