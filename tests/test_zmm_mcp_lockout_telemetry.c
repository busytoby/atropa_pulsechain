#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

#define MAX_TELEMETRY_LOGS 16
#define MAX_LOCKED_OUT_PEERS 4
#define LOCKOUT_THRESHOLD 3

// PPN Account Structure
typedef struct {
    uint16_t project;
    uint16_t programmer;
} PPN;

// Telemetry log format for 2-3 tree activity
typedef struct {
    uint64_t timestamp;
    PPN ppn;
    char path_choices[16];
    bool verified;
} Telemetry23Log;

Telemetry23Log g_telemetry_logs[MAX_TELEMETRY_LOGS];
size_t g_telemetry_count = 0;

// Lockout tracking structure
typedef struct {
    PPN ppn;
    uint32_t consecutive_failures;
    bool is_locked_out;
} LockoutEntry;

LockoutEntry g_lockouts[MAX_LOCKED_OUT_PEERS];
size_t g_lockout_count = 0;

// Log to unified 2-3 tree telemetry
void log_23_telemetry(PPN ppn, const char *choices, bool verified) {
    if (g_telemetry_count < MAX_TELEMETRY_LOGS) {
        Telemetry23Log *entry = &g_telemetry_logs[g_telemetry_count++];
        entry->timestamp = 12345678ULL; // Simulated timestamp
        entry->ppn = ppn;
        strncpy(entry->path_choices, choices, sizeof(entry->path_choices) - 1);
        entry->verified = verified;
        printf("   [Telemetry Log] PPN [%u,%u] - Path: %s - Verified: %s\n",
               ppn.project, ppn.programmer, choices, verified ? "TRUE" : "FALSE");
    }
}

// Find or create lockout entry
LockoutEntry *get_or_create_lockout(PPN ppn) {
    for (size_t i = 0; i < g_lockout_count; i++) {
        if (g_lockouts[i].ppn.project == ppn.project &&
            g_lockouts[i].ppn.programmer == ppn.programmer) {
            return &g_lockouts[i];
        }
    }
    if (g_lockout_count < MAX_LOCKED_OUT_PEERS) {
        LockoutEntry *entry = &g_lockouts[g_lockout_count++];
        entry->ppn = ppn;
        entry->consecutive_failures = 0;
        entry->is_locked_out = false;
        return entry;
    }
    return NULL;
}

// Process 2-3 tree traversal signature checking with failure lockout tracking
bool verify_and_dispatch_lau(PPN ppn, const char *path_choices, uint16_t auth_signature, const char **out_err) {
    LockoutEntry *lockout = get_or_create_lockout(ppn);
    if (lockout && lockout->is_locked_out) {
        *out_err = "REVERT: PEER_LOCKED_OUT_DUE_TO_CONSECUTIVE_FAILURES";
        return false;
    }

    // Simulate BigModExp-based 2-3 tree signature validation (expects auth_signature == 999)
    bool is_valid = (auth_signature == 999);

    // Log the traversal attempt to 2-3 tree telemetry
    log_23_telemetry(ppn, path_choices, is_valid);

    if (!is_valid) {
        if (lockout) {
            lockout->consecutive_failures++;
            if (lockout->consecutive_failures >= LOCKOUT_THRESHOLD) {
                lockout->is_locked_out = true;
                printf("[LOCKOUT] PPN [%u,%u] locked out after %d failed attempts!\n",
                       ppn.project, ppn.programmer, lockout->consecutive_failures);
            }
        }
        *out_err = "REVERT: 2-3_TREE_SIGNATURE_VERIFICATION_FAILED";
        return false;
    }

    // Reset failures on success
    if (lockout) {
        lockout->consecutive_failures = 0;
    }

    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: 2-3 TREE TELEMETRY AND LOCKOUT TESTS\n");
    printf("=============================================================\n");

    PPN peer = { .project = 10, .programmer = 5 };
    const char *err = NULL;

    // 1. Attempt 1: Failed signature validation (failure 1)
    printf("1. First invalid attempt from PPN [10,5]...\n");
    bool res = verify_and_dispatch_lau(peer, "L->R->M", 111, &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: 2-3_TREE_SIGNATURE_VERIFICATION_FAILED") == 0);
    printf("   ✓ First failure handled correctly.\n\n");

    // 2. Attempt 2: Failed signature validation (failure 2)
    printf("2. Second invalid attempt from PPN [10,5]...\n");
    res = verify_and_dispatch_lau(peer, "L->R->M", 222, &err);
    assert(res == false);
    printf("   ✓ Second failure handled correctly.\n\n");

    // 3. Attempt 3: Failed signature validation (failure 3 -> triggers Lockout)
    printf("3. Third invalid attempt from PPN [10,5]...\n");
    res = verify_and_dispatch_lau(peer, "L->R->M", 333, &err);
    assert(res == false);
    printf("   ✓ Third failure triggered lockout correctly.\n\n");

    // 4. Attempt 4: Blocked by active lockout
    printf("4. Dispatching subsequent attempt from locked out PPN [10,5]...\n");
    res = verify_and_dispatch_lau(peer, "L->R->M", 999, &err); // Valid signature but locked out
    assert(res == false);
    assert(strcmp(err, "REVERT: PEER_LOCKED_OUT_DUE_TO_CONSECUTIVE_FAILURES") == 0);
    printf("   ✓ Access blocked successfully by active lockout: %s\n\n", err);

    // 5. Verify Telemetry Logging counts
    printf("5. Verifying telemetry log entries...\n");
    assert(g_telemetry_count == 3);
    assert(g_telemetry_logs[0].verified == false);
    assert(strcmp(g_telemetry_logs[0].path_choices, "L->R->M") == 0);
    printf("   ✓ Telemetry verified. Total logs: %lu\n\n", g_telemetry_count);

    printf("=============================================================\n");
    printf("AUNCIENT 2-3 TREE TELEMETRY AND LOCKOUT TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
