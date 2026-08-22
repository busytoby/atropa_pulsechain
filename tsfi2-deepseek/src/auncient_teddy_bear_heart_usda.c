#include "auncient_teddy_bear_heart_usda.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void auncient_teddy_bear_heart_init(TeddyBearHeartUsdaAsset *asset, uint32_t dna_seed) {
    if (!asset) return;
    memset(asset, 0, sizeof(TeddyBearHeartUsdaAsset));

    /* Rule 16: Participant qualification, DNA-to-SSA mapping, and Hogan account endowment */
    asset->profile.dna_seed = dna_seed;
    asset->profile.hogan_account_saat = TEDDY_BEAR_DEFAULT_ENDOWMENT_SAAT;
    asset->profile.ssa_qualification_verified = true;

    /* Deterministic SSN translation */
    uint32_t area = (dna_seed % 899) + 100;
    uint32_t group = ((dna_seed >> 10) % 89) + 10;
    uint32_t serial = ((dna_seed >> 17) % 8999) + 1000;
    snprintf(asset->profile.ssn_identifier, sizeof(asset->profile.ssn_identifier),
             "%03u-%02u-%04u", area, group, serial);

    /* Initial Resting Heart State */
    asset->heart.heart_rate_bpm = 72.0f;
    asset->heart.metabolic_heat_w = 45.0f; /* 45 W metabolic output */
    asset->heart.respiratory_rate_bpm = 16.0f;
    asset->heart.thermal_comfort_pmv = 0.0f; /* Neutral baseline */
    asset->heart.clay_plasticity_response = 0.85f;
    asset->heart.leed_compliance_score = 1.0f;

    asset->room_temperature_c = 22.0f;
    asset->relative_humidity_pct = 50.0f;
    asset->co2_ppm = 400.0f;
    asset->hvac_air_velocity_m_s = 0.15f;
}

bool auncient_teddy_bear_heart_update_environment(TeddyBearHeartUsdaAsset *asset, float temp_c, float rh_pct, float co2_ppm, float v_air) {
    if (!asset) return false;
    asset->room_temperature_c = temp_c;
    asset->relative_humidity_pct = rh_pct;
    asset->co2_ppm = co2_ppm;
    asset->hvac_air_velocity_m_s = v_air;

    /* Dynamic physiological response */
    float temp_delta = temp_c - 22.0f;
    float co2_excess = (co2_ppm > 600.0f) ? (co2_ppm - 600.0f) : 0.0f;

    /* Heart rate accelerates with elevated CO2 or thermal stress */
    asset->heart.heart_rate_bpm = 72.0f + 2.5f * temp_delta + 0.02f * co2_excess;
    if (asset->heart.heart_rate_bpm < 50.0f) asset->heart.heart_rate_bpm = 50.0f;
    if (asset->heart.heart_rate_bpm > 140.0f) asset->heart.heart_rate_bpm = 140.0f;

    /* Metabolic heat modulation */
    asset->heart.metabolic_heat_w = 45.0f + 0.8f * (asset->heart.heart_rate_bpm - 72.0f);

    /* Clayscape viscoelastic response modulation */
    asset->heart.clay_plasticity_response = 0.85f + 0.003f * (rh_pct - 50.0f) + 0.005f * temp_delta;
    if (asset->heart.clay_plasticity_response < 0.60f) asset->heart.clay_plasticity_response = 0.60f;
    if (asset->heart.clay_plasticity_response > 1.00f) asset->heart.clay_plasticity_response = 1.00f;

    return auncient_teddy_bear_heart_evaluate_leed_response(asset);
}

bool auncient_teddy_bear_heart_evaluate_leed_response(TeddyBearHeartUsdaAsset *asset) {
    if (!asset) return false;

    /* Fanger PMV calculation approximation */
    float operative_temp = asset->room_temperature_c;
    float pmv = (operative_temp - 22.0f) * 0.35f + (asset->relative_humidity_pct - 50.0f) * 0.01f - (asset->hvac_air_velocity_m_s - 0.15f) * 1.5f;
    asset->heart.thermal_comfort_pmv = pmv;

    /* LEED Platinum compliance: PMV in [-0.5, +0.5] and CO2 <= 800 ppm */
    float pmv_penalty = fabsf(pmv) > 0.5f ? (fabsf(pmv) - 0.5f) * 0.5f : 0.0f;
    float co2_penalty = (asset->co2_ppm > 800.0f) ? (asset->co2_ppm - 800.0f) * 0.001f : 0.0f;

    float score = 1.0f - (pmv_penalty + co2_penalty);
    if (score < 0.0f) score = 0.0f;
    if (score > 1.0f) score = 1.0f;
    asset->heart.leed_compliance_score = score;

    return (score >= 0.70f);
}

uint32_t auncient_teddy_bear_heart_compute_rule18_checksum(const TeddyBearHeartUsdaAsset *asset) {
    if (!asset) return 0;
    const uint8_t *data = (const uint8_t *)asset;
    size_t len = sizeof(TeddyBearHeartUsdaAsset);

    uint64_t p0 = 1;
    uint64_t p1 = (len > 0) ? (data[0] + 7) : 1;
    uint64_t pn = p1;

    for (size_t i = 1; i < len; ++i) {
        uint64_t alpha = (i * 17ULL) % 256ULL;
        uint64_t beta = (i * 31ULL) % 256ULL;
        pn = ((data[i] + alpha) * p1 - beta * p0) % 65535ULL;
        p0 = p1;
        p1 = pn;
    }
    return (uint32_t)pn;
}
