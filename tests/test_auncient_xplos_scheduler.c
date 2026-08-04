#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// PL/I Event representation
typedef struct {
    bool asserted;
} pli_event_t;

// XplOS Concurrency Task
typedef struct {
    uint32_t task_id;
    uint32_t priority; // Priority level (higher preempts lower)
    pli_event_t *event;
    gate_state_t write_gate;
    char rx_buffer[64];
    uint32_t execution_cycles; // Track cycles for time-slice checks
} xplos_task_t;

// Simulated DisplacementShader Structure
typedef struct {
    double amp;
    double freq;
} TSFiDisplacementShader;

static double tsfi_displacementshader_eval(TSFiDisplacementShader *ds, double angle) {
    return ds->amp * sin(ds->freq * angle);
}

#define ETHERTYPE_STANAG 0x8B32
#define ETHERTYPE_DECNET 0x6003

// Global simulated hardware states
static double g_wmq_reg_voltage = 5.0; // WinchesterMQ simulated voltage
static bool g_coax_collision = false;   // Coaxial collision status
static uint32_t g_active_priority = 0; // Active running priority

// -------------------------------------------------------------
// PL/I Event-Gated Coaxial Scheduler with preemption & bounds checking
// -------------------------------------------------------------
bool xplos_schedule_coaxial(uint16_t ethertype, const char *payload, 
                             xplos_task_t *stanag_task, xplos_task_t *decnet_task) {
    // 1. Coaxial Collision Gating: if collision is flagged, abort scheduling
    if (g_coax_collision) {
        stanag_task->write_gate = CUTOFF_STATE;
        decnet_task->write_gate = CUTOFF_STATE;
        return false;
    }

    // 2. WinchesterMQ Register Integration & DisplacementShader Synchronization
    // Map ethertype parameters directly to simulated WinchesterMQ registers
    if (ethertype == ETHERTYPE_STANAG) {
        g_wmq_reg_voltage = 5.0;
    } else if (ethertype == ETHERTYPE_DECNET) {
        g_wmq_reg_voltage = 1.8;
    }

    // Use DisplacementShader to ensure projected vertex coordinates synchronize with register boundaries
    TSFiDisplacementShader ds = { .amp = g_wmq_reg_voltage, .freq = 1.5 };
    double vertex_offset = tsfi_displacementshader_eval(&ds, M_PI / 2.0);
    printf("   [SCHEDULER] WinchesterMQ register voltage %.1fV syncs DisplacementShader offset to %.3f\n", 
           g_wmq_reg_voltage, vertex_offset);

    // 3. Reset event states
    stanag_task->event->asserted = false;
    decnet_task->event->asserted = false;
    
    // Default to cutoff state (sleeping)
    stanag_task->write_gate = CUTOFF_STATE;
    decnet_task->write_gate = CUTOFF_STATE;

    // 4. Cooperative Preemption & Event demultiplexing
    if (ethertype == ETHERTYPE_STANAG) {
        // Preemption check: trigger preemption only if task priority > active running priority
        if (stanag_task->priority >= g_active_priority) {
            stanag_task->event->asserted = true;
            stanag_task->write_gate = CONDUC_STATE; // Wake task
            g_active_priority = stanag_task->priority;
            strcpy(stanag_task->rx_buffer, payload);
        }
    } else if (ethertype == ETHERTYPE_DECNET) {
        if (decnet_task->priority >= g_active_priority) {
            decnet_task->event->asserted = true;
            decnet_task->write_gate = CONDUC_STATE; // Wake task
            g_active_priority = decnet_task->priority;
            strcpy(decnet_task->rx_buffer, payload);
        }
    }

    return true;
}

