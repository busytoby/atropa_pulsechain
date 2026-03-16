#include "tsfi_wiring.h"
#include "tsfi_logic.h"
#include "lau_thunk.h"
#include "lau_memory.h"
#include "tsfi_wire_firmware.h"
#include "lau_wire_log.h"
#include "vulkan/vulkan_system.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <sys/mman.h>
#include <immintrin.h>

WaveSystem* tsfi_create_system(void) {
    // Abstracted signal handling setup via firmware
    tsfi_wire_firmware_setup_signals();
    
    // Ensure the firmware is initialized early
    LauWireFirmware *fw = tsfi_wire_firmware_get();

    WaveSystem *ws = (WaveSystem *)lau_malloc_wired(sizeof(WaveSystem));
    if (!ws) return NULL;
    ws->fw = fw;
    LauSystemHeader *h = (LauSystemHeader *)((char *)ws - offsetof(LauSystemHeader, payload));
    memset(&h->regs, 0, sizeof(LauRegisterBank));
    h->resonance_as_status = lau_strdup("LAU_SYNC_START");
    lau_wire_system(ws, h, tsfi_get_default_logic());
    return ws;
}

// --- Wavefront Wiring ---

static void svdag_vgpr_kernel(void* ctx) {
    (void)ctx;
    // The JIT thunk sets R15 to point to the register bank.
    register LauRegisterBank *regs __asm__("r15");

    // Perform a simple AVX-512 operation: ZMM0 = ZMM0 + ZMM1
    // Assume ZMM0 is loaded from vgpr[0] and ZMM1 from vgpr[16]
    // And store result in ZMM3 (vgpr[48])
    __m512 zmm0_val = _mm512_loadu_ps(&regs->vgpr[0]);
    __m512 zmm1_val = _mm512_loadu_ps(&regs->vgpr[16]);
    __m512 zmm_result = _mm512_add_ps(zmm0_val, zmm1_val);
    _mm512_storeu_ps(&regs->vgpr[48], zmm_result);
}

WavefrontContext* tsfi_create_wavefront(WaveSystem *parent_ws) {
    (void)parent_ws;
    WavefrontContext *wf = (WavefrontContext*)lau_malloc_wired(sizeof(WavefrontContext));
    if (!wf) return NULL;

    LauSystemHeader *h = (LauSystemHeader *)((char *)wf - offsetof(LauSystemHeader, payload));
    memset(&h->regs, 0, sizeof(LauRegisterBank));
    h->resonance_as_status = lau_strdup("WF_START");

    lau_wire_wavefront(wf, h);
    return wf;
}

void lau_wire_wavefront(WavefrontContext *wf, LauSystemHeader *h) {
    if (!h->proxy) h->proxy = ThunkProxy_create();
    ThunkProxy *p = h->proxy;

    if (p) {
        // Calculate secrets into the LauRegisterBank's vgpr array
        int secret_from_regs_to_vgpr_base = (int)offsetof(LauRegisterBank, vgpr);

        // Assign specific secrets within the vgpr array to the WavefrontContext
        wf->input_x_vgpr_secret = secret_from_regs_to_vgpr_base + (0 * sizeof(float));   // ZMM0
        wf->input_y_vgpr_secret = secret_from_regs_to_vgpr_base + (16 * sizeof(float));  // ZMM1
        wf->input_z_vgpr_secret = secret_from_regs_to_vgpr_base + (32 * sizeof(float));  // ZMM2
        wf->output_vgpr_secret  = secret_from_regs_to_vgpr_base + (48 * sizeof(float));  // ZMM3

        // JIT thunks that first set R15 to &h->regs, then call the relative load/store/kernel.
        void *rel_load_x = ThunkProxy_emit_vgpr_load_relative(p, wf->input_x_vgpr_secret, 0);
        printf("[DEBUG] wiring: rel_load_x=%p\n", rel_load_x);
        wf->load_x_to_vgpr = (void(*)(void))ThunkProxy_emit_set_r15_and_call(
            p, 
            rel_load_x, // ZMM0
            &h->regs
        );
        wf->load_y_to_vgpr = (void(*)(void))ThunkProxy_emit_set_r15_and_call(
            p, 
            ThunkProxy_emit_vgpr_load_relative(p, wf->input_y_vgpr_secret, 1), // ZMM1
            &h->regs
        );
        wf->load_z_to_vgpr = (void(*)(void))ThunkProxy_emit_set_r15_and_call(
            p, 
            ThunkProxy_emit_vgpr_load_relative(p, wf->input_z_vgpr_secret, 2), // ZMM2
            &h->regs
        );
        wf->store_output_from_vgpr = (void(*)(void))ThunkProxy_emit_set_r15_and_call(
            p, 
            ThunkProxy_emit_vgpr_store_relative(p, wf->output_vgpr_secret, 3), // ZMM3
            &h->regs
        );
        wf->execute_vgpr_kernel = (void(*)(void))ThunkProxy_emit_set_r15_and_call(
            p, 
            (void*)svdag_vgpr_kernel, 
            &h->regs
        );
        
        mprotect(((ThunkProxy*)p)->thunk_pool, ((ThunkProxy*)p)->pool_size, PROT_READ | PROT_EXEC);
    }
    tsfi_wire_firmware_load_struct(tsfi_wire_firmware_get(), wf);
}

