#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "lau_memory.h"
#include "tsfi_wire_firmware.h"
#include "tsfi_resonance.h"
#include "tsfi_hotloader.h"
#include "lau_thunk.h"
#include "tsfi_k0rn_ops.h"

#define ALLOC_COUNT 500

int main() {
    printf("[ALIGN_STRESS] Initializing geometric alignment constraints...\n");
    
    size_t off = offsetof(LauWiredHeader, payload);
    if (off % 512 != 0) {
        printf("[FAIL] Payload offset %zu is NOT 512-aligned!\n", off);
        return 1;
    }
    
    LauWireFirmware *fw = tsfi_wire_firmware_get();
    if (!fw) return 1;

    void *wired_blocks[ALLOC_COUNT];
    for (int i = 0; i < ALLOC_COUNT; i++) {
        wired_blocks[i] = lau_malloc_wired(1024 + (i * 16));
        
        // 8. Introduce Memory Scrambling Dynamics
        lau_mem_scramble(wired_blocks[i], 1024 + (i * 16), 0xFF);
        
        if ((uintptr_t)wired_blocks[i] % 512 != 0) return 1;
    }
    
    LauWireThread *threads[4];
    for (int i = 0; i < 4; i++) {
        threads[i] = tsfi_wire_thread_create(i, 1.0);
        tsfi_wire_thread_seal(threads[i]);
    }
    
    // 9. Asynchronous OS-Level PTY Harvesting Boundary
    printf("[ALIGN_STRESS] Spawning Asynchronous Subshell PTY Geometry...\n");
    LauWireThread *pty_thread = tsfi_wire_thread_create_pty(99, "ls > /dev/null", NULL);
    if (pty_thread) {
        tsfi_wire_thread_seal(pty_thread);
    }
    
    ThunkProxy *proxy = ThunkProxy_create();
    if (proxy) {
        ThunkProxy_seal(proxy);
    }
    
    // 10. K0Rn JIT Execution Wave Synthesis
    printf("[ALIGN_STRESS] Synthesizing Dynamic K0Rn Wavefront Structs...\n");
    K0RnStream *stream_a = tsfi_k0rn_compile_char(0x41); // 'A'
    K0RnStream *stream_b = tsfi_k0rn_compile_char(0x42); // 'B'
    
    TSFiLogicTable logic;
    if (tsfi_compile_plugin("plugins/minimal_plugin.c", "plugins/minimal_plugin.so") == 0) {
        tsfi_load_plugin("plugins/minimal_plugin.so", &logic);
    }
    
    for (int i = 0; i < ALLOC_COUNT; i++) lau_free(wired_blocks[i]);
    
    for (int i = 0; i < 4; i++) tsfi_wire_thread_destroy(threads[i]);
    
    if (pty_thread) tsfi_wire_thread_destroy(pty_thread);
    
    if (proxy) {
        ThunkProxy_unseal(proxy);
        ThunkProxy_destroy(proxy);
    }
    
    if (stream_a) {
        if (stream_a->ops) lau_free(stream_a->ops);
        lau_free(stream_a);
    }
    if (stream_b) {
        if (stream_b->ops) lau_free(stream_b->ops);
        lau_free(stream_b);
    }
    
    extern void tsfi_dl_thunks_teardown(void);
    tsfi_dl_thunks_teardown();
    
    extern void tsfi_wire_firmware_teardown(void);
    tsfi_wire_firmware_teardown();

    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    
    printf("\n[ALIGN_STRESS] Expecting PERFECTLY ZERO LEAKS...\n");
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    
    
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
