#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define MAX_CHORD_TONES 4

// Command actions
typedef enum {
    ACT_LOCK_SCSI,
    ACT_WRITE_LEDGER,
    ACT_NONE
} action_t;

// Filter band definition
typedef struct {
    double center_frequency;
    double bandwidth;
    action_t action;
} bandpass_filter_t;

// System state
typedef struct {
    bool scsi_locked;
    uint32_t ledger_writes;
} poly_system_state_t;

// A mixed audio sample representing an acoustic chord
typedef struct {
    double frequencies[MAX_CHORD_TONES];
    double amplitudes[MAX_CHORD_TONES];
    uint32_t active_tones;
} chord_sample_t;

// Process a mixed acoustic chord through the parallel bandpass filter bank
void process_polyphonic_gating(poly_system_state_t *state, const bandpass_filter_t *filters, uint32_t num_filters, const chord_sample_t *chord) {
    printf("   [OS FILTER BANK] Processing polyphonic chord with %u active frequencies...\n", chord->active_tones);
    
    // For each hardware filter channel, check if any frequency in the chord resonates
    for (uint32_t f = 0; f < num_filters; f++) {
        const bandpass_filter_t *filter = &filters[f];
        bool channel_resonating = false;

        for (uint32_t t = 0; t < chord->active_tones; t++) {
            double diff = fabs(chord->frequencies[t] - filter->center_frequency);
            // If frequency is within the filter's bandwidth window
            if (diff <= filter->bandwidth && chord->amplitudes[t] > 0.5) {
                channel_resonating = true;
                break;
            }
        }

        if (channel_resonating) {
            // Execute the action mapped to this filter band in parallel
            if (filter->action == ACT_LOCK_SCSI) {
                state->scsi_locked = true;
                printf("      - [FILTER PATH] Resonated at %.1fHz: Executed LOCK_SCSI\n", filter->center_frequency);
            } else if (filter->action == ACT_WRITE_LEDGER) {
                state->ledger_writes++;
                printf("      - [FILTER PATH] Resonated at %.1fHz: Executed WRITE_LEDGER\n", filter->center_frequency);
            }
        }
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT POLYPHONIC CHORD GATING TEST SUITE\n");
    printf("=============================================================\n");

    // Configure OS filter bank
    bandpass_filter_t filters[2] = {
        { .center_frequency = 440.0, .bandwidth = 10.0, .action = ACT_LOCK_SCSI },
        { .center_frequency = 350.0, .bandwidth = 10.0, .action = ACT_WRITE_LEDGER }
    };

    poly_system_state_t state = { .scsi_locked = false, .ledger_writes = 0 };

    // 1. Simulate Agent speaking a mixed chord (combining 440Hz and 350Hz formants)
    chord_sample_t mixed_chord = {
        .frequencies = { 440.0, 350.0, 0.0, 0.0 },
        .amplitudes = { 1.0, 1.0, 0.0, 0.0 },
        .active_tones = 2
    };

    printf("[TEST] Sending mixed polyphonic command chord to the OS...\n");
    process_polyphonic_gating(&state, filters, 2, &mixed_chord);

    // Verify both actions executed in parallel from the single mixed chord
    assert(state.scsi_locked == true);
    assert(state.ledger_writes == 1);
    printf("   ✓ Both command actions executed simultaneously from the mixed waveform.\n");

    // 2. Verify single note trigger
    printf("[TEST] Sending single-tone wave (350Hz only)...\n");
    state.scsi_locked = false;
    state.ledger_writes = 0;
    
    chord_sample_t single_tone = {
        .frequencies = { 350.0, 0.0, 0.0, 0.0 },
        .amplitudes = { 1.0, 0.0, 0.0, 0.0 },
        .active_tones = 1
    };
    process_polyphonic_gating(&state, filters, 2, &single_tone);
    
    assert(state.scsi_locked == false);
    assert(state.ledger_writes == 1);
    printf("   ✓ Only the matching filter channel action executed.\n");

    printf("=============================================================\n");
    printf("POLYPHONIC CHORD GATING TESTS PASSED\n");
    printf("=============================================================\n");

    return 0;
}
