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

// XCOM WinchesterMQ Handshake Simulation State
typedef struct {
    double req_line; // NPN Black base voltage (REQ)
    double ack_line; // PNP Red base voltage (ACK)
    gate_state_t tx_gate;
    gate_state_t rx_gate;
    char data_bus[64];
} xcom_wmq_handshake_t;

// -------------------------------------------------------------
// XCOM WinchesterMQ SCSI Handshake execution
// -------------------------------------------------------------
bool xcom_wmq_step_handshake(xcom_wmq_handshake_t *state, const char *payload) {
    state->tx_gate = CUTOFF_STATE;
    state->rx_gate = CUTOFF_STATE;

    // 1. Assert REQ (NPN Black base voltage high: > 0.7 V)
    if (state->req_line > 0.7) {
        state->tx_gate = CONDUC_STATE;
    }

    // 2. Assert ACK (PNP Red base voltage low: < 0.2 V)
    if (state->ack_line < 0.2) {
        state->rx_gate = CONDUC_STATE;
    }

    // 3. Conduction Check: If both gates conduct, copy data across the SCSI bus
    if (state->tx_gate == CONDUC_STATE && state->rx_gate == CONDUC_STATE) {
        strcpy(state->data_bus, payload);
        return true; // Handshake successful
    }

    return false; // Handshake blocked or incomplete
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT XCOM WINCHESTERMQ SCSI HANDSHAKE VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    xcom_wmq_handshake_t state = {
        .req_line = 0.0,
        .ack_line = 5.0, // High voltage initially (Cutoff)
        .tx_gate = CUTOFF_STATE,
        .rx_gate = CUTOFF_STATE,
        .data_bus = ""
    };

    // 1. Attempt transfer without active REQ/ACK -> Should fail/remain empty
    printf("[TEST] Attempting transfer with inactive REQ/ACK...\n");
    fflush(stdout);
    bool ok = xcom_wmq_step_handshake(&state, "XCOM_WMQ_PAYLOAD");
    assert(ok == false);
    assert(state.tx_gate == CUTOFF_STATE);
    assert(state.rx_gate == CUTOFF_STATE);
    assert(strcmp(state.data_bus, "") == 0);
    printf("   ✓ Transfer blocked successfully. Bus remains idle.\n");
    fflush(stdout);

    // 2. Assert REQ high (5.0V) and ACK low (0.0V) -> Handshake should execute
    printf("[TEST] Asserting REQ high (5.0V) and ACK low (0.0V)...\n");
    fflush(stdout);
    state.req_line = 5.0;
    state.ack_line = 0.0;
    ok = xcom_wmq_step_handshake(&state, "XCOM_WMQ_PAYLOAD");
    assert(ok == true);
    assert(state.tx_gate == CONDUC_STATE);
    assert(state.rx_gate == CONDUC_STATE);
    assert(strcmp(state.data_bus, "XCOM_WMQ_PAYLOAD") == 0);
    printf("   ✓ Handshake successful: %s transmitted.\n", state.data_bus);
    fflush(stdout);

    // 3. Noise injection check: pull ACK line high (5.0V) -> Should block transmission
    printf("[TEST] Injecting noise (pulling ACK high to 5.0V)...\n");
    fflush(stdout);
    strcpy(state.data_bus, "");
    state.req_line = 5.0;
    state.ack_line = 5.0; // Noise surge
    ok = xcom_wmq_step_handshake(&state, "XCOM_WMQ_PAYLOAD");
    assert(ok == false);
    assert(state.rx_gate == CUTOFF_STATE); // Isolated
    assert(strcmp(state.data_bus, "") == 0);
    printf("   ✓ Noise detected: receiver isolated successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("XCOM WINCHESTERMQ SCSI HANDSHAKE TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
