#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include "tsfi_broadcaster.h"
#include "vulkan_system.h"
#include "lau_memory.h"

extern VulkanSystem* create_vulkan_headless_system(void);
extern void vulkan_broadcaster_update_BASE(LauBroadcaster *lb, const char *buf, size_t n);
extern void tsfi_broadcaster_throttle_bytes(LauBroadcaster *lb, size_t bytes);

double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

void test_broadcaster_throughput() {
    printf("[TEST] Verifying Broadcaster Throughput (AVX-512 + Soft-Encoder)...\n");

    VulkanSystem *vs = create_vulkan_headless_system();
    assert(vs != NULL);

    pid_t child_pid;
    LauBroadcaster *lb = lau_wire_broadcaster("THROUGHPUT_TEST", vs, &child_pid);
    assert(lb != NULL);
    lb->soft_encode_active = true; 

    char burst[32];
    for(int i=0; i<32; i++) burst[i] = (char)(32 + (i % 95));

    const int frames = 100;
    double start = get_time_ms();

    for (int i = 0; i < frames; i++) {
        vulkan_broadcaster_update_BASE(lb, burst, sizeof(burst));
        lb->step_safety_epoch();
    }

    double end = get_time_ms();
    double total_time = end - start;
    double avg_frame = total_time / frames;

    printf("[INFO] Processed %d frames in %.2f ms (Avg: %.2f ms/frame)\n", frames, total_time, avg_frame);

    assert(avg_frame < 10.0);
    printf("[PASS] Throughput Requirements Met.\n");

    if (child_pid > 0) kill(child_pid, SIGTERM);
    if (lb->pty_wire) { lau_unseal_object(lb->pty_wire); lau_free(lb->pty_wire); }
    lau_unseal_object(lb);
    lau_free(lb);
    extern void cleanup_vulkan(VulkanContext *s);
    if (vs->vk) cleanup_vulkan(vs->vk);
    if (vs->paint_buffer) {
        destroy_staging_buffer(vs->paint_buffer);
    }
    LauSystemHeader *h = (LauSystemHeader *)((char *)vs - offsetof(LauSystemHeader, payload));
    if (h->resonance_as_status) lau_free((void*)h->resonance_as_status);
    lau_unseal_object(vs);
    lau_free(vs);
}

void test_bandwidth_throttle_determinism() {
    printf("\n[TEST] Verifying Bandwidth Throttling Determinism...\n");

    LauBroadcaster *lb = (LauBroadcaster *)lau_malloc_wired(sizeof(LauBroadcaster));
    assert(lb != NULL);
    memset(lb, 0, sizeof(LauBroadcaster));
    lb->target_bitrate_bps = 2500000;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    size_t total_sent = 0;
    size_t chunk_size = 65536;
    while (total_sent < 1048576) {
        tsfi_broadcaster_throttle_bytes(lb, chunk_size);
        total_sent += chunk_size;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double duration = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("[TEST] Sent 1MB in %.2f seconds.\n", duration);
    assert(duration >= 1.5);
    lau_unseal_object(lb);
    lau_free(lb);
    printf("[PASS] Bandwidth Throttling verified empirically.\n");
}

int main() {
    printf("=== TSFi Broadcaster VTube Performance Suite ===\n");
    test_broadcaster_throughput();
    test_bandwidth_throttle_determinism();
    printf("ALL THROUGHPUT TESTS PASSED\n");
    extern void tsfi_wire_firmware_teardown(void);
    tsfi_wire_firmware_teardown();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
