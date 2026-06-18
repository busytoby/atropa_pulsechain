#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

// Mock ZMM VM Structures
typedef struct {
    uint8_t status; // 1 = Active, 0 = Suspended
    uint8_t idle;   // 1 = Idle/Sleeping, 0 = Active
} GuestPCB;

typedef struct {
    uint32_t head;
    uint32_t tail;
    uint32_t pending_ack; // Slot 0x30 lease tracking
    uint8_t sectors[256][256];
} VirtualLUN;

GuestPCB g_pcb[6];       // Scaled down to 6 slots
VirtualLUN g_luns[8];
uint32_t g_active_card = 0;

void init_pipeline_hardware() {
    memset(g_pcb, 0, sizeof(g_pcb));
    memset(g_luns, 0, sizeof(g_luns));
    for (int i = 0; i < 8; i++) {
        g_luns[i].pending_ack = 0xFFFFFFFF; // Clear leases
    }
}

void mock_mq_put(uint8_t lun, uint32_t lba, uint8_t* data) {
    // Flow Control check: queue capacity cap at 16
    uint32_t size = 0;
    if (g_luns[lun].head < g_luns[lun].tail) {
        size = g_luns[lun].tail - g_luns[lun].head;
    }
    if (size >= 16) {
        printf("[VM WARNING] LUN %d Queue Full (Flow Control Backpressure)!\n", lun);
        return; // Abort write
    }

    memcpy(g_luns[lun].sectors[lba], data, 256);
    if (lba == g_luns[lun].tail) {
        g_luns[lun].tail++;
    }
}

// 2-Phase Commit read: leases block but does not advance head
int mock_mq_get_phase1(uint8_t lun, uint32_t lba, uint8_t* out_data) {
    memcpy(out_data, g_luns[lun].sectors[lba], 256);
    g_luns[lun].pending_ack = lba; // Lock lease
    return 0;
}

// Commits consumption and permanently increments head
int mock_mq_ack_phase2(uint8_t lun, uint32_t lba) {
    if (g_luns[lun].pending_ack == lba) {
        if (lba == g_luns[lun].head) {
            g_luns[lun].head++; // Consume block
        }
        g_luns[lun].pending_ack = 0xFFFFFFFF; // Clear lease
        return 0; // Commit success
    }
    return -1; // Fail
}

// Scheduler dispatch helper
uint32_t scheduler_find_ready() {
    for (uint32_t i = 0; i < 6; i++) {
        uint32_t candidate = (g_active_card + i) % 6;
        if (g_pcb[candidate].status == 1) {
            // Wake up if inbox LUN has pending messages or process is actively running
            uint32_t head = g_luns[candidate].head;
            uint32_t tail = g_luns[candidate].tail;
            if (head < tail || g_pcb[candidate].idle == 0) {
                g_active_card = candidate;
                return candidate;
            }
        }
    }
    return 0xFFFFFFFF;
}

// =============================================================================
// PIPELINE RUNNER
// =============================================================================
int main() {
    printf("=== RUNNING WINCHESTER_MQ 3-STAGE ASYNC PIPELINE SIMULATION ===\n");
    init_pipeline_hardware();

    // Setup Process Cards
    g_pcb[1].status = 1; g_pcb[1].idle = 0; // Card 1: Parser (Always active/running)
    g_pcb[2].status = 1; g_pcb[2].idle = 1; // Card 2: Evaluator (Idle, listens to LUN 2 inbox)
    g_pcb[3].status = 1; g_pcb[3].idle = 1; // Card 3: Logger (Idle, listens to LUN 3 inbox)

    // STAGE 1: Parser runs and publishes data to LUN 2
    printf("[STAGE 1] Parser running...\n");
    uint32_t active = scheduler_find_ready();
    assert(active == 1); // Card 1 runs

    uint8_t telemetry[256];
    memset(telemetry, 0, 256);
    telemetry[0] = 'D'; telemetry[1] = 'A'; telemetry[2] = 'T'; telemetry[3] = 'A'; // Magic
    telemetry[12] = 0x11; // CorrelId

    mock_mq_put(2, g_luns[2].tail, telemetry); // Parser writes parsed data to LUN 2 (Card 2 inbox)
    printf("  Parser wrote block to LUN 2. Queue tail: %d\n", g_luns[2].tail);
    
    // Parser yields and goes to sleep
    g_pcb[1].idle = 1; 

    // STAGE 2: Evaluator wakes up because LUN 2 has data
    printf("[STAGE 2] Scheduler checking for ready processes...\n");
    active = scheduler_find_ready();
    assert(active == 2); // Card 2 wakes up via queue trigger

    g_pcb[2].idle = 0; // Wakes up
    uint8_t incoming[256];
    int res = mock_mq_get_phase1(2, g_luns[2].head, incoming);
    assert(res == 0);
    assert(incoming[0] == 'D' && g_luns[2].pending_ack == 0); // Leased, head not advanced yet

    // Evaluator evaluates data and prepares log
    uint8_t eval_log[256];
    memset(eval_log, 0, 256);
    eval_log[0] = 'E'; eval_log[1] = 'V'; eval_log[2] = 'A'; eval_log[3] = 'L'; // Magic
    memcpy(&eval_log[12], &incoming[12], 32); // Preserve Correlation ID

    mock_mq_put(3, g_luns[3].tail, eval_log); // Writes evaluation log to LUN 3 (Card 3 inbox)
    printf("  Evaluator wrote block to LUN 3. Queue tail: %d\n", g_luns[3].tail);

    // Commit consumer ACK on LUN 2
    mock_mq_ack_phase2(2, 0); 
    printf("  Evaluator committed ACK on LUN 2. Head index advanced: %d\n", g_luns[2].head);
    assert(g_luns[2].head == 1);

    // Evaluator yields and goes to sleep
    g_pcb[2].idle = 1;

    // STAGE 3: Logger wakes up because LUN 3 has data
    printf("[STAGE 3] Scheduler checking for ready processes...\n");
    active = scheduler_find_ready();
    assert(active == 3); // Card 3 wakes up

    g_pcb[3].idle = 0; // Wakes up
    uint8_t final_log[256];
    res = mock_mq_get_phase1(3, g_luns[3].head, final_log);
    assert(res == 0);
    assert(final_log[0] == 'E' && final_log[12] == 0x11);

    // Write final audit commit to LUN 0 (System Cartridge)
    mock_mq_put(0, 100, final_log);
    printf("  Logger committed audit record to LUN 0.\n");

    // Commit consumer ACK on LUN 3
    mock_mq_ack_phase2(3, 0);
    printf("  Logger committed ACK on LUN 3. Head index advanced: %d\n", g_luns[3].head);
    assert(g_luns[3].head == 1);

    g_pcb[3].idle = 1;

    printf("=== PIPELINE RUN COMPLETED SUCCESSFULLY ===\n");
    return 0;
}
