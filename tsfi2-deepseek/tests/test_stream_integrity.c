#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdatomic.h>
#include <immintrin.h>
#include <unistd.h>
#include "tsfi_resonance.h"
#include "tsfi_zhao.h"
#include "tsfi_wave_any.h"
#include "lau_memory.h"
#include <nmmintrin.h>

#define STREAM_SIZE 1000
#define RUN_TIME_US 200000

typedef struct {
    uint64_t data[7];
    uint32_t seq;
    uint32_t checksum;
} AtomicFrame;

WaveStream g_stream;
atomic_bool g_running = true;
atomic_uint g_tail_idx = 0;
atomic_uint g_safe_idx = 0;
atomic_long g_valid_reads = 0;
atomic_long g_torn_reads = 0;

uint32_t calc_checksum(const AtomicFrame* f) {
    uint64_t crc = 0;
    const uint64_t* p = (const uint64_t*)f->data;
    for(int i=0; i<7; i++) crc = _mm_crc32_u64(crc, p[i]);
    crc = _mm_crc32_u64(crc, f->seq);
    return (uint32_t)crc;
}

uint32_t g_writer_seq = 1;
void* writer_thread(void* arg) {
    (void)arg;
    for (uint32_t i = 0; i < STREAM_SIZE; i++) {
        AtomicFrame* f = (AtomicFrame*)WAVE_PTR(&g_stream, i);
        uint64_t val = g_writer_seq + i;
        for(int k=0; k<7; k++) f->data[k] = val;
        f->seq = g_writer_seq;
        atomic_thread_fence(memory_order_release);
        f->checksum = calc_checksum(f);
        atomic_thread_fence(memory_order_release);
        atomic_store(&g_safe_idx, i);
    }
    g_writer_seq++;
    return NULL;
}

void* reader_thread(void* arg) {
    (void)arg;
    for (uint32_t i = 0; i < STREAM_SIZE; i++) {
        AtomicFrame local;
        wave_load_any(&g_stream, i, &local, sizeof(AtomicFrame));
        uint32_t expected = calc_checksum(&local);
        if (expected == local.checksum) {
            atomic_fetch_add(&g_valid_reads, 1);
        } else {
            atomic_fetch_add(&g_torn_reads, 1);
        }
    }
    return NULL;
}

// Dispatcher task for Zhao scheduler
static void dispatcher_task(void *arg) {
    LauWireThread *wt = (LauWireThread *)arg;
    while (atomic_load(&g_running)) {
        tsfi_wire_thread_dispatch(wt);
    }
}

int main(int argc, char** argv) {
    printf("=== TSFi Stream Integrity (Augmented CRC32) ===\\n");
    g_stream.atom_size = 64;
    g_stream.stride = 64;
    g_stream.count = STREAM_SIZE;
    g_stream.flags = WAVE_FLAG_CHECKSUMMED;
    if (argc > 1 && strcmp(argv[1], "--backpressure") == 0) {
        g_stream.flags |= WAVE_FLAG_BACKPRESSURE;
        printf("[MODE] Back-Pressure Enabled (Goal: 0 Tears)\\n");
    } else {
        printf("[MODE] Optimistic (High Throughput)\\n");
    }
    g_stream.data = lau_memalign(64, STREAM_SIZE * 64);
    LauWireThread *t_writer = tsfi_wire_thread_create(1, 1.0);
    LauWireThread *t_reader = tsfi_wire_thread_create(2, 1.0);
    t_writer->persistent = true;
    t_reader->persistent = true;
    t_writer->start_routine = writer_thread;
    t_reader->start_routine = reader_thread;
    g_running = true;

    tsfi_zhao_init(2);
    tsfi_zhao_submit(dispatcher_task, t_writer);
    tsfi_zhao_submit(dispatcher_task, t_reader);

    // Use a custom timer to stop the experiment
    uint64_t start_time = _rdtsc();
    uint64_t runtime_cycles = 2000000000; // Rough approximation for 200ms
    while (_rdtsc() - start_time < runtime_cycles) {
        _mm_pause();
    }
    atomic_store(&g_running, false);
    tsfi_zhao_sync();

    printf("  Valid Reads: %ld\n", g_valid_reads);
    printf("  Torn Reads : %ld\n", g_torn_reads);
    if (g_valid_reads > 0) printf("[PASS] Stream processing sustained.\n");
    else printf("[FAIL] No valid reads!\n");

    tsfi_zhao_shutdown();

    LauSystemHeader *h_w = (LauSystemHeader *)((char *)t_writer - offsetof(LauSystemHeader, payload));
    h_w->proxy = NULL;
    lau_free(t_writer);
    LauSystemHeader *h_r = (LauSystemHeader *)((char *)t_reader - offsetof(LauSystemHeader, payload));
    h_r->proxy = NULL;
    lau_free(t_reader);
    lau_free(g_stream.data);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
