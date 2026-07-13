#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

#define MAX_PEERS 4
#define PAGE_SIZE 4096
#define MAX_PAGES 16
#define QUEUE_SIZE 8

typedef struct {
    uint16_t project;
    uint16_t programmer;
} PPN;

// 1. Dynamic Priority Queueing (Priority: 2 = Admin, 1 = User, 0 = Anonymous)
typedef struct {
    PPN ppn;
    int priority;
    bool is_valid;
} PriorityQueueEntry;

PriorityQueueEntry g_priority_queue[QUEUE_SIZE];
size_t g_queue_count = 0;

void enqueue_priority_tx(PPN ppn, int priority) {
    if (g_queue_count < QUEUE_SIZE) {
        g_priority_queue[g_queue_count++] = (PriorityQueueEntry){ ppn, priority, true };
        printf("   [Priority Enqueue] PPN [%u,%u] (Priority %d) added to queue.\n",
               ppn.project, ppn.programmer, priority);
    }
}

PPN dequeue_priority_tx(void) {
    PPN empty = { 0, 0 };
    if (g_queue_count == 0) return empty;

    // Find highest priority index
    size_t best_idx = 0;
    int max_prio = -1;
    for (size_t i = 0; i < g_queue_count; i++) {
        if (g_priority_queue[i].is_valid && g_priority_queue[i].priority > max_prio) {
            max_prio = g_priority_queue[i].priority;
            best_idx = i;
        }
    }

    PPN result = g_priority_queue[best_idx].ppn;
    
    // Remove element by shifting subsequent entries
    for (size_t i = best_idx; i < g_queue_count - 1; i++) {
        g_priority_queue[i] = g_priority_queue[i + 1];
    }
    g_queue_count--;
    return result;
}

// 2. Frequency-Shift Loopback Calibration with Idle Coaxial Transfers
typedef struct {
    double noise_threshold;
    double calibration_freq_hz;
    bool is_idle_transmitting;
} CoaxialCalibration;

void run_idle_calibration_pulse(CoaxialCalibration *cal) {
    cal->is_idle_transmitting = true;
    // Lower frequency calibration tone for idle telemetry
    cal->calibration_freq_hz = 600.0; 
    cal->noise_threshold = 0.05; // Tighten noise limits to 5% during idle
    printf("   [Calibration Idle] Channel idle. Calibrating noise floor to %.2f at %.1f Hz pilot tone.\n",
           cal->noise_threshold, cal->calibration_freq_hz);
}

// 3. Hardware Page Table Isolation (ZMM)
typedef enum {
    PAGE_PERM_NONE  = 0,
    PAGE_PERM_READ  = 1,
    PAGE_PERM_WRITE = 2,
    PAGE_PERM_EXEC  = 4
} PagePermissions;

typedef struct {
    uintptr_t page_addr;
    PPN owner_ppn;
    PagePermissions perm;
    bool is_allocated;
} AuncientPageTableEntry;

AuncientPageTableEntry g_page_table[MAX_PAGES];

void init_page_table(void) {
    memset(g_page_table, 0, sizeof(g_page_table));
}

bool allocate_zmm_page(uintptr_t addr, PPN owner, PagePermissions perm) {
    for (int i = 0; i < MAX_PAGES; i++) {
        if (!g_page_table[i].is_allocated) {
            g_page_table[i].page_addr = addr;
            g_page_table[i].owner_ppn = owner;
            g_page_table[i].perm = perm;
            g_page_table[i].is_allocated = true;
            return true;
        }
    }
    return false;
}

bool verify_page_access(uintptr_t addr, PPN caller, PagePermissions requested_perm, const char **out_err) {
    uintptr_t aligned_page = addr & ~(PAGE_SIZE - 1);
    for (int i = 0; i < MAX_PAGES; i++) {
        if (g_page_table[i].is_allocated && g_page_table[i].page_addr == aligned_page) {
            // Owner check
            if (g_page_table[i].owner_ppn.project != caller.project ||
                g_page_table[i].owner_ppn.programmer != caller.programmer) {
                *out_err = "REVERT: PAGE_ACCESS_DENIED_OWNERSHIP_VIOLATION";
                return false;
            }
            // Permissions check
            if ((g_page_table[i].perm & requested_perm) != requested_perm) {
                *out_err = "REVERT: PAGE_ACCESS_DENIED_INSUFFICIENT_PERMISSIONS";
                return false;
            }
            return true;
        }
    }
    *out_err = "REVERT: PAGE_FAULT_UNMAPPED_ADDRESS";
    return false;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: PRIORITY QUEUES, IDLE TRANSFERS, AND PT ISOLATION\n");
    printf("=============================================================\n");

    PPN user_peer = { 10, 5 };
    PPN admin_peer = { 11, 1 };

    // 1. Test Priority Queueing
    printf("1. Testing Priority Queueing (Admin bypass)...\n");
    enqueue_priority_tx(user_peer, 1);  // User priority (1)
    enqueue_priority_tx(admin_peer, 2); // Admin priority (2)

    PPN next = dequeue_priority_tx();
    assert(next.project == 11 && next.programmer == 1); // Admin dequeued first
    printf("   ✓ Admin transaction bypasses user transactions correctly.\n");

    next = dequeue_priority_tx();
    assert(next.project == 10 && next.programmer == 5); // User dequeued second
    printf("   ✓ User transaction dequeued correctly.\n\n");

    // 2. Test Idle Calibration
    printf("2. Testing Idle Calibration & Pilot Tones...\n");
    CoaxialCalibration cal = { 0.2, 1200.0, false };
    run_idle_calibration_pulse(&cal);
    assert(cal.is_idle_transmitting == true);
    assert(cal.calibration_freq_hz == 600.0);
    printf("   ✓ Idle calibration pulse executed successfully.\n\n");

    // 3. Test Hardware Page Table Isolation
    printf("3. Testing Hardware Page Table Isolation (ZMM)...\n");
    init_page_table();

    uintptr_t secret_page = 0x55555000;
    allocate_zmm_page(secret_page, admin_peer, PAGE_PERM_READ | PAGE_PERM_WRITE);

    const char *err = NULL;

    // Admin accesses their own page (Allowed)
    bool access_ok = verify_page_access(secret_page + 0x10, admin_peer, PAGE_PERM_READ, &err);
    assert(access_ok == true);
    printf("   ✓ Owner access verified.\n");

    // User accesses Admin page (Blocked)
    access_ok = verify_page_access(secret_page + 0x10, user_peer, PAGE_PERM_READ, &err);
    assert(access_ok == false);
    assert(strcmp(err, "REVERT: PAGE_ACCESS_DENIED_OWNERSHIP_VIOLATION") == 0);
    printf("   ✓ Non-owner access blocked correctly: %s\n\n", err);

    printf("=============================================================\n");
    printf("AUNCIENT MULTIPLEXING AND ISOLATION TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
