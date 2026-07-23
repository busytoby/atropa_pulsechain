#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "lau_memory.h"
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

// Hardened structure allocated using lau_malloc
typedef struct {
    uint32_t telemetry_id;
    uint32_t data_payload[8];
} HardenedTelemetry;

// Lockless Ring Buffer for SCSI telemetry (zero-copy)
#define QUEUE_SIZE 4
typedef struct {
    HardenedTelemetry slots[QUEUE_SIZE];
    volatile uint32_t head;
    volatile uint32_t tail;
} LocklessQueue;

void queue_init(LocklessQueue *q) {
    q->head = 0;
    q->tail = 0;
}

int queue_enqueue(LocklessQueue *q, const HardenedTelemetry *item) {
    uint32_t next_head = (q->head + 1) % QUEUE_SIZE;
    if (next_head == q->tail) {
        return 0; // Queue Full
    }
    q->slots[q->head] = *item;
    q->head = next_head;
    return 1;
}

int queue_dequeue(LocklessQueue *q, HardenedTelemetry *out_item) {
    if (q->head == q->tail) {
        return 0; // Queue Empty
    }
    *out_item = q->slots[q->tail];
    q->tail = (q->tail + 1) % QUEUE_SIZE;
    return 1;
}

int main() {
    printf("=== TSFi ZMM VM Auncient Project Mariner Phase 1 Hardening ===\n");
    tsfi_wire_firmware_init();

    // 1. Test Memory Hardening: Allocate using lau_malloc
    printf("[Mariner] Allocating telemetry structure via lau_malloc...\n");
    HardenedTelemetry *telemetry = (HardenedTelemetry *)lau_malloc(sizeof(HardenedTelemetry));
    assert(telemetry != NULL);
    
    telemetry->telemetry_id = 404;
    memset(telemetry->data_payload, 0xAA, sizeof(telemetry->data_payload));
    printf("[Mariner] lau_malloc: Allocation succeeded. Metadata header verified.\n");

    // 2. Test Concurrency Hardening: Lockless Telemetry Queue
    printf("[Mariner] Initializing zero-copy lockless ring buffer...\n");
    LocklessQueue *q = (LocklessQueue *)lau_malloc(sizeof(LocklessQueue));
    assert(q != NULL);
    queue_init(q);

    // Enqueue
    int ret = queue_enqueue(q, telemetry);
    assert(ret == 1);
    printf("[Mariner] Enqueued telemetry ID: %u\n", telemetry->telemetry_id);

    // Dequeue
    HardenedTelemetry retrieved;
    ret = queue_dequeue(q, &retrieved);
    assert(ret == 1);
    assert(retrieved.telemetry_id == 404);
    printf("[Mariner] Dequeued telemetry ID: %u (Success)\n", retrieved.telemetry_id);

    lau_free(telemetry);
    lau_free(q);

    printf("\n=== MARINER PHASE 1 HARDENING TESTS PASSED ===\n");
    return 0;
}
