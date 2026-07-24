#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define MAX_PREFETCH_SIZE 8

typedef enum {
    PREC_ROUTINE = 1,
    PREC_IMMEDIATE = 2,
    PREC_FLASH = 3
} autodin_precedence_t;

typedef enum {
    SEC_UNCLASSIFIED = 1,
    SEC_SECRET = 2,
    SEC_TOP_SECRET = 3
} autodin_security_t;

typedef struct {
    uint64_t sequence;
    autodin_precedence_t precedence;
    autodin_security_t security_level;
    char payload[64];
} autodin_msg_t;

typedef struct {
    autodin_msg_t queue[MAX_PREFETCH_SIZE];
    int count;
} autodin_prefetch_queue_t;

// Insert message keeping the queue sorted by precedence (highest precedence first)
static void enqueue_precedence(autodin_prefetch_queue_t *pq, const autodin_msg_t *msg) {
    if (pq->count >= MAX_PREFETCH_SIZE) {
        return; // Full
    }

    // Find insertion index based on precedence
    int i = pq->count - 1;
    while (i >= 0 && pq->queue[i].precedence < msg->precedence) {
        pq->queue[i + 1] = pq->queue[i];
        i--;
    }

    pq->queue[i + 1] = *msg;
    pq->count++;
}

// Dequeue the highest-precedence message
static bool dequeue_highest(autodin_prefetch_queue_t *pq, autodin_msg_t *out_msg) {
    if (pq->count == 0) return false;
    
    *out_msg = pq->queue[0];
    
    // Shift queue
    for (int i = 1; i < pq->count; i++) {
        pq->queue[i - 1] = pq->queue[i];
    }
    pq->count--;
    return true;
}

// Bell-LaPadula Security Clearance Verification (No Read Up)
static bool verify_security_gate(autodin_security_t reader_clearance, const autodin_msg_t *msg) {
    return (reader_clearance >= msg->security_level);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT AUTODIN PREFETCH & SECURITY VERIFICATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    autodin_prefetch_queue_t pq = { .count = 0 };

    // 1. Initialize message targets
    autodin_msg_t msg_routine = {
        .sequence = 1,
        .precedence = PREC_ROUTINE,
        .security_level = SEC_SECRET,
        .payload = "Routine transaction data"
    };

    autodin_msg_t msg_flash = {
        .sequence = 2,
        .precedence = PREC_FLASH,
        .security_level = SEC_SECRET,
        .payload = "Critical flash action payload"
    };

    autodin_msg_t msg_top_secret = {
        .sequence = 3,
        .precedence = PREC_IMMEDIATE,
        .security_level = SEC_TOP_SECRET,
        .payload = "Highly sensitive compartmented data"
    };

    // 2. Test Precedence Reordering
    printf("[TEST] Enqueueing messages in arbitrary order...\n");
    fflush(stdout);
    enqueue_precedence(&pq, &msg_routine);
    enqueue_precedence(&pq, &msg_flash);
    enqueue_precedence(&pq, &msg_top_secret);

    // Verify reordering: FLASH (3) first, then IMMEDIATE (2), then ROUTINE (1)
    assert(pq.queue[0].precedence == PREC_FLASH);
    assert(pq.queue[1].precedence == PREC_IMMEDIATE);
    assert(pq.queue[2].precedence == PREC_ROUTINE);
    printf("   ✓ Prefetch queue correctly sorted by precedence order.\n");
    fflush(stdout);

    // 3. Test Security Gate Clearance Checks
    printf("[TEST] Testing security clearance validations...\n");
    fflush(stdout);

    autodin_msg_t rx;
    
    // Dequeue first message (FLASH, SEC_SECRET)
    bool ok = dequeue_highest(&pq, &rx);
    assert(ok == true);
    assert(rx.precedence == PREC_FLASH);
    
    // Verify gate check with SECRET clearance -> should pass
    assert(verify_security_gate(SEC_SECRET, &rx) == true);
    printf("   ✓ FLASH message verified under SECRET clearance.\n");
    fflush(stdout);

    // Dequeue next message (IMMEDIATE, SEC_TOP_SECRET)
    ok = dequeue_highest(&pq, &rx);
    assert(ok == true);
    assert(rx.precedence == PREC_IMMEDIATE);

    // Verify gate check with SECRET clearance -> should FAIL (Bell-LaPadula block)
    assert(verify_security_gate(SEC_SECRET, &rx) == false);
    
    // Verify gate check with TOP_SECRET clearance -> should PASS
    assert(verify_security_gate(SEC_TOP_SECRET, &rx) == true);
    printf("   ✓ TOP_SECRET message blocked under SECRET, permitted under TOP_SECRET.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("AUTODIN PREFETCH & SECURITY TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
