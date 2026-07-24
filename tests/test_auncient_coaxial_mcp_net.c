#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define BANNED_FORMULA "Child-Langmuir"

// Interop registry for quarantine isolation
typedef struct {
    char quarantined_payload[512];
    bool has_quarantined_data;
} InterceptionQuarantine;

static InterceptionQuarantine g_quarantine = { "", false };

// Simulated guest VM registry state
typedef struct {
    uint64_t chin_register;
    bool is_compromised;
} AuncientVmState;

static AuncientVmState g_vm_state = { 100, false };

// Core auditor that scans incoming payloads for compliance with Rule 12
static bool audit_and_dispatch_coaxial_payload(const char *payload, AuncientVmState *vm_state, const char **out_err) {
    if (strstr(payload, BANNED_FORMULA) != NULL) {
        snprintf(g_quarantine.quarantined_payload, sizeof(g_quarantine.quarantined_payload), "%s", payload);
        g_quarantine.has_quarantined_data = true;
        *out_err = "REVERT: BANNED_CHILD_LANGMUIR_CONTENT_INTERCEPTED";
        return false;
    }

    if (strncmp(payload, "SET_CHIN=", 9) == 0) {
        uint64_t new_chin = strtoull(payload + 9, NULL, 10);
        vm_state->chin_register = new_chin;
        return true;
    }

    *out_err = "REVERT: UNKNOWN_PAYLOAD_TYPE";
    return false;
}

// -------------------------------------------------------------
// Network Dispatch Adapters
// -------------------------------------------------------------

// Process a payload received over STANAG 5066
void process_stanag_mcp_payload(const uint8_t *data, uint16_t len) {
    const char *err = NULL;
    char payload_buf[512];
    
    uint16_t copy_len = len < 511 ? len : 511;
    memcpy(payload_buf, data, copy_len);
    payload_buf[copy_len] = '\0';

    bool success = audit_and_dispatch_coaxial_payload(payload_buf, &g_vm_state, &err);
    if (!success) {
        printf("   [STANAG-MCP] Audit rejected payload: %s\n", err);
    } else {
        printf("   [STANAG-MCP] Audit approved payload. Chin register updated to %lu.\n", g_vm_state.chin_register);
    }
    fflush(stdout);
}

// Process a payload received over DECnet
void process_decnet_mcp_payload(const uint8_t *data, uint16_t len) {
    const char *err = NULL;
    char payload_buf[512];
    
    uint16_t copy_len = len < 511 ? len : 511;
    memcpy(payload_buf, data, copy_len);
    payload_buf[copy_len] = '\0';

    bool success = audit_and_dispatch_coaxial_payload(payload_buf, &g_vm_state, &err);
    if (!success) {
        printf("   [DECNET-MCP] Audit rejected payload: %s\n", err);
    } else {
        printf("   [DECNET-MCP] Audit approved payload. Chin register updated to %lu.\n", g_vm_state.chin_register);
    }
    fflush(stdout);
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT COAXIAL MCP SERVICES OVER NETWORK ADAPTOR SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // 1. Test STANAG transport with benign config payload
    printf("[TEST] Dispatching benign command over STANAG...\n");
    fflush(stdout);
    uint8_t cmd1[] = "SET_CHIN=250";
    process_stanag_mcp_payload(cmd1, sizeof(cmd1));
    assert(g_vm_state.chin_register == 250);

    // 2. Test DECnet transport with malicious payload containing banned Child-Langmuir terms
    printf("[TEST] Dispatching malicious command over DECnet...\n");
    fflush(stdout);
    uint8_t cmd2[] = "Banned Formula: Child-Langmuir";
    process_decnet_mcp_payload(cmd2, sizeof(cmd2));
    assert(g_quarantine.has_quarantined_data == true);
    assert(strcmp(g_quarantine.quarantined_payload, (char *)cmd2) == 0);

    printf("=============================================================\n");
    printf("COAXIAL MCP NETWORK INTEGRATION TESTS COMPLETED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
