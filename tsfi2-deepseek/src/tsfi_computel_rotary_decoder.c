#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * Auncient Computel Rotary Dial Decadic Pulse Decoder
 * 
 * Decodes rotary dial pulses (Make/Break state sequences) at 10 pulses per second.
 * A break indicates a loop disconnect, and a make indicates loop closure.
 */

#define PULSE_BREAK_MS_MIN 45
#define PULSE_BREAK_MS_MAX 75
#define INTER_DIGIT_PAUSE_MS 700

typedef struct {
    uint32_t current_break_duration_ms;
    uint32_t current_make_duration_ms;
    uint8_t accumulated_pulses;
    bool last_state_was_make;
    bool digit_ready;
    char decoded_digit;
} RotaryDecoderState;

void init_rotary_decoder(RotaryDecoderState *state) {
    if (!state) return;
    state->current_break_duration_ms = 0;
    state->current_make_duration_ms = 0;
    state->accumulated_pulses = 0;
    state->last_state_was_make = true;
    state->digit_ready = false;
    state->decoded_digit = '\0';
}

/* Processes a line-state tick (typically 1ms resolution) */
void process_line_state_tick(RotaryDecoderState *state, bool loop_closed) {
    if (!state) return;
    state->digit_ready = false;

    if (loop_closed) {
        // Line is MAKE
        if (!state->last_state_was_make) {
            // Transition from BREAK to MAKE: Evaluate the pulse break width
            if (state->current_break_duration_ms >= PULSE_BREAK_MS_MIN && 
                state->current_break_duration_ms <= PULSE_BREAK_MS_MAX) {
                state->accumulated_pulses++;
            }
            state->current_break_duration_ms = 0;
        }
        state->current_make_duration_ms++;
        
        // Check for inter-digital pause to finalize digit
        if (state->accumulated_pulses > 0 && state->current_make_duration_ms >= INTER_DIGIT_PAUSE_MS) {
            if (state->accumulated_pulses == 10) {
                state->decoded_digit = '0';
            } else if (state->accumulated_pulses < 10) {
                state->decoded_digit = '0' + state->accumulated_pulses;
            } else {
                state->decoded_digit = '\0'; // Invalid count
            }
            state->digit_ready = true;
            state->accumulated_pulses = 0;
        }
        state->last_state_was_make = true;
    } else {
        // Line is BREAK
        state->current_break_duration_ms++;
        state->current_make_duration_ms = 0;
        state->last_state_was_make = false;
    }
}
