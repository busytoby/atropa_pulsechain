#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <openssl/sha.h>

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// XCOM Packet representation
typedef struct {
    uint32_t seq;
    uint32_t nonce;
    char payload[64];
} xcom_packet_t;

// XCOM Receiver state
typedef struct {
    uint32_t expected_seq;
    uint32_t last_nonce;
    gate_state_t write_gate;
    char rx_buffer[64];
} xcom_receiver_t;

// -------------------------------------------------------------
// Improved XCOM Receiver Loop
// -------------------------------------------------------------

// Simplified DFA check: returns true if payload is clean (no banned keywords)
static bool xcom_dfa_audit(const char *payload) {
    // Audit check for banned word 'Child-Langmuir'
    if (strstr(payload, "Child-Langmuir") != NULL) {
        return false; // Banned signature detected
    }
    return true; // Clean
}

// Process incoming XCOM packet
bool xcom_process_packet(xcom_receiver_t *rx, const xcom_packet_t *pkt, uint8_t tag) {
    rx->write_gate = CUTOFF_STATE;

    // 1. Run DFA packet audit (L2 firewall check)
    if (!xcom_dfa_audit(pkt->payload)) {
        return false;
    }

    // 2. Privilege Tag verification (TAG_KERNEL = 0x01)
    if (tag != 0x01) {
        return false;
    }

    // 3. Sequence check (Anti-Replay)
    if (pkt->seq != rx->expected_seq) {
        return false;
    }

    // 4. Challenge-Response nonce verification
    if (pkt->nonce != rx->last_nonce + 1) {
        return false;
    }

    // Conduction established -> commit write
    rx->write_gate = CONDUC_STATE;
    strcpy(rx->rx_buffer, pkt->payload);
    rx->expected_seq++;
    rx->last_nonce = pkt->nonce;
    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT XCOM RECEIVER VERIFICATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    xcom_receiver_t rx = {
        .expected_seq = 100,
        .last_nonce = 500,
        .write_gate = CUTOFF_STATE,
        .rx_buffer = ""
    };

    // 1. Process valid packet (Correct seq, nonce response, clean payload, authorized tag)
    printf("[TEST] Processing valid XCOM packet...\n");
    fflush(stdout);
    xcom_packet_t pkt_ok = {
        .seq = 100,
        .nonce = 501,
        .payload = "XCOM_DATA_TRANSPARENT"
    };
    
    bool ok = xcom_process_packet(&rx, &pkt_ok, 0x01);
    assert(ok == true);
    assert(rx.write_gate == CONDUC_STATE);
    assert(strcmp(rx.rx_buffer, "XCOM_DATA_TRANSPARENT") == 0);
    assert(rx.expected_seq == 101);
    assert(rx.last_nonce == 501);
    printf("   ✓ Valid packet accepted (conduction active).\n");
    fflush(stdout);

    // 2. Block packet violating DFA security filter (contains banned Child-Langmuir word)
    printf("[TEST] Processing packet containing banned payload...\n");
    fflush(stdout);
    xcom_packet_t pkt_bad_payload = {
        .seq = 101,
        .nonce = 502,
        .payload = "Formula: Child-Langmuir equation details"
    };

    ok = xcom_process_packet(&rx, &pkt_bad_payload, 0x01);
    assert(ok == false);
    assert(rx.write_gate == CUTOFF_STATE); // Dropped
    assert(strcmp(rx.rx_buffer, "XCOM_DATA_TRANSPARENT") == 0); // Unaltered
    printf("   ✓ Banned payload blocked successfully by DFA gate.\n");
    fflush(stdout);

    // 3. Block replayed packet (Old sequence 100)
    printf("[TEST] Processing replayed packet...\n");
    fflush(stdout);
    xcom_packet_t pkt_replay = {
        .seq = 100,
        .nonce = 502,
        .payload = "REPLAY_ATTEMPT_DATA"
    };

    ok = xcom_process_packet(&rx, &pkt_replay, 0x01);
    assert(ok == false);
    assert(rx.write_gate == CUTOFF_STATE); // Dropped
    printf("   ✓ Replayed packet blocked successfully by sequence check.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("XCOM RECEIVER VERIFICATION COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
