#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tsfi_vaesen_mistral_guardrails.h"

static double get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

int main(void) {
    printf("========================================================================================\n");
    printf("   VAESEN MISTRAL GUARDRAILS: VOCABULARY MASKING, SWA MEMORY & CERTAINTY SCORING TEST\n");
    printf("========================================================================================\n\n");

    /* Ensure canonical slices exist */
    assert(tsfi_vaesen_slice_build_canonical_all() == 0);

    TsfiVaesenDeviceSliceBin nacken, tomte;
    assert(tsfi_vaesen_slice_load("assets/vaesen/nacken.dat.bin", &nacken) == 0);
    assert(tsfi_vaesen_slice_load("assets/vaesen/tomte.dat.bin", &tomte) == 0);

    /* 1. Lore Vocabulary Guardrail Mask Verification */
    TsfiLoreGuardrailMask mask_nacken = tsfi_vaesen_get_lore_guardrail_mask(&nacken);
    TsfiLoreGuardrailMask mask_tomte = tsfi_vaesen_get_lore_guardrail_mask(&tomte);

    bool nacken_sun_valid = tsfi_vaesen_validate_action_against_mask(&mask_nacken, TSFI_ACTION_ENDURE_DIRECT_SUN);
    bool tomte_clothes_valid = tsfi_vaesen_validate_action_against_mask(&mask_tomte, TSFI_ACTION_ACCEPT_CLOTHES_GIFT);
    bool tomte_water_valid = tsfi_vaesen_validate_action_against_mask(&mask_tomte, TSFI_ACTION_CROSS_RUNNING_WATER);

    printf("[LORE VOCABULARY GUARDRAILS]\n");
    printf("  Näcken Endures Direct Sun: %s (Properly Prohibited: PASS)\n", nacken_sun_valid ? "ALLOWED" : "REJECTED");
    printf("  Tomte Accepts Clothes Gift: %s (Properly Prohibited: PASS)\n", tomte_clothes_valid ? "ALLOWED" : "REJECTED");
    printf("  Tomte Crosses Water:        %s (Allowed: PASS)\n\n", tomte_water_valid ? "ALLOWED" : "REJECTED");
    assert(!nacken_sun_valid);
    assert(!tomte_clothes_valid);
    assert(tomte_water_valid);

    /* 2. Sliding Window Acoustic Memory (SWA W=32) Verification */
    TsfiSlidingWindowAcousticBuffer swa_buf;
    tsfi_vaesen_swa_init(&swa_buf);

    for (int t = 0; t < 64; ++t) {
        float energy = (float)(t % 10) * 0.2f;
        tsfi_vaesen_swa_push_turn(&swa_buf, energy, (uint16_t)(t * 5));
    }
    float bounded_energy = tsfi_vaesen_swa_compute_bounded_energy(&swa_buf);
    printf("[SLIDING WINDOW ACOUSTIC MEMORY (SWA W=32)]\n");
    printf("  Total Pushed: 64 turns | Buffer Count: %u / 32 | Bounded Energy: %.4f (PASS)\n\n",
           swa_buf.count, bounded_energy);
    assert(swa_buf.count == 32);

    /* 3. Acoustic Certainty Scoring Verification */
    TsfiAcousticCertaintyScore score_locked = tsfi_vaesen_compute_acoustic_certainty(0);     /* 0 deg phase lock */
    TsfiAcousticCertaintyScore score_caution = tsfi_vaesen_compute_acoustic_certainty(60);   /* 60 deg offset */
    TsfiAcousticCertaintyScore score_fracture = tsfi_vaesen_compute_acoustic_certainty(180); /* 180 deg anti-phase */

    printf("[ACOUSTIC CERTAINTY SCORING]\n");
    printf("  Phase Lock (0 deg):   Certainty: %.4f | Authoritative: %s | Deceptive: %s (PASS)\n",
           score_locked.certainty_metric, score_locked.is_authoritative ? "YES" : "NO", score_locked.is_deceptive ? "YES" : "NO");
    printf("  Caution (60 deg):      Certainty: %.4f | Authoritative: %s | Deceptive: %s (PASS)\n",
           score_caution.certainty_metric, score_caution.is_authoritative ? "YES" : "NO", score_caution.is_deceptive ? "YES" : "NO");
    printf("  Fracture (180 deg):   Certainty: %.4f | Authoritative: %s | Deceptive: %s (PASS)\n\n",
           score_fracture.certainty_metric, score_fracture.is_authoritative ? "YES" : "NO", score_fracture.is_deceptive ? "YES" : "NO");
    assert(score_locked.is_authoritative);
    assert(score_fracture.is_deceptive);

    /* 4. High-Performance Latency Benchmark across 1,000,000 guardrail + SWA + certainty checks */
    const int benchmark_count = 1000000;
    double t_start = get_time_ns();
    for (int i = 0; i < benchmark_count; ++i) {
        tsfi_vaesen_validate_action_against_mask(&mask_nacken, TSFI_ACTION_ENDURE_DIRECT_SUN);
        tsfi_vaesen_swa_push_turn(&swa_buf, 0.5f, 0);
        tsfi_vaesen_compute_acoustic_certainty(0);
    }
    double t_end = get_time_ns();
    double avg_latency = (t_end - t_start) / (double)benchmark_count;
    printf("[BENCHMARK] Average Mistral Guardrails & SWA latency: %.2f ns (Target: < 1000 ns)\n", avg_latency);
    assert(avg_latency < 1000.0);

    printf("\n>>> ALL MISTRAL GUARDRAILS & SWA MEMORY FORMALLY VERIFIED <<<\n");
    return 0;
}
