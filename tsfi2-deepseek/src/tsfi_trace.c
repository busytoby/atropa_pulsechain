#include "tsfi_trace.h"
#include "lau_memory.h"
#include "lau_audit.h"
#include <stdio.h>

void apply_traced_resonance(WaveSystem *ws, void (*thunk)(void*), void *arg, const char *fn_name) {
    if (*ws->resonance_as_status != NULL) {
        lau_free(*ws->resonance_as_status);
        *ws->resonance_as_status = NULL;
    }
    int prev_counter = *ws->counter;
    
    thunk(arg); // Execute Thunk
    
    int delta = *ws->counter - prev_counter;
    char buffer[512];
    snprintf(buffer, sizeof(buffer), "[TRACE] AB-316 | EXEC: %s() | Δ_CTR: %d", fn_name, delta);
    *ws->resonance_as_status = lau_strdup(buffer);
    lau_log_status(buffer);
}
