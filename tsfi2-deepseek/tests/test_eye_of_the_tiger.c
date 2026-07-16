#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "../../firmware/tsfi_lib/tsfi_alsa_synth.c"

// Frequency lookup helper mirroring MusicNotationParser.yul logic
static double parse_bionika_note(const char *note_str) {
    if (strcmp(note_str, "REST") == 0 || strcmp(note_str, "rest") == 0) {
        return 0.0;
    }
    
    char char0 = note_str[0];
    char char1 = note_str[1];
    
    int note_index = 0;
    switch (char0) {
        case 'C': case 'c': note_index = 0; break;
        case 'D': case 'd': note_index = 2; break;
        case 'E': case 'e': note_index = 4; break;
        case 'F': case 'f': note_index = 5; break;
        case 'G': case 'g': note_index = 7; break;
        case 'A': case 'a': note_index = 9; break;
        case 'B': case 'b': note_index = 11; break;
        default: return 0.0;
    }
    
    int octave_char = char1;
    if (char1 == '#' || char1 == 'b') {
        octave_char = note_str[2];
        if (char1 == '#') {
            note_index += 1;
        } else if (char1 == 'b') {
            if (note_index == 0) note_index = 12;
            note_index -= 1;
        }
    }
    
    int octave = octave_char - '0';
    if (octave < 0 || octave > 8) return 0.0;
    
    // Base frequency calculations matching MusicNotationParser.yul scaled outputs
    double freq = 0.0;
    switch (note_index) {
        case 0:  freq = 261.63; break; // C4
        case 1:  freq = 277.18; break; // C#4 / Db4
        case 2:  freq = 293.66; break; // D4
        case 3:  freq = 311.13; break; // D#4 / Eb4
        case 4:  freq = 329.63; break; // E4
        case 5:  freq = 349.23; break; // F4
        case 6:  freq = 369.99; break; // F#4 / Gb4
        case 7:  freq = 392.00; break; // G4
        case 8:  freq = 415.30; break; // G#4 / Ab4
        case 9:  freq = 440.00; break; // A4
        case 10: freq = 466.16; break; // A#4 / Bb4
        case 11: freq = 493.88; break; // B4
        default: return 0.0;
    }
    
    // Scale octave shift
    if (octave > 4) {
        for (int i = 4; i < octave; i++) freq *= 2.0;
    } else if (octave < 4) {
        for (int i = octave; i < 4; i++) freq /= 2.0;
    }
    
    return freq;
}

int main(void) {
    printf("[BIONIKA] Loading arrangements for Eye of the Tiger...\n");
    fflush(stdout);
    
    // Hardcoded sequence extract mirroring the exact arrangements parsed from eye_of_the_tiger.bio
    const char *intro_lead[] = {
        "C4", "C4", "C4", "C4", "C4", "C4", "C4", "C4",
        "C4", "C4", "Bb3", "C4", "REST", "REST", "REST", "REST",
        "REST", "REST", "Bb3", "C4", "REST", "REST", "REST", "REST",
        "REST", "REST", "Bb3", "C4", "REST", "Ab3", "Bb3", "C4"
    };
    int total_notes = sizeof(intro_lead) / sizeof(intro_lead[0]);
    
    ALSASynth synth;
    if (!alsa_synth_init(&synth, 0.0)) {
        fprintf(stderr, "[BIONIKA] Error initializing ALSA hardware stream.\n");
        return 1;
    }
    
    printf("[BIONIKA] Playing Eye of the Tiger modular arrangements...\n");
    fflush(stdout);
    
    // 109.6 BPM -> 136 ms per eighth note step
    int step_duration_ms = 136;
    
    for (int i = 0; i < total_notes; i++) {
        double freq = parse_bionika_note(intro_lead[i]);
        if (freq == 0.0) {
            printf("       [REST] Step: %d\n", i);
        } else {
            printf("       [PLAY] Note: %s (%6.2f Hz) Step: %d\n", intro_lead[i], freq, i);
        }
        fflush(stdout);
        
        alsa_synth_set_frequency(&synth, freq);
        usleep(step_duration_ms * 1000);
    }
    
    alsa_synth_stop(&synth);
    printf("[BIONIKA] Performance finished successfully.\n");
    fflush(stdout);
    
    return 0;
}
