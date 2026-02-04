#include "tsfi_wiring.h"
#include "tsfi_logic.h"
#include "lau_thunk.h"
#include "lau_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

WaveSystem* tsfi_create_system(void) {
    WaveSystem *ws = (WaveSystem *)lau_malloc_wired(sizeof(WaveSystem));
    if (!ws) return NULL;

    LauSystemHeader *h = (LauSystemHeader *)((char *)ws - sizeof(LauSystemHeader));
    // Initial status
    h->resonance_as_status = lau_strdup("LAU_SYNC_START");
    
    lau_wire_system(ws, h, tsfi_get_default_logic());
    return ws;
}

void lau_wire_system(WaveSystem *ws, LauSystemHeader *h, const TSFiLogicTable *logic) {
    // 1. Map Data Pointers
    *(int**)&ws->version = &h->version;
    *(char***)&ws->resonance_as_status = &h->resonance_as_status;
    *(bool**)&ws->ftw = &h->ftw;
    *(int**)&ws->counter = &h->counter;
    *(bool**)&ws->is_autonomous_excuse_active = &h->is_autonomous_excuse_active;

    // 2. Setup Logic & Proxy
    h->logic_epoch = logic->logic_epoch;
    h->logic_state = logic->logic_state;
    h->logic_directive = logic->logic_directive;
    h->logic_scramble = logic->logic_scramble;
    h->logic_provenance = logic->logic_provenance;
    h->proxy = ThunkProxy_create();
    
    // 3. Generate Thunks
    ThunkProxy *p = h->proxy;
    if (!p) {
        fprintf(stderr, "[TSFI_LOGIC] FATAL: Failed to create ThunkProxy (JIT backend).\n");
        abort();
    }

    if (p) {
        mprotect(p->thunk_pool, p->pool_size, PROT_READ | PROT_WRITE);
        ws->step_safety_epoch = (void(*)(void))ThunkProxy_emit_baked(p, h->logic_epoch, 1, &h->version);
        ws->step_safety_state = (void(*)(void))ThunkProxy_emit_baked(p, h->logic_state, 1, &h->ftw);
        ws->step_executor_directive = (void(*)(char*))ThunkProxy_emit_mixed(p, h->logic_directive, &h->counter);
        ws->scramble = (void(*)(void))ThunkProxy_emit_baked(p, h->logic_scramble, 1, ws);
        ws->provenance = (void(*)(void))ThunkProxy_emit_baked(p, h->logic_provenance, 1, ws);
        
        // Finalize Executable Memory
        __builtin___clear_cache((char*)p->thunk_pool, (char*)p->thunk_cursor);
        mprotect(p->thunk_pool, p->pool_size, PROT_READ | PROT_EXEC);
    }
    
    // Seal the object to protect logic pointers from tampering
    lau_seal_object(ws);
}

void lau_update_logic(WaveSystem *ws, const TSFiLogicTable *new_logic) {
    if (!ws || !new_logic) return; 
    
    // Rebind the thunks to the new functions
    ThunkProxy_rebind((void*)ws->step_safety_epoch, (void*)new_logic->logic_epoch);
    ThunkProxy_rebind((void*)ws->step_safety_state, (void*)new_logic->logic_state);
    ThunkProxy_rebind((void*)ws->step_executor_directive, (void*)new_logic->logic_directive);
    ThunkProxy_rebind((void*)ws->scramble, (void*)new_logic->logic_scramble);
    ThunkProxy_rebind((void*)ws->provenance, (void*)new_logic->logic_provenance);
    
    printf("[TSFI_WIRING] Logic hot-swapped successfully.\n");
}

void lau_final_cleanup(WaveSystem *ws, int sfd) {
    if (ws) {
        if (ws->current_directive) {
            lau_free(ws->current_directive);
            ws->current_directive = NULL;
        }
        
        lau_free(ws);
    }
    lau_quarantine_drain();
    if (sfd != -1) close(sfd);
}