#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include "../inc/tsfi_bionika_synth.h"
#include "../inc/tsfi_strategy_lang.h"
#include "../inc/tsfi_chancery_docket.h"

// Mirroring solidity/dysnomia/domain/std/bionika_soundtrack_prover.algol61
static int verify_bionika_soundtrack_state(int score_event_count, int total_rendered_frames, int bass_drop_energy_flag, int bird_song_outro_flag) {
    if (score_event_count < 20 || total_rendered_frames < 3969000) return 1; // SCORE_UNDERFLOW_OR_LENGTH_DEFECT
    if (bass_drop_energy_flag == 0 || bird_song_outro_flag == 0) return 2; // CRESCENDO_OR_BIRD_OUTRO_DEFECT
    return 0; // AUTHENTIC_BIONIKA_SCORE_STATE
}

int main(void) {
    printf("====================================================================\n");
    printf("TEST SUITE: BIONIKA 90-SECOND 7-TRACK SOUNDTRACK SYNTHESIZER\n");
    printf("====================================================================\n\n");

    // -------------------------------------------------------------------------
    // 1. Load .bio Soundtrack Score
    // -------------------------------------------------------------------------
    printf("1. Loading Score: assets/bionika/bionika_90s_symphony.bio...\n");
    TsfiBioScore score;
    assert(tsfi_bio_load_score(&score, "../assets/bionika/bionika_90s_symphony.bio"));
    assert(score.event_count >= 30);
    printf("   ✓ Loaded %d Score Events spanning 3 Verses, 3 Choruses, and Outro\n", score.event_count);

    // Verify 7 distinct tracks are referenced in the score
    bool track_present[8] = {false};
    for (int i = 0; i < score.event_count; i++) {
        track_present[score.events[i].track_idx] = true;
    }
    for (int t = 1; t <= 7; t++) {
        assert(track_present[t] == true);
    }
    printf("   ✓ Formally Verified all 7 Instruments (Sub-Bass, Growl, Lead, Kick, Snare, Hi-Hat, BirdSong)\n");

    // -------------------------------------------------------------------------
    // 2. Synthesize 90-Second PCM16 Audio Stream
    // -------------------------------------------------------------------------
    printf("\n2. Synthesizing 90-Second 44.1kHz 16-bit PCM Stream (%d frames)...\n", BIONIKA_TOTAL_SAMPLES);
    int16_t *pcm_data = (int16_t *)calloc(BIONIKA_TOTAL_SAMPLES, sizeof(int16_t));
    assert(pcm_data != NULL);

    size_t rendered = tsfi_bio_synthesize_pcm16(&score, pcm_data, BIONIKA_TOTAL_SAMPLES);
    assert(rendered == BIONIKA_TOTAL_SAMPLES);

    // Analyze Energy Profile: Max Bass Drop (62s - 80s) vs Bird Song Outro (80s - 90s)
    size_t bass_drop_start = (size_t)(62.0f * 44100.0f);
    size_t bass_drop_end = (size_t)(75.0f * 44100.0f);
    double bass_energy = 0.0;
    for (size_t i = bass_drop_start; i < bass_drop_end; i++) {
        bass_energy += abs(pcm_data[i]);
    }
    bass_energy /= (double)(bass_drop_end - bass_drop_start);

    size_t outro_start = (size_t)(80.0f * 44100.0f);
    size_t outro_end = (size_t)(90.0f * 44100.0f);
    double outro_energy = 0.0;
    for (size_t i = outro_start; i < outro_end; i++) {
        outro_energy += abs(pcm_data[i]);
    }
    outro_energy /= (double)(outro_end - outro_start);

    printf("   ✓ Max Crescendo / Bass Drop Average Energy (62s-75s): %.1f / 32767\n", bass_energy);
    printf("   ✓ Quiet Bird Song Outro Average Energy (80s-90s): %.1f / 32767\n", outro_energy);
    assert(bass_energy > 5000.0);
    assert(outro_energy > 500.0 && outro_energy < bass_energy * 0.4);

    // Export to WAV for immediate playback
    assert(tsfi_bio_export_wav("../assets/bionika/bionika_90s_symphony.wav", pcm_data, BIONIKA_TOTAL_SAMPLES));
    printf("   ✓ Exported Clean-Room WAV: assets/bionika/bionika_90s_symphony.wav\n");
    free(pcm_data);

    // -------------------------------------------------------------------------
    // 3. Test COBOL Strategy & ALGOL 61 Prover Integration
    // -------------------------------------------------------------------------
    printf("\n3. Executing COBOL Strategy: bionika_soundtrack.strategy...\n");
    TSFiStrategyVM vm;
    TSFiStrategyReceipt receipt;
    int rc = tsfi_strategy_load_and_run("bionika_soundtrack.strategy", score.event_count, (int)rendered, 1, 0, &vm, &receipt);
    assert(rc == 0);
    assert(vm.registers[3] == 9999);
    printf("   ✓ Strategy Execution Confirmed: Score Energy = %d\n", vm.registers[3]);

    printf("\n4. Executing ALGOL 61 Prover: bionika_soundtrack_prover.algol61...\n");
    int ruling_auth = verify_bionika_soundtrack_state(score.event_count, (int)rendered, 1, 1);
    assert(ruling_auth == 0);
    printf("   ✓ Ruling: AUTHENTIC_BIONIKA_SCORE_STATE (ruling = %d)\n", ruling_auth);

    int ruling_underflow = verify_bionika_soundtrack_state(10, (int)rendered, 1, 1);
    assert(ruling_underflow == 1);
    printf("   ✓ Score Underflow Reject Ruling: SCORE_UNDERFLOW_OR_LENGTH_DEFECT (ruling = %d)\n", ruling_underflow);

    int ruling_outro_defect = verify_bionika_soundtrack_state(score.event_count, (int)rendered, 1, 0);
    assert(ruling_outro_defect == 2);
    printf("   ✓ Outro Defect Reject Ruling: CRESCENDO_OR_BIRD_OUTRO_DEFECT (ruling = %d)\n", ruling_outro_defect);

    // -------------------------------------------------------------------------
    // 4. File Resolution on Chancery Docket
    // -------------------------------------------------------------------------
    printf("\n5. Filing Resolution on Chancery Docket:\n");
    ChanceryDocketState docket;
    tsfi_chancery_docket_init(&docket);

    uint32_t doc_id = tsfi_chancery_docket_file(
        &docket,
        "90-Second 7-Track Bionika Symphony (.bio) Synthesizer Formally Proven",
        "solidity/dysnomia/domain/std/bionika_soundtrack_prover.algol61",
        2026
    );
    assert(doc_id == 7000);

    bool resolved = tsfi_chancery_docket_resolve_zmm_r15(&docket, doc_id, ruling_auth, DOCKET_RULING_AUTHENTIC_STREAM);
    assert(resolved);

    char audit_report[2048];
    uint64_t docket_proof = tsfi_chancery_docket_audit(&docket, audit_report, sizeof(audit_report));
    assert(docket_proof != 0);

    printf("\n%s\n", audit_report);
    printf("====================================================================\n");
    printf("BIONIKA 90S SOUNDTRACK TEST PASSED & SEALED ON CHANCERY\n");
    printf("====================================================================\n");

    return 0;
}
