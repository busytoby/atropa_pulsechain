#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/*
 * Auncient Computel Subscriber Line Loop & Trunk Selector
 * 
 * Simulates automatic trunk scanning, line identification (quiet line, 1000 Hz reference),
 * and dynamic loopback bridging for teleconferencing systems.
 */

#define MAX_TRUNKS 16
#define TONE_QUIET 0
#define TONE_REF_1000 1000
#define TONE_FSK 1200

typedef struct {
    uint16_t trunk_id;
    bool is_busy;
    uint32_t active_frequency;
    char line_type[32];
} TrunkLine;

typedef struct {
    TrunkLine trunks[MAX_TRUNKS];
    uint16_t active_bridge_a;
    uint16_t active_bridge_b;
} TrunkSelectorState;

void init_trunk_selector(TrunkSelectorState *state) {
    if (!state) return;
    state->active_bridge_a = 0xFFFF;
    state->active_bridge_b = 0xFFFF;

    for (uint16_t i = 0; i < MAX_TRUNKS; i++) {
        state->trunks[i].trunk_id = 800 + i;
        state->trunks[i].is_busy = false;
        state->trunks[i].active_frequency = TONE_QUIET;
        strcpy(state->trunks[i].line_type, "Quiet Line");
    }

    // Configure specific historical test trunks
    state->trunks[2].active_frequency = TONE_REF_1000;
    strcpy(state->trunks[2].line_type, "1000Hz Milliwatt Reference");

    state->trunks[5].is_busy = true;
    state->trunks[5].active_frequency = TONE_FSK;
    strcpy(state->trunks[5].line_type, "FSK Carrier Line");

    state->trunks[8].active_frequency = 0;
    strcpy(state->trunks[8].line_type, "Loopback Responder");
}

/* Scans trunks looking for active carrier lines or loopback links */
int16_t scan_available_trunks(TrunkSelectorState *state, uint32_t target_freq) {
    if (!state) return -1;
    for (uint16_t i = 0; i < MAX_TRUNKS; i++) {
        if (!state->trunks[i].is_busy && state->trunks[i].active_frequency == target_freq) {
            return state->trunks[i].trunk_id;
        }
    }
    return -1;
}

/* Bridges two trunks together to form a loopback conference link */
bool establish_loopback_bridge(TrunkSelectorState *state, uint16_t trunk_a, uint16_t trunk_b) {
    if (!state) return false;
    
    int intidx_a = -1;
    int intidx_b = -1;

    for (uint16_t i = 0; i < MAX_TRUNKS; i++) {
        if (state->trunks[i].trunk_id == trunk_a) intidx_a = i;
        if (state->trunks[i].trunk_id == trunk_b) intidx_b = i;
    }

    if (intidx_a == -1 || intidx_b == -1) return false;

    state->trunks[intidx_a].is_busy = true;
    state->trunks[intidx_b].is_busy = true;
    state->active_bridge_a = trunk_a;
    state->active_bridge_b = trunk_b;

    printf("[Computel Trunk Selector] Bridge established between Trunk %d and Trunk %d (%s <-> %s)\n",
           trunk_a, trunk_b, state->trunks[intidx_a].line_type, state->trunks[intidx_b].line_type);
    
    return true;
}
