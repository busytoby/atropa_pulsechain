#include "auncient_clayscape_usda_digital_twin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void clayscape_usda_init(ClayscapeUsdaDigitalTwin *asset, uint32_t dna_seed, const char *type_name) {
    if (!asset) return;
    memset(asset, 0, sizeof(ClayscapeUsdaDigitalTwin));

    /* Rule 16: Entity qualification, SSA identifier translation, Hogan 1M Saat endowment */
    asset->profile.asset_dna_seed = dna_seed;
    snprintf(asset->profile.asset_type_name, sizeof(asset->profile.asset_type_name),
             "%s", type_name ? type_name : "ClayscapeAsset");
    asset->profile.hogan_account_balance_saat = USDA_DEFAULT_HOGAN_ENDOWMENT_SAAT;
    asset->profile.participant_qualified = true;

    uint32_t area = (dna_seed % 899) + 100;
    uint32_t group = ((dna_seed >> 10) % 89) + 10;
    uint32_t serial = ((dna_seed >> 17) % 8999) + 1000;
    snprintf(asset->profile.ssn_registry_id, sizeof(asset->profile.ssn_registry_id),
             "%03u-%02u-%04u", area, group, serial);

    /* Baseline Initializations */
    asset->leed_state.ambient_lux = 300.0f;
    asset->leed_state.artificial_lux = 200.0f;
    asset->leed_state.total_task_lux = 500.0f;
    asset->leed_state.chiller_cop = 5.2f;
    asset->leed_state.dry_bulb_temp_c = 22.0f;
    asset->leed_state.relative_humidity_pct = 45.0f;
    asset->leed_state.mold_inhibition_verified = true;
    asset->leed_state.economizer_active = false;
    asset->leed_state.duct_velocity_m_s = 1.8f;
    asset->leed_state.nc30_acoustic_compliant = true;
    asset->leed_state.leed_sustainability_index = 1.0f;
    asset->leed_state.clayscape_elastic_modulus_kpa = 125.0f;
}

bool clayscape_usda_evaluate_leed_theorems(
    ClayscapeUsdaDigitalTwin *asset,
    float daylight_lux,
    float ambient_temp_c,
    float rh_pct,
    float enthalpy_ambient_kj_kg,
    float enthalpy_return_kj_kg,
    float duct_flow_m3_s,
    float duct_area_m2
) {
    if (!asset) return false;

    /* Theorem 21: Daylight Harvesting Control */
    asset->leed_state.ambient_lux = daylight_lux;
    float target_lux = 500.0f;
    if (daylight_lux >= target_lux) {
        asset->leed_state.artificial_lux = 0.0f;
    } else {
        asset->leed_state.artificial_lux = target_lux - daylight_lux;
    }
    asset->leed_state.total_task_lux = asset->leed_state.ambient_lux + asset->leed_state.artificial_lux;

    /* Theorem 22: Chiller Carnot COP Evaluation */
    float t_chws_k = 273.15f + 7.0f; /* 7 C supply water */
    float t_cond_k = 273.15f + ambient_temp_c + 5.0f;
    float carnot_ideal = t_chws_k / (t_cond_k - t_chws_k);
    asset->leed_state.chiller_cop = 0.65f * carnot_ideal;

    /* Theorem 23: Psychrometric Relative Humidity & Mold Inhibition */
    asset->leed_state.dry_bulb_temp_c = ambient_temp_c;
    asset->leed_state.relative_humidity_pct = rh_pct;
    asset->leed_state.mold_inhibition_verified = (rh_pct >= 30.0f && rh_pct <= 60.0f);

    /* Theorem 24: Air-Side Economizer Differential Enthalpy Switchover */
    float deadband_kj_kg = 2.0f;
    asset->leed_state.economizer_active = (enthalpy_ambient_kj_kg < (enthalpy_return_kj_kg - deadband_kj_kg));

    /* Theorem 25: Acoustic NC-30 Duct Velocity Evaluation */
    if (duct_area_m2 > 0.01f) {
        asset->leed_state.duct_velocity_m_s = duct_flow_m3_s / duct_area_m2;
    } else {
        asset->leed_state.duct_velocity_m_s = 2.0f;
    }
    asset->leed_state.nc30_acoustic_compliant = (asset->leed_state.duct_velocity_m_s <= 2.5f);

    /* Clayscape Viscoelastic Elastic Modulus */
    asset->leed_state.clayscape_elastic_modulus_kpa = 120.0f + 0.5f * (rh_pct - 50.0f) + 1.2f * (ambient_temp_c - 22.0f);

    /* Overall LEED Certification Index */
    float score = 0.0f;
    if (asset->leed_state.total_task_lux >= 500.0f) score += 0.20f;
    if (asset->leed_state.chiller_cop >= 4.0f) score += 0.20f;
    if (asset->leed_state.mold_inhibition_verified) score += 0.20f;
    if (asset->leed_state.nc30_acoustic_compliant) score += 0.20f;
    if (asset->profile.participant_qualified) score += 0.20f;

    asset->leed_state.leed_sustainability_index = score;
    asset->rule18_parity_checksum = clayscape_usda_compute_rule18_checksum(asset);

    return (score >= 0.80f && asset->rule18_parity_checksum > 0);
}

uint32_t clayscape_usda_compute_rule18_checksum(const ClayscapeUsdaDigitalTwin *asset) {
    if (!asset) return 0;
    const uint8_t *data = (const uint8_t *)asset;
    size_t len = sizeof(ClayscapeUsdaDigitalTwin);

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
