#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

typedef struct {
    char precedence_code; // 'F'=Flash, 'I'=Immediate, 'P'=Priority, 'R'=Routine
    uint32_t routing_code;
    uint8_t security_level; // 0=Unclassified, 1=Confidential, 2=Secret, 3=TopSecret
    uint32_t payload_val;
    uint16_t checksum;
} autodin_frame_t;

typedef struct {
    uint8_t node_clearance;
    uint32_t messages_sent;
} autodin_node_t;

// -------------------------------------------------------------
// AUTODIN DbC Contract Verification (Bell-LaPadula model)
// -------------------------------------------------------------
bool verify_autodin_preconditions(const autodin_frame_t *frame) {
    // Pre-condition 1: Validate precedence code
    char p = frame->precedence_code;
    if (p != 'F' && p != 'I' && p != 'P' && p != 'R') {
        return false;
    }

    // Pre-condition 2: Validate frame checksum (simple algebraic match)
    uint16_t calculated = (uint16_t)((frame->routing_code + frame->payload_val) & 0xFFFF);
    if (frame->checksum != calculated) {
        return false;
    }

    return true;
}

bool verify_autodin_node_invariants(const autodin_node_t *node, const autodin_frame_t *frame) {
    // Bell-LaPadula security invariant: No Write-Up
    // A node cannot transmit a message with a security classification higher than its own clearance
    return (frame->security_level <= node->node_clearance);
}

bool transmit_autodin_message(autodin_node_t *node, const autodin_frame_t *frame, uint32_t *audit_log) {
    // 1. Check pre-conditions
    if (!verify_autodin_preconditions(frame)) {
        return false;
    }

    // 2. Check invariants
    if (!verify_autodin_node_invariants(node, frame)) {
        return false;
    }

    // Modify state
    node->messages_sent++;

    // 3. Post-condition: Message successfully recorded in local audit log
    *audit_log = frame->payload_val;
    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT AUTODIN CONTRACT VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    autodin_node_t sender_node = {
        .node_clearance = 2, // Secret clearance
        .messages_sent = 0
    };

    uint32_t audit_log = 0;

    // 1. Valid Frame: Secret (2), Flash precedence, correct checksum -> Should pass
    autodin_frame_t valid_frame = {
        .precedence_code = 'F',
        .routing_code = 100,
        .security_level = 2,
        .payload_val = 500,
        .checksum = (100 + 500) & 0xFFFF
    };

    printf("[TEST] Transmitting compliant AUTODIN frame...\n");
    fflush(stdout);
    bool ok = transmit_autodin_message(&sender_node, &valid_frame, &audit_log);
    assert(ok == true);
    assert(sender_node.messages_sent == 1);
    assert(audit_log == 500);
    printf("   ✓ Compliant AUTODIN frame transmitted. Invariants satisfied.\n");
    fflush(stdout);

    // 2. Invalid Frame: Correct classification but corrupted checksum -> Should fail pre-conditions
    autodin_frame_t bad_checksum_frame = {
        .precedence_code = 'F',
        .routing_code = 100,
        .security_level = 2,
        .payload_val = 500,
        .checksum = 0xFFFF // Invalid
    };

    printf("[TEST] Transmitting frame with corrupted checksum (expected fail)...\n");
    fflush(stdout);
    ok = transmit_autodin_message(&sender_node, &bad_checksum_frame, &audit_log);
    assert(ok == false);
    assert(sender_node.messages_sent == 1); // Unchanged
    printf("   ✓ Corrupted checksum successfully caught by pre-conditions.\n");
    fflush(stdout);

    // 3. Security Violation Frame: TopSecret (3) sent by Secret (2) node -> Should fail invariant check
    autodin_frame_t write_up_frame = {
        .precedence_code = 'I',
        .routing_code = 200,
        .security_level = 3, // Exceeds node clearance
        .payload_val = 900,
        .checksum = (200 + 900) & 0xFFFF
    };

    printf("[TEST] Transmitting frame exceeding security clearance (expected fail)...\n");
    fflush(stdout);
    ok = transmit_autodin_message(&sender_node, &write_up_frame, &audit_log);
    assert(ok == false);
    assert(sender_node.messages_sent == 1); // Unchanged
    printf("   ✓ Write-up attempt trapped and rejected by security invariant.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("AUTODIN CONTRACT TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