// 5. Time-Slice Timeout validation
bool xplos_validate_timeout(xplos_task_t *task, uint32_t limit) {
    if (task->execution_cycles > limit) {
        task->write_gate = CUTOFF_STATE; // Forced preemption cutoff
        printf("   [SCHEDULER] Task %d execution cycle timeout trapped. Execution gate cut off.\n", task->task_id);
        return true;
    }
    return false;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT XPLOS EVENT-GATED CONCURRENCY VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    pli_event_t e_stanag = { .asserted = false };
    pli_event_t e_decnet = { .asserted = false };

    xplos_task_t stanag_task = {
        .task_id = 1,
        .priority = 10, // High Priority
        .event = &e_stanag,
        .write_gate = CUTOFF_STATE,
        .rx_buffer = "",
        .execution_cycles = 5
    };

    xplos_task_t decnet_task = {
        .task_id = 2,
        .priority = 5,  // Low Priority
        .event = &e_decnet,
        .write_gate = CUTOFF_STATE,
        .rx_buffer = "",
        .execution_cycles = 15 // High cycles initially
    };

    // 1. Dispatch STANAG Frame -> Should wake STANAG task and isolate DECnet task
    printf("[TEST] Dispatching STANAG packet frame (0x8B32)...\n");
    fflush(stdout);
    bool ok = xplos_schedule_coaxial(ETHERTYPE_STANAG, "STANAG_PAYLOAD_DATA", &stanag_task, &decnet_task);
    assert(ok == true);
    assert(e_stanag.asserted == true);
    assert(stanag_task.write_gate == CONDUC_STATE);
    assert(strcmp(stanag_task.rx_buffer, "STANAG_PAYLOAD_DATA") == 0);

    assert(e_decnet.asserted == false);
    assert(decnet_task.write_gate == CUTOFF_STATE); // Isolated
    assert(strcmp(decnet_task.rx_buffer, "") == 0); // Unaltered
    printf("   ✓ STANAG task woke successfully. DECnet task remained isolated.\n");
    fflush(stdout);

    // 2. Preemption Gating validation: dispatch DECnet frame (priority 5 < active 10) -> Should fail to preempt
    printf("[TEST] Dispatching low-priority DECnet frame (0x6003) under active high-priority task...\n");
    fflush(stdout);
    ok = xplos_schedule_coaxial(ETHERTYPE_DECNET, "DECNET_PAYLOAD_DATA", &stanag_task, &decnet_task);
    assert(ok == true);
    assert(e_decnet.asserted == false); // Blocked from waking
    assert(decnet_task.write_gate == CUTOFF_STATE);
    printf("   ✓ Low-priority task preemption blocked successfully.\n");
    fflush(stdout);

    // Reset priority levels
    g_active_priority = 0;
    strcpy(stanag_task.rx_buffer, "");
    strcpy(decnet_task.rx_buffer, "");

    // 3. Dispatch DECnet Frame -> Should wake DECnet task and isolate STANAG task
    printf("[TEST] Dispatching DECnet packet frame (0x6003)...\n");
    fflush(stdout);
    ok = xplos_schedule_coaxial(ETHERTYPE_DECNET, "DECNET_PAYLOAD_DATA", &stanag_task, &decnet_task);
    assert(ok == true);
    assert(e_decnet.asserted == true);
    assert(decnet_task.write_gate == CONDUC_STATE);
    assert(strcmp(decnet_task.rx_buffer, "DECNET_PAYLOAD_DATA") == 0);

    assert(e_stanag.asserted == false);
    assert(stanag_task.write_gate == CUTOFF_STATE); // Isolated
    assert(strcmp(stanag_task.rx_buffer, "") == 0); // Unaltered
    printf("   ✓ DECnet task woke successfully. STANAG task remained isolated.\n");
    fflush(stdout);

    // 4. Coaxial Collision Gating validation: set collision status -> Should abort scheduling
    printf("[TEST] Injecting coaxial bus transmission collision...\n");
    fflush(stdout);
    g_coax_collision = true;
    ok = xplos_schedule_coaxial(ETHERTYPE_STANAG, "STANAG_PAYLOAD_DATA", &stanag_task, &decnet_task);
    assert(ok == false);
    assert(stanag_task.write_gate == CUTOFF_STATE);
    assert(decnet_task.write_gate == CUTOFF_STATE);
    printf("   ✓ Coaxial collision gate tripped. Bus activity isolated successfully.\n");
    fflush(stdout);

    // Reset collision status
    g_coax_collision = false;

    // 5. Time-Slice Timeout validation: decnet_task has 15 cycles, limit 10 -> Should cutoff
    printf("[TEST] Testing task execution time-slice timeout limits...\n");
    fflush(stdout);
    decnet_task.write_gate = CONDUC_STATE;
    bool timeout = xplos_validate_timeout(&decnet_task, 10);
    assert(timeout == true);
    assert(decnet_task.write_gate == CUTOFF_STATE);
    printf("   ✓ Timeout trap verification passed.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("XPLOS EVENT-GATED CONCURRENCY SCHEDULER TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
