#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "../src/tsfi_computel_rotary_decoder.c"

bool digit_fired = false;
char final_digit = '\0';

/* Simulates a loop state for a duration of milliseconds and records event firings */
void simulate_ticks(RotaryDecoderState *state, bool loop_closed, uint32_t duration_ms) {
    for (uint32_t i = 0; i < duration_ms; i++) {
        process_line_state_tick(state, loop_closed);
        if (state->digit_ready) {
            digit_fired = true;
            final_digit = state->decoded_digit;
        }
    }
}

int main(void) {
    printf("[TEST] Initializing Auncient Computel Rotary Pulse Decoder test suite...\n");

    RotaryDecoderState state;
    init_rotary_decoder(&state);

    // Simulate dialing digit '3' (3 pulses: each pulse has 60ms break and 40ms make)
    // Pulse 1
    simulate_ticks(&state, false, 60); // Break
    simulate_ticks(&state, true, 40);  // Make
    assert(digit_fired == false);

    // Pulse 2
    simulate_ticks(&state, false, 60); // Break
    simulate_ticks(&state, true, 40);  // Make
    assert(digit_fired == false);

    // Pulse 3
    simulate_ticks(&state, false, 60); // Break
    simulate_ticks(&state, true, 40);  // Make
    assert(digit_fired == false);

    // Inter-digit pause (750ms Make) -> Should trigger event
    simulate_ticks(&state, true, 750);
    assert(digit_fired == true);
    printf("[TEST] Decoded rotary digit: %c (expected: 3)\n", final_digit);
    assert(final_digit == '3');

    printf("[SUCCESS] All Computel rotary pulse decoder tests passed successfully.\n");
    return 0;
}
