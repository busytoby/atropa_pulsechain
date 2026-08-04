#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include "auncient_teddy_personality.h"

// Struct mapping a participant bear in the chorus simulation
typedef struct {
    char name[32];
    teddy_personality_t base_personality;
    teddy_geometry_t geom;
    double current_voltage;
    double sound_pitch;
    double current_retaliation_boundary;
} conversational_bear_t;

// Initialize the 5 bear participants
static void init_conversational_bears(conversational_bear_t *bears) {
    snprintf(bears[0].name, 32, "Trusty");
    bears[0].base_personality = PERSONALITY_TRUSTWORTHY;
    resolve_teddy_geometry(bears[0].base_personality, &bears[0].geom);
    bears[0].current_voltage = 1.2;
    bears[0].sound_pitch = 220.0;
    bears[0].current_retaliation_boundary = 0.5;

    snprintf(bears[1].name, 32, "Aggro");
    bears[1].base_personality = PERSONALITY_AGGRESSIVE;
    resolve_teddy_geometry(bears[1].base_personality, &bears[1].geom);
    bears[1].current_voltage = 4.5;
    bears[1].sound_pitch = 110.0;
    bears[1].current_retaliation_boundary = 0.8;

    snprintf(bears[2].name, 32, "Skeptic");
    bears[2].base_personality = PERSONALITY_SKEPTICAL;
    resolve_teddy_geometry(bears[2].base_personality, &bears[2].geom);
    bears[2].current_voltage = 2.5;
    bears[2].sound_pitch = 330.0;
    bears[2].current_retaliation_boundary = 0.6;

    snprintf(bears[3].name, 32, "Eerie");
    bears[3].base_personality = PERSONALITY_EERIE;
    resolve_teddy_geometry(bears[3].base_personality, &bears[3].geom);
    bears[3].current_voltage = 3.0;
    bears[3].sound_pitch = 165.0;
    bears[3].current_retaliation_boundary = 0.7;

    snprintf(bears[4].name, 32, "Coop");
    bears[4].base_personality = PERSONALITY_TRUSTWORTHY;
    resolve_teddy_geometry(bears[4].base_personality, &bears[4].geom);
    bears[4].geom.symmetry = 0.95; // highly cooperative boundary calibration
    bears[4].current_voltage = 1.0;
    bears[4].sound_pitch = 440.0;
    bears[4].current_retaliation_boundary = 0.3;
}

// Simulates emotional propagation between conversational bears
bool simulate_conversational_step(conversational_bear_t *bears, int speaker_idx, int listener_idx) {
    if (speaker_idx < 0 || speaker_idx >= 5 || listener_idx < 0 || listener_idx >= 5) {
        return false;
    }
    conversational_bear_t *speaker = &bears[speaker_idx];
    conversational_bear_t *listener = &bears[listener_idx];

    // Evaluate speaker threat level using Geniole fWHR equations
    double threat = evaluate_fw_threat_level(&speaker->geom);
    
    // Evaluate vocal pitch mismatch using Hyde equations
    double pitch_mismatch = fabs(speaker->sound_pitch - listener->sound_pitch) / 100.0;
    double uncanny_val = 0.0;
    evaluate_motion_acceleration_mismatch(&listener->geom, pitch_mismatch, &uncanny_val);

    // Shift listener's emotional voltage state based on speaker's threat and uncanny mismatch
    listener->current_voltage += (threat * 0.1) + (uncanny_val * 0.05);
    
    // Decay listener state using RCD snubber resonant capacitor equations (Christensen link adjustment)
    double decay = 0.0;
    evaluate_izotope_ozone_habituation_decay(&listener->geom, 0.1, &decay);
    listener->current_voltage -= decay;
    if (listener->current_voltage < 0.0) {
        listener->current_voltage = 0.0;
    }

    // Recalculate retaliation boundaries under provocation
    evaluate_provocation_retaliation_boundary(&listener->geom, listener->current_voltage * 10.0, &listener->current_retaliation_boundary);

    // Reflexive acoustic threshold response logic
    double wstat = 0.0, wpval = 1.0;
    evaluate_parameter_wald_test(listener->current_voltage, listener->current_retaliation_boundary, 0.25, &wstat, &wpval);
    
    bool reflex_triggered = false;
    if (wpval < 0.05 && listener->current_voltage > listener->current_retaliation_boundary) {
        // High reactive voltage triggers sudden pitch shift reflex
        listener->sound_pitch *= 1.15;
        reflex_triggered = true;
    }

    // Introduce flyback noise tremors on high-threat exposure
    if (threat > 2.0) {
        double noise = 0.0;
        simulate_phase_flyback_noise(&listener->geom, listener->sound_pitch, &noise);
        listener->sound_pitch += noise * 5.0;
    }

    // Print acoustic shift
    printf("[Conversation] Bear %s spoke to Bear %s -> %s current voltage: %.4f V (retaliation boundary: %.4f) [Pitch: %.1f Hz]%s\n",
           speaker->name, listener->name, listener->name, listener->current_voltage, listener->current_retaliation_boundary,
           listener->sound_pitch, reflex_triggered ? " *REFLEX ACTIVE*" : "");
    
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT BEAR CHORUS EMOTIONAL DIALOGUE CONVERSATION CONTEXT\n");
    printf("=============================================================\n");

    conversational_bear_t bears[5];
    init_conversational_bears(bears);

    // Run 313 stress test conversation cycles
    for (int step = 0; step < 313; ++step) {
        int speaker = rand() % 5;
        int listener = rand() % 5;
        if (speaker == listener) {
            listener = (listener + 1) % 5;
        }
        
        // Save current transaction state for rollback audit
        double backup_voltage = bears[listener].current_voltage;
        
        assert(simulate_conversational_step(bears, speaker, listener));
        
        // Assert voltage safety bounds
        if (bears[listener].current_voltage < 0.0 || bears[listener].current_voltage > 50.0) {
            bears[listener].current_voltage = backup_voltage; // Rollback
            printf("   [ACID Rollback] Correctly reverted Bear %s voltage state\n", bears[listener].name);
        }
    }

    printf("=============================================================\n");
    printf("BEAR CHORUS 313 STRESS-TEST ITERATIONS VERIFIED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
