#ifndef TSFI_TRACE_H
#define TSFI_TRACE_H

#include "tsfi_types.h"

// Tracing Function
void apply_traced_resonance(WaveSystem *ws, void (*thunk)(void*), void *arg, const char *fn_name);

// Macros
#define STEP_safety_epoch(ws) \
    apply_traced_resonance(ws, (void(*)(void*))ws->step_safety_epoch, NULL, "step_safety_epoch");

#define STEP_safety_state(ws) \
    apply_traced_resonance(ws, (void(*)(void*))ws->step_safety_state, NULL, "step_safety_state");

#define STEP_executor_directive(ws) \
    apply_traced_resonance(ws, (void(*)(void*))ws->step_executor_directive, ws->current_directive, "step_executor_directive");

#define STEP(ws, func, val) STEP_##func(ws)

#define HELMHOLTZ_RESONANCE_LIST(X, ws, i) \
    X(ws, safety_epoch, 1.25) \
    X(ws, safety_state, 0.50) \
    X(ws, executor_directive, i)

#endif // TSFI_TRACE_H
