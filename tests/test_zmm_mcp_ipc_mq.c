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

// WinchesterMQ Message Format (256 bytes)
typedef struct {
    uint8_t payload[240];
    PPN     sender;
    PPN     recipient;
} WinchesterMessage;

// Circular mailbox queue per Job slot
#define MAILBOX_SIZE 4
typedef struct {
    WinchesterMessage messages[MAILBOX_SIZE];
    uint32_t head;
    uint32_t tail;
} Mailbox;

// Job Slot
#define MAX_JOBS 8
typedef struct {
    uint32_t job_id;
    PPN      ppn;
    Mailbox  mailbox;
    uint64_t gas_balance;
    bool     is_active;
} JobSlot;

JobSlot g_jobs[MAX_JOBS];
const uint64_t UNIVERSAL_GAS_FEE = 15;

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
    
    // Setup Job 0 (PPN [10, 5])
    g_jobs[0] = (JobSlot){ .job_id = 0, .ppn = {10, 5}, .gas_balance = 100, .is_active = true };
    // Setup Job 1 (PPN [10, 6])
    g_jobs[1] = (JobSlot){ .job_id = 1, .ppn = {10, 6}, .gas_balance = 100, .is_active = true };
    // Setup Job 2 (PPN [20, 1])
    g_jobs[2] = (JobSlot){ .job_id = 2, .ppn = {20, 1}, .gas_balance = 100, .is_active = true };
}

// Send IPC message to target job
bool ipc_send(PPN sender, PPN recipient, const char *msg_text, const char **out_err) {
    uint32_t dest_idx = get_job_index(recipient);
    if (dest_idx == 0xFFFFFFFF) {
        *out_err = "REVERT: IPC_RECIPIENT_NOT_FOUND";
        return false;
    }
    
    Mailbox *mb = &g_jobs[dest_idx].mailbox;
    uint32_t next_tail = (mb->tail + 1) % MAILBOX_SIZE;
    if (next_tail == mb->head) {
        *out_err = "REVERT: IPC_MAILBOX_FULL";
        return false;
    }
    
    WinchesterMessage *msg = &mb->messages[mb->tail];
    memset(msg->payload, 0, sizeof(msg->payload));
    strncpy((char*)msg->payload, msg_text, sizeof(msg->payload) - 1);
    msg->sender = sender;
    msg->recipient = recipient;
    
    mb->tail = next_tail;
    printf("   [IPC Send] PPN [%u,%u] sent message to PPN [%u,%u]: '%s'\n",
           sender.project, sender.programmer, recipient.project, recipient.programmer, msg_text);
    return true;
}

// Receive IPC message from inbox
bool ipc_recv(PPN receiver, char *out_text, PPN *out_sender, const char **out_err) {
    uint32_t idx = get_job_index(receiver);
    if (idx == 0xFFFFFFFF) {
        *out_err = "REVERT: INVALID_RECEIVER";
        return false;
    }
    
    Mailbox *mb = &g_jobs[idx].mailbox;
    if (mb->head == mb->tail) {
        *out_err = "REVERT: IPC_MAILBOX_EMPTY";
        return false;
    }
    
    WinchesterMessage *msg = &mb->messages[mb->head];
    strcpy(out_text, (char*)msg->payload);
    *out_sender = msg->sender;
    
    mb->head = (mb->head + 1) % MAILBOX_SIZE;
    printf("   [IPC Recv] PPN [%u,%u] retrieved message from PPN [%u,%u]: '%s'\n",
           receiver.project, receiver.programmer, msg->sender.project, msg->sender.programmer, out_text);
    return true;
}

// Coaxial execution loop
bool process_coaxial_transaction(PPN caller, YulOpcode op, PPN target, const char *msg_text,
                                 char *out_text, PPN *out_sender, const char **out_err) {
    uint32_t idx = get_job_index(caller);
    if (idx == 0xFFFFFFFF) {
        *out_err = "REVERT: UNREGISTERED_CALLER";
        return false;
    }
    
    // Gas charging
    if (g_jobs[idx].gas_balance < UNIVERSAL_GAS_FEE) {
        *out_err = "REVERT: INSUFFICIENT_GAS_BALANCE";
        return false;
    }
    g_jobs[idx].gas_balance -= UNIVERSAL_GAS_FEE;
    
    switch (op) {
        case YUL_OP_IPC_SEND:
            return ipc_send(caller, target, msg_text, out_err);
        case YUL_OP_IPC_RECV:
            return ipc_recv(caller, out_text, out_sender, out_err);
        default:
            *out_err = "REVERT: INVALID_OPCODE";
            return false;
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: JOB TO JOB IPC MQ VERIFICATION\n");
    printf("=============================================================\n");

    init_jobs();
    const char *err = NULL;

    PPN job_alice = { .project = 10, .programmer = 5 };
    PPN job_bob   = { .project = 10, .programmer = 6 };
    PPN job_charlie = { .project = 20, .programmer = 1 };

    // 1. Test Case 1: Send Message from Alice to Bob (Allowed)
    printf("1. Alice sending message to Bob...\n");
    bool res = process_coaxial_transaction(job_alice, YUL_OP_IPC_SEND, job_bob, "HELLO_BOB_FROM_ALICE", NULL, NULL, &err);
    assert(res == true);
    assert(g_jobs[0].gas_balance == 85); // 100 - 15 gas fee
    printf("   ✓ Message queued successfully.\n\n");

    // 2. Test Case 2: Charlie attempts to read Bob's message queue (Denied implicitly by mailbox mapping)
    printf("2. Charlie attempting to read message from Bob's mailbox...\n");
    char buffer[256];
    PPN sender;
    res = process_coaxial_transaction(job_charlie, YUL_OP_IPC_RECV, job_bob, NULL, buffer, &sender, &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: IPC_MAILBOX_EMPTY") == 0); // Charlie can only query his own empty mailbox
    printf("   ✓ Charlie blocked from accessing Bob's mailbox.\n\n");

    // 3. Test Case 3: Bob reads message from Alice (Allowed)
    printf("3. Bob checking mailbox for Alice's message...\n");
    res = process_coaxial_transaction(job_bob, YUL_OP_IPC_RECV, job_alice, NULL, buffer, &sender, &err);
    assert(res == true);
    assert(strcmp(buffer, "HELLO_BOB_FROM_ALICE") == 0);
    assert(sender.project == 10 && sender.programmer == 5);
    assert(g_jobs[1].gas_balance == 85); // 100 - 15 gas fee
    printf("   ✓ Bob successfully retrieved Alice's message.\n\n");

    printf("=============================================================\n");
    printf("AUNCIENT JOB-TO-JOB IPC TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
