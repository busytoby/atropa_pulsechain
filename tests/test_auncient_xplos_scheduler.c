#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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
    pli_event_t *event;
    gate_state_t write_gate;
    char rx_buffer[64];
} xplos_task_t;

#define ETHERTYPE_STANAG 0x8B32
#define ETHERTYPE_DECNET 0x6003

// -------------------------------------------------------------
// PL/I Event-Gated Coaxial Scheduler
// -------------------------------------------------------------
void xplos_schedule_coaxial(uint16_t ethertype, const char *payload, 
                            xplos_task_t *stanag_task, xplos_task_t *decnet_task) {
    // 1. Reset event states
    stanag_task->event->asserted = false;
    decnet_task->event->asserted = false;
    
    // Default to cutoff state (sleeping)
    stanag_task->write_gate = CUTOFF_STATE;
    decnet_task->write_gate = CUTOFF_STATE;

    // 2. Demultiplex and assert target PL/I event directly via hardware gating
    if (ethertype == ETHERTYPE_STANAG) {
        stanag_task->event->asserted = true;
        stanag_task->write_gate = CONDUC_STATE; // Wake task
        strcpy(stanag_task->rx_buffer, payload);
    } else if (ethertype == ETHERTYPE_DECNET) {
        decnet_task->event->asserted = true;
        decnet_task->write_gate = CONDUC_STATE; // Wake task
        strcpy(decnet_task->rx_buffer, payload);
    }
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
        .event = &e_stanag,
        .write_gate = CUTOFF_STATE,
        .rx_buffer = ""
    };

    xplos_task_t decnet_task = {
        .task_id = 2,
        .event = &e_decnet,
        .write_gate = CUTOFF_STATE,
        .rx_buffer = ""
    };

    // 1. Dispatch STANAG Frame -> Should wake STANAG task and isolate DECnet task
    printf("[TEST] Dispatching STANAG packet frame (0x8B32)...\n");
    fflush(stdout);
    xplos_schedule_coaxial(ETHERTYPE_STANAG, "STANAG_PAYLOAD_DATA", &stanag_task, &decnet_task);
    
    assert(e_stanag.asserted == true);
    assert(stanag_task.write_gate == CONDUC_STATE);
    assert(strcmp(stanag_task.rx_buffer, "STANAG_PAYLOAD_DATA") == 0);

    assert(e_decnet.asserted == false);
    assert(decnet_task.write_gate == CUTOFF_STATE); // Isolated
    assert(strcmp(decnet_task.rx_buffer, "") == 0); // Unaltered
    printf("   ✓ STANAG task woke successfully. DECnet task remained isolated.\n");
    fflush(stdout);

    // Reset task buffers
    strcpy(stanag_task.rx_buffer, "");
    strcpy(decnet_task.rx_buffer, "");

    // 2. Dispatch DECnet Frame -> Should wake DECnet task and isolate STANAG task
    printf("[TEST] Dispatching DECnet packet frame (0x6003)...\n");
    fflush(stdout);
    xplos_schedule_coaxial(ETHERTYPE_DECNET, "DECNET_PAYLOAD_DATA", &stanag_task, &decnet_task);

    assert(e_decnet.asserted == true);
    assert(decnet_task.write_gate == CONDUC_STATE);
    assert(strcmp(decnet_task.rx_buffer, "DECNET_PAYLOAD_DATA") == 0);

    assert(e_stanag.asserted == false);
    assert(stanag_task.write_gate == CUTOFF_STATE); // Isolated
    assert(strcmp(stanag_task.rx_buffer, "") == 0); // Unaltered
    printf("   ✓ DECnet task woke successfully. STANAG task remained isolated.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("XPLOS EVENT-GATED CONCURRENCY SCHEDULER TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
