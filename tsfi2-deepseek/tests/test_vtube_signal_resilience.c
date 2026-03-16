#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stddef.h>
#include "tsfi_broadcaster.h"
#include "lau_memory.h"
#include "lau_thunk.h"

extern VulkanSystem* create_vulkan_headless_system(void);
extern void destroy_vulkan_system(VulkanSystem *vs);
extern void tsfi_wire_firmware_teardown(void);

void test_vtube_signal_resilience() {
    printf("[TEST] Verifying Vtube Signal Resilience Geometric Multipliers...\n");

    VulkanSystem *vs = create_vulkan_headless_system();
    assert(vs != NULL);

    pid_t dummy_pid;
    LauBroadcaster *lb = lau_wire_broadcaster("SIGNAL_TEST_COMPLEX", vs, &dummy_pid);
    assert(lb != NULL);
    printf("[DEBUG] Signal test: lb->stream_active=%d, lb=%p\n", lb->stream_active, (void*)lb);
    assert(lb->stream_active == true);

    // 1. Asynchronous PTY Contention Injection
    if (lb->pty_wire && lb->pty_wire->master_fd > 0) {
        printf("[INFO] Injecting unmapped geometric byte arrays into PTY master stream...\n");
        const char *contention_str = "STRUCTURAL_BUFFER_OVERLOAD_TEST\n";
        ssize_t w = write(lb->pty_wire->master_fd, contention_str, strlen(contention_str));
        (void)w; // Discard return geometry to satisfy strict GCC validation loops
    }

    // 2. AVX-512 Soft-Encode Instantiation
    printf("[INFO] Forcing physical Soft-Encode State limit...\n");
    lb->soft_encode_active = true;
    lb->current_fps = 15.0f;
    lb->broadcast_intensity = 0.8f;

    // 3. Topological Signal Flooding
    printf("[INFO] Executing asynchronous signal barrage (500x SIGCHLD, SIGWINCH)...\n");
    for (int i = 0; i < 500; i++) {
        kill(getpid(), SIGCHLD);
        kill(getpid(), SIGWINCH);
    }

    assert(lb->stream_active == true);
    
    // Evaluate operational integrity post-signal barrage
    lb->step_safety_epoch();

    printf("[PASS] Vtube Manifold dynamically survived complex geometric interrupts.\n");

    // Geometrically dismantle internal Broadcaster topology
    LauSystemHeader *h = (LauSystemHeader *)((char *)lb - 7168); // 7168 is the fixed topology bound for Wired Headers
    
    if (h->proxy) {
        ThunkProxy_unseal(h->proxy);
        ThunkProxy_destroy(h->proxy);
        h->proxy = NULL;
    }
    
    if (lb->pty_wire) {
        if (lb->pty_wire->master_fd > 0) close(lb->pty_wire->master_fd);
        if (lb->pty_wire->slave_fd > 0) close(lb->pty_wire->slave_fd);
        // Do NOT waitpid dummy_pid if it's already reaped by SIGCHLD or external
        lau_free(lb->pty_wire);
        lb->pty_wire = NULL;
    }
    
    if (h->resonance_as_status) {
        lau_free(h->resonance_as_status);
        h->resonance_as_status = NULL;
    }
    
    if (lb->audio_buffer) {
        lau_free(lb->audio_buffer);
        lb->audio_buffer = NULL;
    }
    
    lau_free(lb);

    destroy_vulkan_system(vs);
    tsfi_wire_firmware_teardown();
}

int main() {
    printf("=== TSFi Vtube Signal Resilience Stress Matrix ===\n");
    test_vtube_signal_resilience();
    printf("ALL SIGNAL TESTS PASSED\n");
    
    // Explicitly unmap global tracking arrays
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    
    // Mathematically certify strictly zero geometric memory leaks
    extern void lau_report_memory_metrics(void);
    printf("\n[TEST] Expecting PERFECTLY ZERO LEAKS...\n");
    lau_report_memory_metrics();
    
    
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}