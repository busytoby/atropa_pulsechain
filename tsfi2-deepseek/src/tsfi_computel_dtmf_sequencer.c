#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_computel_trunk_selector.c"

/*
 * Auncient Computel DTMF Command Sequencer
 * 
 * Accumulates decoded DTMF digits and parses switching commands to control
 * the trunk selector matrix.
 */

#define SEQ_BUFFER_SIZE 16

typedef struct {
    char key_buffer[SEQ_BUFFER_SIZE];
    uint8_t buffer_len;
    bool command_executed;
} DtmfSequencer;

void init_dtmf_sequencer(DtmfSequencer *seq) {
    if (!seq) return;
    memset(seq->key_buffer, 0, SEQ_BUFFER_SIZE);
    seq->buffer_len = 0;
    seq->command_executed = false;
}

/* Appends a new digit and processes the sequence if terminated by '#' */
void feed_dtmf_digit(DtmfSequencer *seq, TrunkSelectorState *state, char digit) {
    if (!seq || !state || digit == '\0') return;

    if (seq->buffer_len < SEQ_BUFFER_SIZE - 1) {
        seq->key_buffer[seq->buffer_len++] = digit;
        seq->key_buffer[seq->buffer_len] = '\0';
    }

    // Process command on '#' terminator
    if (digit == '#') {
        seq->key_buffer[seq->buffer_len - 1] = '\0'; // Remove '#' from buffer
        printf("[Computel Sequencer] Processing DTMF Command: %s\n", seq->key_buffer);

        if (seq->key_buffer[0] == '*') {
            // Hypervisor Command Override: e.g. "*0" Resets all trunks
            if (strcmp(seq->key_buffer, "*0") == 0) {
                init_trunk_selector(state);
                printf("[Computel Sequencer] Master reset command executed.\n");
                seq->command_executed = true;
            }
        } else {
            // Assume it is a trunk ID dialing command (e.g. "802")
            int dialed_trunk = atoi(seq->key_buffer);
            if (dialed_trunk >= 800 && dialed_trunk < 816) {
                printf("[Computel Sequencer] Dialing Trunk %d...\n", dialed_trunk);
                // Attempt to bridge calling line with the dialed trunk
                establish_loopback_bridge(state, 800, dialed_trunk);
                seq->command_executed = true;
            }
        }

        // Clear buffer after processing
        memset(seq->key_buffer, 0, SEQ_BUFFER_SIZE);
        seq->buffer_len = 0;
    }
}
