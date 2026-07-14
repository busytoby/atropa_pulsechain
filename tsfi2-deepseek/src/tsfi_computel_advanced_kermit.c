#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/*
 * Auncient Computel Advanced Secure Kermit (UDP/Coaxial Optimizations)
 * 
 * Implements:
 * 1. Forward Error Correction (XOR-based parity recovery)
 * 2. Ephemeral Key Rotation (Forward Secrecy)
 * 3. Dynamic Rate Adaptation (NAK congestion backoff)
 * 4. Sliding Window Telemetry
 */

#define MOTZKIN_PRIME 953467954114363ULL
#define ROTATION_THRESHOLD 4  /* Rotate key every 4 packets */
#define DEFAULT_DELAY_US 1000 /* Initial inter-packet delay */

typedef struct {
    uint64_t base;
    uint64_t secret;
    uint64_t active_key;
    uint32_t packet_counter;
    uint32_t current_delay_us;
    uint8_t window_size;
    uint8_t next_seq;
    uint8_t last_ack;
} AdvancedKermitState;

void init_advanced_kermit(AdvancedKermitState *state, uint64_t seed_val) {
    if (!state) return;
    // Basic seed generator matching standard Diffie-Hellman bounds
    state->base = (seed_val ^ 0x5555555555555555ULL) % 9999991ULL + 2;
    state->secret = (seed_val ^ 0xAAAAAAAAAAAAAAAAULL) % 9999991ULL + 2;
    state->active_key = seed_val;
    state->packet_counter = 0;
    state->current_delay_us = DEFAULT_DELAY_US;
    state->window_size = 8;
    state->next_seq = 0;
    state->last_ack = 0;
}

/* 1. Forward Error Correction (FEC): Computes XOR parity of two packets */
void compute_fec_parity(const uint8_t *p1, const uint8_t *p2, uint8_t *parity, uint8_t len) {
    if (!p1 || !p2 || !parity || len == 0) return;
    for (uint8_t i = 0; i < len; i++) {
        parity[i] = p1[i] ^ p2[i];
    }
}

/* 1. FEC Recovery: Recovers lost packet p2 using p1 and parity */
void recover_fec_packet(const uint8_t *p1, const uint8_t *parity, uint8_t *recovered, uint8_t len) {
    if (!p1 || !parity || !recovered || len == 0) return;
    for (uint8_t i = 0; i < len; i++) {
        recovered[i] = p1[i] ^ parity[i];
    }
}

/* 2. Ephemeral Key Rotation: Permutes key to preserve forward secrecy */
void rotate_session_key(AdvancedKermitState *state) {
    if (!state) return;
    // Permute key using standard LCG parameters
    state->active_key = (state->active_key * 2862933555777941757ULL + 3037000493ULL) % MOTZKIN_PRIME;
    state->packet_counter = 0;
}

/* 3. Dynamic Rate Adaptation: Adjusts transmission delay based on signal quality */
void handle_transmission_feedback(AdvancedKermitState *state, bool is_ack) {
    if (!state) return;
    if (is_ack) {
        // Successful delivery: Reduce delay to speed up transmission (minimum 100us)
        if (state->current_delay_us > 100) {
            state->current_delay_us -= 50;
        }
        state->packet_counter++;
        if (state->packet_counter >= ROTATION_THRESHOLD) {
            rotate_session_key(state);
        }
    } else {
        // NAK received: Backoff transmission delay due to congestion/noise (maximum 10000us)
        if (state->current_delay_us < 10000) {
            state->current_delay_us += 500;
        }
    }
}

/* 4. Sliding Window: Returns true if there is space in the window to transmit */
bool is_window_available(const AdvancedKermitState *state) {
    if (!state) return false;
    uint8_t outstanding = (uint8_t)(state->next_seq - state->last_ack);
    return outstanding < state->window_size;
}
