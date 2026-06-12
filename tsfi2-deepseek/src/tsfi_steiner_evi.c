#include "tsfi_steiner_evi.h"
#include <math.h>

void tsfi_steiner_evi_translate(const SteinerEVIState *evi, float *out_frequency, float *out_amplitude) {
    if (!evi || !out_frequency || !out_amplitude) return;

    // Semitone calculations for Electronic Valve Instrument (EVI):
    // Valves are additive:
    // Valve 2 = -1 semitone
    // Valve 1 = -2 semitones
    // Valve 3 = -3 semitones
    int semitones = 0;
    if (evi->valve2) semitones -= 1;
    if (evi->valve1) semitones -= 2;
    if (evi->valve3) semitones -= 3;

    // Frequency = base_pitch * 2^(octave + semitones/12)
    float exponent = (float)evi->octave + ((float)semitones / 12.0f);
    *out_frequency = evi->base_pitch * powf(2.0f, exponent);

    // Breath pressure maps directly to amplitude (VCA)
    *out_amplitude = (evi->breath_pressure < 0.0f) ? 0.0f : ((evi->breath_pressure > 1.0f) ? 1.0f : evi->breath_pressure);
}

void tsfi_steiner_evi_untranslate(float frequency, float amplitude, SteinerEVIState *out_evi) {
    if (!out_evi) return;

    out_evi->breath_pressure = amplitude;
    out_evi->base_pitch = 261.63f; // Default C4 reference pitch

    if (frequency <= 0.0f) {
        out_evi->octave = 0;
        out_evi->valve1 = 0;
        out_evi->valve2 = 0;
        out_evi->valve3 = 0;
        return;
    }

    // calculate pitch factor: log2(frequency / base_pitch)
    float pitch_factor = log2f(frequency / out_evi->base_pitch);
    
    // Nearest octave
    int octave = (int)roundf(pitch_factor);
    float remainder_semitones = (pitch_factor - (float)octave) * 12.0f;
    int semitones = (int)roundf(remainder_semitones);

    // EVI valve fingering yields offsets in [0, -6] semitones range
    while (semitones > 0) {
        octave++;
        semitones -= 12;
    }
    while (semitones < -6) {
        octave--;
        semitones += 12;
    }

    out_evi->octave = octave;
    out_evi->valve1 = 0;
    out_evi->valve2 = 0;
    out_evi->valve3 = 0;

    switch (semitones) {
        case 0:
            break;
        case -1:
            out_evi->valve2 = 1;
            break;
        case -2:
            out_evi->valve1 = 1;
            break;
        case -3:
            out_evi->valve3 = 1; // Default -3 semitones to Valve 3
            break;
        case -4:
            out_evi->valve2 = 1;
            out_evi->valve3 = 1;
            break;
        case -5:
            out_evi->valve1 = 1;
            out_evi->valve3 = 1;
            break;
        case -6:
            out_evi->valve1 = 1;
            out_evi->valve2 = 1;
            out_evi->valve3 = 1;
            break;
        default:
            break;
    }
}