void lau_wire_system(WaveSystem *ws, LauSystemHeader *h, const TSFiLogicTable *logic) {
    { LauWireFirmware *fw = tsfi_wire_firmware_get(); *(int**)&ws->version = (int*)&fw->rtl.cell_version; }
    { LauWireFirmware *fw = tsfi_wire_firmware_get(); *(char***)&ws->resonance_as_status = (char**)&fw->rtl.cell_resonance_status_ptr; }
    { LauWireFirmware *fw = tsfi_wire_firmware_get(); *(int**)&ws->counter = (int*)&fw->rtl.cell_counter; }
    h->logic_epoch = logic->logic_epoch;
    h->logic_state = logic->logic_state;
    h->logic_directive = logic->logic_directive;
    h->logic_scramble = logic->logic_scramble;
    h->logic_provenance = logic->logic_provenance;
    h->logic_hilbert = logic->logic_hilbert;
    h->logic_hilbert_batch = logic->logic_hilbert_batch;
    if (!h->proxy) h->proxy = ThunkProxy_create();
    if (h->proxy) {
        ThunkProxy *p = (ThunkProxy*)h->proxy;
        ws->step_safety_epoch = (void(*)(void))ThunkProxy_emit_baked(p, h->logic_epoch, 1, ws->version);
        ws->step_safety_state = (void(*)(void))ThunkProxy_emit_baked(p, h->logic_state, 1, &h->ftw);
        ws->step_executor_directive = (void(*)(char*))ThunkProxy_emit_mixed(p, h->logic_directive, &h->counter);
        ws->scramble = (void(*)(void))ThunkProxy_emit_baked(p, h->logic_scramble, 1, ws);
        ws->provenance = (void(*)(void))ThunkProxy_emit_baked(p, h->logic_provenance, 1, ws);
        ws->hilbert_eval = (void(*)(float, float, float*))ThunkProxy_emit_mixed(p, h->logic_hilbert, ws);
        ws->hilbert_batch = (void(*)(void*, const float*, float*, int))ThunkProxy_emit_forwarding(p, h->logic_hilbert_batch, ws);
        __builtin___clear_cache((char*)p->thunk_pool, (char*)p->thunk_cursor);
        mprotect(p->thunk_pool, p->pool_size, PROT_READ | PROT_EXEC);
    }
    
    tsfi_wire_firmware_load_struct(tsfi_wire_firmware_get(), ws);
    lau_seal_object(ws);
}

void lau_wire_log(LauWireLog *log) {
    if (!log) return;
    LauWireFirmware *fw = tsfi_wire_firmware_get();
    void *buf_ptr = (void*)(uintptr_t)fw->rtl.log_stdout_ptr;
    tsfi_io_printf(stdout, "[DEBUG] lau_wire_log: buf_ptr=%p, head_ptr=%p\n", buf_ptr, (void*)&fw->rtl.log_stdout_head);
    memcpy((void*)&log->buffer, &buf_ptr, sizeof(void*));
    uint32_t *h_ptr = &fw->rtl.log_stdout_head;
    memcpy((void*)&log->head, &h_ptr, sizeof(void*));
    uint32_t *v_ptr = &fw->rtl.log_stdout_valid_head;
    memcpy((void*)&log->valid_head, &v_ptr, sizeof(void*));
    uint32_t *t_ptr = &fw->rtl.log_stdout_tail;
    memcpy((void*)&log->tail, &t_ptr, sizeof(void*));
    _Atomic uint32_t *l_ptr = (_Atomic uint32_t *)&fw->rtl.log_stdout_lock;
    memcpy((void*)&log->lock, &l_ptr, sizeof(void*));
    log->capacity = 4096;
}

void lau_update_logic(WaveSystem *ws, const TSFiLogicTable *new_logic) {
    if (!ws || !new_logic) return; 
    ThunkProxy_rebind((void*)ws->step_safety_epoch, (void*)new_logic->logic_epoch);
}

void lau_final_cleanup(WaveSystem *ws, int sfd) {
    if (ws) {
        LauSystemHeader *h = (LauSystemHeader *)((char *)ws - offsetof(LauSystemHeader, payload));
        if (h->resonance_as_status) { lau_free(h->resonance_as_status); h->resonance_as_status = NULL; }
        if (ws->current_directive) { lau_free(ws->current_directive); ws->current_directive = NULL; }
        
        if (ws->fw) {
            extern void tsfi_wire_firmware_teardown(void);
            tsfi_wire_firmware_teardown();
        }
        
        lau_free(ws);
    }
    
    extern void tsfi_dl_thunks_teardown(void);
    extern void lau_registry_teardown(void);
    tsfi_dl_thunks_teardown();
    lau_registry_teardown();
    
    lau_quarantine_drain();
    if (sfd != -1) close(sfd);
}
