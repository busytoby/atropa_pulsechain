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
    double propagation_delay_ns;
    bool parity_error;
    bool overcurrent_trap;
} xcom_wmq_handshake_t;

// Parity bit calculator (Odd parity)
static bool calculate_odd_parity(const char *data) {
    int count = 0;
    while (*data) {
        char c = *data++;
        for (int i = 0; i < 8; i++) {
            if ((c >> i) & 1) count++;
        }
    }
    return (count % 2) != 0;
}

// -------------------------------------------------------------
// XCOM WinchesterMQ SCSI Handshake execution
// -------------------------------------------------------------
bool xcom_wmq_step_handshake(xcom_wmq_handshake_t *state, const char *payload) {
    state->tx_gate = CUTOFF_STATE;
    state->rx_gate = CUTOFF_STATE;
    state->parity_error = false;

    // 1. Overcurrent Trap: Cut off immediately if voltage exceeds 12.0 V
    if (state->req_line > 12.0 || state->ack_line > 12.0) {
        state->overcurrent_trap = true;
        return false;
    }

    // 2. Electrostatic Decay Delay Solver: t_delay = req_line * ack_line * 2 (simplified model)
    // If delay exceeds standard 10ns limit, simulation slows down
    state->propagation_delay_ns = state->req_line * 1.5;

    // 3. Assert REQ (NPN Black base voltage high: > 0.7 V)
    if (state->req_line > 0.7) {
        state->tx_gate = CONDUC_STATE;
    }

    // 4. Assert ACK (PNP Red base voltage low: < 0.2 V)
    if (state->ack_line < 0.2) {
        state->rx_gate = CONDUC_STATE;
    }

    // 5. Conduction check and transmission
    if (state->tx_gate == CONDUC_STATE && state->rx_gate == CONDUC_STATE) {
        // Bus Parity Verification
        if (!calculate_odd_parity(payload)) {
            state->parity_error = true;
            return false; // Parity failure blocks handshake
        }
        
        strcpy(state->data_bus, payload);
        return true; // Handshake successful
    }

    return false; // Handshake blocked or incomplete
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    xcom_wmq_handshake_t state = {
        .req_line = 0.0,
        .ack_line = 5.0, // High voltage initially (Cutoff)
        .tx_gate = CUTOFF_STATE,
        .rx_gate = CUTOFF_STATE,
        .data_bus = "",
        .propagation_delay_ns = 0.0,
        .parity_error = false,
        .overcurrent_trap = false
    };

    // 1. Attempt transfer without active REQ/ACK
    bool ok = xcom_wmq_step_handshake(&state, "XCOM_WMQ_PAYLOAD"); // Odd parity payload
    assert(ok == false);
    assert(state.tx_gate == CUTOFF_STATE);
    assert(state.rx_gate == CUTOFF_STATE);
    assert(strcmp(state.data_bus, "") == 0);
    printf("[XCOM-WMQ-TEST] Inactive transfer blocked successfully\n");
    fflush(stdout);

    // 2. Assert REQ high (5.0V) and ACK low (0.0V)
    state.req_line = 5.0;
    state.ack_line = 0.0;
    ok = xcom_wmq_step_handshake(&state, "XCOM_WMQ_PAYLOAD");
    assert(ok == true);
    assert(state.tx_gate == CONDUC_STATE);
    assert(state.rx_gate == CONDUC_STATE);
    assert(strcmp(state.data_bus, "XCOM_WMQ_PAYLOAD") == 0);
    assert(state.propagation_delay_ns == 7.5);
    printf("[XCOM-WMQ-TEST] Conduction handshake successful\n");
    fflush(stdout);

    // 3. Parity failure check (even parity payload)
    strcpy(state.data_bus, "");
    ok = xcom_wmq_step_handshake(&state, "XCOM_WMQ_PAYLOAE");
    assert(ok == false);
    assert(state.parity_error == true);
    printf("[XCOM-WMQ-TEST] Frame parity failure trapped\n");
    fflush(stdout);

    // 4. Overcurrent Surge
    state.req_line = 15.0;
    ok = xcom_wmq_step_handshake(&state, "XCOM_WMQ_PAYLOAD");
    assert(ok == false);
    assert(state.overcurrent_trap == true);
    printf("[XCOM-WMQ-TEST] Overcurrent surge isolation trapped\n");
    fflush(stdout);

    return 0;
}
