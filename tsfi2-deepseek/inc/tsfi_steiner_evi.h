#ifndef TSFI_STEINER_EVI_H
#define TSFI_STEINER_EVI_H

typedef struct {
    float breath_pressure; // 0.0 to 1.0 (controlling VCA and VCF)
    int valve1;            // 0 or 1 (Needle key offset: -2 semitones)
    int valve2;            // 0 or 1 (Middle key offset: -1 semitone)
    int valve3;            // 0 or 1 (Pinky key offset: -3 semitones)
    int octave;            // Octave selector (-3 to +3 octaves)
    float base_pitch;      // Base tuning frequency (e.g., 220.0 Hz)
} SteinerEVIState;

/**
 * @brief Translates physical EVI breath and valve keys to dynamic frequency and amplitude.
 */
void tsfi_steiner_evi_translate(const SteinerEVIState *evi, float *out_frequency, float *out_amplitude);

/**
 * @brief Translates an objective frequency and amplitude back into Steiner EVI state parameters.
 */
void tsfi_steiner_evi_untranslate(float frequency, float amplitude, SteinerEVIState *out_evi);

#endif // TSFI_STEINER_EVI_H
