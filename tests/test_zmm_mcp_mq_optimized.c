#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

// Yul Opcode definition
typedef enum {
    YUL_OP_IPC_SEND = 0x70,
    YUL_OP_IPC_RECV = 0x71
} YulOpcode;

// PPN Account Structure
typedef struct {
    uint16_t project;
    uint16_t programmer;
} PPN;

// WinchesterMQ Message Format
typedef struct {
    uint8_t  payload[240];
    PPN      sender;
    PPN      recipient;
    uint64_t gas_fee; // Optimization 1: Priority Queueing based on fee
} WinchesterMessage;

#define MAILBOX_SIZE 4
typedef struct {
    WinchesterMessage messages[MAILBOX_SIZE];
    uint32_t count;
} Mailbox;

// Job Slot
#define MAX_JOBS 8
typedef struct {
    uint32_t job_id;
    PPN      ppn;
    Mailbox  mailbox;
    uint64_t gas_balance;
    bool     is_active;
    bool     is_sleeping; // Optimization 2: Sleeping state for scheduling
} JobSlot;

JobSlot g_jobs[MAX_JOBS];
uint32_t g_scheduler_interrupt_count = 0; // Tracks interrupt reschedules

uint32_t get_job_index(PPN ppn) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (g_jobs[i].is_active && g_jobs[i].ppn.project == ppn.project && g_jobs[i].ppn.programmer == ppn.programmer) {
            return i;
        }
    }
    return 0xFFFFFFFF;
}

void init_jobs(void) {
    memset(g_jobs, 0, sizeof(g_jobs));
    g_scheduler_interrupt_count = 0;
    
    // Setup Job 0 (PPN [10, 5])
    g_jobs[0] = (JobSlot){ .job_id = 0, .ppn = {10, 5}, .gas_balance = 200, .is_active = true, .is_sleeping = false };
    // Setup Job 1 (PPN [10, 6])
    g_jobs[1] = (JobSlot){ .job_id = 1, .ppn = {10, 6}, .gas_balance = 200, .is_active = true, .is_sleeping = false };
}

// Push message into Mailbox using Gas-Ordered Priority sorting (Descending)
bool mailbox_push_priority(Mailbox *mb, WinchesterMessage msg, const char **out_err) {
    if (mb->count >= MAILBOX_SIZE) {
        *out_err = "REVERT: IPC_MAILBOX_FULL";
        return false;
    }
    
    // Insert sorting
    int i = mb->count - 1;
    while (i >= 0 && mb->messages[i].gas_fee < msg.gas_fee) {
        mb->messages[i + 1] = mb->messages[i];
        i--;
    }
    mb->messages[i + 1] = msg;
    mb->count++;
    return true;
}

// Pop highest priority message
bool mailbox_pop(Mailbox *mb, WinchesterMessage *out_msg) {
    if (mb->count == 0) return false;
    
    // Top priority is at index 0
    *out_msg = mb->messages[0];
    
    // Shift left
    for (uint32_t i = 1; i < mb->count; i++) {
        mb->messages[i - 1] = mb->messages[i];
    }
    mb->count--;
    return true;
}

// Send IPC message with custom gas fee (tip)
bool ipc_send_optimized(PPN sender, PPN recipient, const char *msg_text, uint64_t gas_fee, const char **out_err) {
    uint32_t dest_idx = get_job_index(recipient);
    if (dest_idx == 0xFFFFFFFF) {
        *out_err = "REVERT: IPC_RECIPIENT_NOT_FOUND";
        return false;
    }
    
    WinchesterMessage msg;
    memset(msg.payload, 0, sizeof(msg.payload));
    strncpy((char*)msg.payload, msg_text, sizeof(msg.payload) - 1);
    msg.sender = sender;
    msg.recipient = recipient;
    msg.gas_fee = gas_fee;
    
    // Push message to destination priority mailbox
    if (!mailbox_push_priority(&g_jobs[dest_idx].mailbox, msg, out_err)) {
        return false;
    }
    
    printf("   [IPC Send] PPN [%u,%u] queued priority message to PPN [%u,%u] with %lu Gas fee: '%s'\n",
           sender.project, sender.programmer, recipient.project, recipient.programmer, gas_fee, msg_text);
           
    // Optimization 2: Interrupt-Driven Scheduler Wakeup
    if (g_jobs[dest_idx].is_sleeping) {
        g_jobs[dest_idx].is_sleeping = false;
        g_scheduler_interrupt_count++;
        printf("   [Scheduler Interrupt] Job %u woken up! Scheduler reschedule count: %u\n",
               dest_idx, g_scheduler_interrupt_count);
    }
    
    return true;
}

