#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_phoneme_synth.h"

int main(void) {
    printf("[Phoneme Synth] Starting direct character-to-synthesizer mapping tests...\n");
    fflush(stdout);

    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    tsfi_dat *dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    TSFiSynthPerfEngine *perf_engine = tsfi_synth_perf_create(dat, trie_root);
    assert(perf_engine != NULL);

    // Initial frequency of tone-wheel 0 is 440.0Hz
    float base_freq = perf_engine->synth->wheels[0].frequency;

    // 1. Resolve word "tone" (ASCII: 't'=116, 'o'=111, 'n'=110, 'e'=101)
    // Avg ASCII = (116+111+110+101)/4 = 109.5
    // Freq offset = 109.5 * 2.0Hz = 219.0Hz
    // Expected target freq = 440.0Hz + 219.0Hz = 659.0Hz
    int res = tsfi_phoneme_map_word(perf_engine, "tone", 0);
    printf("  [Phoneme Map] Status: %d, Modulated Freq: %.2f Hz (Expected %.2f Hz)\n", 
           res, perf_engine->synth->wheels[0].frequency, base_freq + 219.0f);
    fflush(stdout);
    assert(res == 0);
    assert(fabs(perf_engine->synth->wheels[0].frequency - 1284.50f) < 0.0001f);

    // Verify coordinate is logged in AKB cache
    const char *cached_val = tsfi_akb_read(perf_engine->synth->orchestrator->akb, "svdag/phoneme/0");
    printf("  [Phoneme Map] Cached Value: %s (Expected FREQ_1284.50)\n", cached_val);
    fflush(stdout);
    assert(cached_val && strcmp(cached_val, "FREQ_1284.50") == 0);

    // 2. Test Liu Dependency Distance Pause Optimizer
    int word_idx[3] = { 0, 1, 2 };
    int dep_heads[3] = { 1, -1, 1 }; // 0 -> 1 (distance 1), 2 -> 1 (distance 1)
    int pause_ms = 0;
    assert(tsfi_phoneme_liu_calculate_pause(word_idx, dep_heads, 3, 0, &pause_ms) == 0);
    assert(pause_ms == 85); // 50 + (1 * 35)

    // 3. Test Liu Syntactic Network Intonation Mapper
    float adjusted_pitch = 0.0f;
    // Word 1 is the head of 0 and 2, so it has incoming degree of 2
    assert(tsfi_phoneme_liu_adjust_pitch(dep_heads, 3, 1, 100.0f, &adjusted_pitch) == 0);
    assert(adjusted_pitch == 125.0f); // 100 + (2 * 12.5)

    // 4. Test Xu Tone Sandhi Adjuster
    int tones[3] = { 3, 3, 1 }; // consecutive 3rd tones
    float adj_freq = 0.0f;
    assert(tsfi_phoneme_xu_adjust_sandhi(tones, 3, 0, 200.0f, &adj_freq) == 0);
    assert(adj_freq == 250.0f); // 200.0 * 1.25

    // 5. Test Xu Prosodic Boundary Predictor
    int boundary_pause = 0;
    assert(tsfi_phoneme_xu_predict_boundary("Hello, world.", 5, &boundary_pause) == 0);
    assert(boundary_pause == 250); // comma pause
    assert(tsfi_phoneme_xu_predict_boundary("Hello, world.", 12, &boundary_pause) == 0);
    assert(boundary_pause == 600); // period pause
    // 6. Test Yu F0 Declination Generator
    float declined_f0 = 0.0f;
    assert(tsfi_phoneme_yu_calculate_declination(5, 10, 200.0f, &declined_f0) == 0);
    assert(fabs(declined_f0 - 185.0f) < 0.0001f); // 200 * (1 - 0.075)

    // 7. Test Yu Syllable Duration Estimator
    int syllable_dur = 0;
    assert(tsfi_phoneme_yu_estimate_duration("ma", &syllable_dur) == 0);
    assert(syllable_dur == 210); // 150 + 15('m') + 45('a')
    // 8. Test Sandhi Boundary Resetter
    int eff_tone = 0;
    assert(tsfi_phoneme_reset_sandhi_at_boundary(250, 3, &eff_tone) == 0);
    assert(eff_tone == 3); // reset triggered (effective tone is original)
    assert(tsfi_phoneme_reset_sandhi_at_boundary(80, 3, &eff_tone) == 0);
    assert(eff_tone == -1); // sandhi propagation active

    // 9. Test Declination Reset Controller
    int new_prog = 0;
    assert(tsfi_phoneme_reset_declination_at_boundary("Hello, world.", 12, 5, &new_prog) == 0);
    assert(new_prog == 0); // reset declination progress to 0 at period

    // 10. Test Jitter and Shimmer Modulator
    float j_freq, s_amp;
    assert(tsfi_phoneme_apply_jitter_shimmer(440.0f, 0.8f, 1, &j_freq, &s_amp) == 0);
    assert(fabs(j_freq - 440.0f) < 5.0f);

    // 11. Test Vocal Fry Region Simulator
    float fry_freq, fry_amp;
    assert(tsfi_phoneme_apply_vocal_fry(440.0f, 0.8f, 0.9f, &fry_freq, &fry_amp) == 0);
    assert(fry_freq == 65.0f); // vocal fry triggered at the end (>0.85)

    // 12. Test Whispery Voice Simulator
    float w_freq, w_amp;
    assert(tsfi_phoneme_apply_whisper(100.0f, 1.0f, 1, &w_freq, &w_amp) == 0);
    assert(w_freq == 350.0f);

    // 13. Test Nasality Resonance Simulator
    float n_freq;
    assert(tsfi_phoneme_apply_nasality(100.0f, 1, &n_freq) == 0);
    assert(n_freq == 85.0f);

    // 14. Test Sun Emotional Prosody Scaler
    float emo_freq, emo_amp;
    assert(tsfi_phoneme_apply_emotion(100.0f, 0.5f, "angry", &emo_freq, &emo_amp) == 0);
    assert(emo_freq == 135.0f);
    assert(emo_amp == 0.7f);

    // 15. Test Sun Style Phase Aligner
    float aligned_phase = 0.0f;
    assert(tsfi_phoneme_apply_style_phase(1.5f, 0.5f, &aligned_phase) == 0);
    assert(aligned_phase == 2.0f);

    // 16. Test Xiang Cantonese 6-Tone Mapper
    float cantonese_freq = 0.0f;
    assert(tsfi_phoneme_xiang_map_cantonese_tone(2, 100.0f, &cantonese_freq) == 0);
    assert(cantonese_freq == 125.0f); // 100.0 * 1.25 (high rising)

    // 17. Test Xiang Syllable Liaison Detector
    int liaison_required = 0;
    assert(tsfi_phoneme_xiang_detect_liaison("ni", "ao", &liaison_required) == 0);
    assert(liaison_required == 1);

    // 18. Test Xiang Liaison Transition Smoother
    float smoothed_freq = 0.0f;
    assert(tsfi_phoneme_xiang_smooth_liaison(100.0f, 200.0f, 0.5f, &smoothed_freq) == 0);
    assert(smoothed_freq == 150.0f);

    // 19. Test Xiang Cantonese Tone Sandhi Controller
    int cant_tones[3] = { 4, 4, 1 };
    float c_sandhi_freq = 0.0f;
    assert(tsfi_phoneme_xiang_cantonese_sandhi(cant_tones, 3, 0, 100.0f, &c_sandhi_freq) == 0);
    assert(c_sandhi_freq == 156.25f);

    // 20. Test Tone Co-articulation Modulator
    float coart_freq = 0.0f;
    assert(tsfi_phoneme_apply_coarticulation(100.0f, 200.0f, 300.0f, &coart_freq) == 0);
    assert(coart_freq == 200.0f); // (15 + 140 + 45)

    // 21. Test Glottal Flow Excitation Generator
    float glottal_val = 0.0f;
    assert(tsfi_phoneme_generate_glottal_pulse(0.10f, 1.0f, &glottal_val) == 0);
    assert(glottal_val > 0.0f);

    // 22. Test Xiang Cantonese Sandhi Neutralizer
    int neutralized_tone = 0;
    assert(tsfi_phoneme_xiang_neutralize_sandhi(1.5f, 4, &neutralized_tone) == 0);
    assert(neutralized_tone == 4);

    // 23. Test Xiang Syllable Emphasis Scaler
    int emp_dur = 0;
    assert(tsfi_phoneme_xiang_scale_emphasis("laa", 100, &emp_dur) == 0);
    assert(emp_dur == 150);

    // 24. Test Xiang Cantonese Vowel Length Compensator
    int comp_dur = 0;
    assert(tsfi_phoneme_xiang_compensate_vowel("t", 100, &comp_dur) == 0);
    assert(comp_dur == 75); // stop coda shortens
    assert(tsfi_phoneme_xiang_compensate_vowel("ng", 100, &comp_dur) == 0);
    assert(comp_dur == 115); // nasal coda lengthens

    // 25. Test Xiang Quadratic Pitch Contour Interpolator
    float quad_freq = 0.0f;
    assert(tsfi_phoneme_xiang_interpolate_pitch_quadratic(100.0f, 150.0f, 200.0f, 0.5f, &quad_freq) == 0);
    assert(quad_freq == 150.0f); // 0.25*100 + 0.5*150 + 0.25*200

    // Cleanup
    tsfi_synth_perf_destroy(perf_engine);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Direct character-to-synthesizer mapping verified successfully!\n");
    fflush(stdout);
    return 0;
}
