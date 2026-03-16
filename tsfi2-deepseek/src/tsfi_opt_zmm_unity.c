#include "lau_memory.h"
#include "tsfi_opt_zmm.h"
#include "lau_thunk.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <stddef.h>

// Helper to find the proxy in the header
static ThunkProxy** get_proxy_slot(TsfiZmmManifest *m) {
    LauWiredHeader *h = (LauWiredHeader*)((char*)m - offsetof(LauWiredHeader, payload));
    return (ThunkProxy**)&h->proxy;
}

// Binds a specific Manifest to a JIT Thunk
void tsfi_unity_bind(TsfiZmmManifest *m) {
    if (!m || !m->contiguous_rf || !m->micro_kernel) return;
    
    ThunkProxy **proxy_ptr = get_proxy_slot(m);
    if (*proxy_ptr) return; // Already bound

    ThunkProxy *p = ThunkProxy_create();
    if (!p) return;

    // Generate Thunk:
    // 1. MOV RDI, contiguous_rf (Arg1: Register File / ReBAR)
    // 2. MOV RSI, &synapse      (Arg2: Synapse / Feedback)
    // 3. JMP micro_kernel       (Tail Call)
    
    // We use emit_baked to set registers and jump
    // void* ThunkProxy_emit_baked(ThunkProxy *p, void *fn, int argc, ...);
    
    // Arguments:
    // RDI = m->contiguous_rf
    // RSI = &m->synapse
    // Function = m->micro_kernel
    
    ThunkProxy_emit_baked(p, m->micro_kernel, 2, m->contiguous_rf, &m->synapse);
    
    *proxy_ptr = p;
}

// Upgraded Unity Dispatcher
// Checks for JIT Thunk. If present, executes it. If not, falls back (and optionally binds).
void tsfi_dispatch_unity(TsfiZmmManifest *m) {
    if (!m) return;

    // 1. Bijection Check
    LauWiredHeader *h = (LauWiredHeader*)((char*)m - offsetof(LauWiredHeader, payload));
    if (h->footer.magic != LAU_MAGIC || h->footer.type != LAU_TYPE_WIRED) {
        tsfi_io_printf(stdout, "[TSFI-UNITY] BIJECTION FAULT: Dispatch manifest is not wired memory.\n");
        return;
    }

    // 2. JIT Fast-Path
    ThunkProxy *p = (ThunkProxy*)h->proxy;
    if (p && p->thunk_cursor > p->thunk_pool) {
        // Execute Thunk
        // The thunk expects no arguments (it has baked them), so we cast to void(*)()
        void (*jit_fn)(void) = (void (*)(void))p->thunk_pool;
        jit_fn();
        return;
    }

    // 3. Fallback / Lazy Binding
    // We bind on first use to promote future speed
    tsfi_unity_bind(m);
    
    // Execute dynamic for this frame
    tsfi_dispatch_zmm_dynamic(m);
}