// Receive IPC message
bool ipc_recv_optimized(PPN receiver, char *out_text, PPN *out_sender, uint64_t *out_gas, const char **out_err) {
    uint32_t idx = get_job_index(receiver);
    if (idx == 0xFFFFFFFF) {
        *out_err = "REVERT: INVALID_RECEIVER";
        return false;
    }
    
    WinchesterMessage msg;
    if (!mailbox_pop(&g_jobs[idx].mailbox, &msg)) {
        *out_err = "REVERT: IPC_MAILBOX_EMPTY";
        return false;
    }
    
    strcpy(out_text, (char*)msg.payload);
    *out_sender = msg.sender;
    *out_gas = msg.gas_fee;
    
    printf("   [IPC Recv] PPN [%u,%u] read priority message from PPN [%u,%u]: '%s' (Gas fee: %lu)\n",
           receiver.project, receiver.programmer, msg.sender.project, msg.sender.programmer, out_text, msg.gas_fee);
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: OPTIMIZED WINCHESTER_MQ PRIORITY & WAKEUP TESTS\n");
    printf("=============================================================\n");

    init_jobs();
    const char *err = NULL;

    PPN job_alice = { .project = 10, .programmer = 5 };
    PPN job_bob   = { .project = 10, .programmer = 6 };

    // --- TEST 1: Gas-Ordered Priority Queueing ---
    printf("1. Running priority queueing test...\n");
    // Send message 1: low gas (15)
    assert(ipc_send_optimized(job_alice, job_bob, "LOW_GAS_MSG_1", 15, &err) == true);
    // Send message 2: high gas (50)
    assert(ipc_send_optimized(job_alice, job_bob, "HIGH_GAS_MSG_2", 50, &err) == true);
    // Send message 3: medium gas (30)
    assert(ipc_send_optimized(job_alice, job_bob, "MED_GAS_MSG_3", 30, &err) == true);
    printf("   ✓ Queue populated.\n\n");

    printf("Retrieving messages sequentially...\n");
    char buffer[256];
    PPN sender;
    uint64_t fee;
    
    // Pop 1: should be HIGH_GAS_MSG_2 (50 gas)
    assert(ipc_recv_optimized(job_bob, buffer, &sender, &fee, &err) == true);
    assert(strcmp(buffer, "HIGH_GAS_MSG_2") == 0);
    assert(fee == 50);
    
    // Pop 2: should be MED_GAS_MSG_3 (30 gas)
    assert(ipc_recv_optimized(job_bob, buffer, &sender, &fee, &err) == true);
    assert(strcmp(buffer, "MED_GAS_MSG_3") == 0);
    assert(fee == 30);
    
    // Pop 3: should be LOW_GAS_MSG_1 (15 gas)
    assert(ipc_recv_optimized(job_bob, buffer, &sender, &fee, &err) == true);
    assert(strcmp(buffer, "LOW_GAS_MSG_1") == 0);
    assert(fee == 15);
    printf("   ✓ Priority order confirmed successfully.\n\n");

    // --- TEST 2: Interrupt-Driven Scheduler Wakeup ---
    printf("2. Running Interrupt Wakeup test...\n");
    // Set Bob to sleep
    g_jobs[1].is_sleeping = true;
    assert(g_scheduler_interrupt_count == 0);
    printf("   ✓ Job 1 (Bob) set to sleep.\n");

    // Alice sends a message to sleeping Bob
    assert(ipc_send_optimized(job_alice, job_bob, "WAKE_UP_CALL", 15, &err) == true);
    
    // Verify Bob is woken up and scheduler interrupt triggered
    assert(g_jobs[1].is_sleeping == false);
    assert(g_scheduler_interrupt_count == 1);
    printf("   ✓ Bob was immediately woken up by interrupt. Reschedule executed.\n\n");

    printf("=============================================================\n");
    printf("AUNCIENT OPTIMIZED MQ TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
