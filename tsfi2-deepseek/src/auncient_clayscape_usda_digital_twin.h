#ifndef AUNCIENT_CLAYSCAPE_USDA_DIGITAL_TWIN_H
#define AUNCIENT_CLAYSCAPE_USDA_DIGITAL_TWIN_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define USDA_TPA_BASE 0x0100
#define USDA_DEFAULT_HOGAN_ENDOWMENT_SAAT 1000000ULL

typedef struct {
    uint32_t asset_dna_seed;
    char asset_type_name[32];
    char ssn_registry_id[12];
    uint64_t hogan_account_balance_saat;
    bool participant_qualified;
} UsdaAssetProfile;

typedef struct {
    float ambient_lux;
    float artificial_lux;
    float total_task_lux;
    float chiller_cop;
    float dry_bulb_temp_c;
    float relative_humidity_pct;
    bool mold_inhibition_verified;
    bool economizer_active;
    float duct_velocity_m_s;
    bool nc30_acoustic_compliant;
    float leed_sustainability_index; /* 0.0 to 1.0 */
    float clayscape_elastic_modulus_kpa;
} UsdaLeedCertifiedState;

typedef struct {
    UsdaAssetProfile profile;
    UsdaLeedCertifiedState leed_state;
    uint32_t rule18_parity_checksum;
} ClayscapeUsdaDigitalTwin;

void clayscape_usda_init(ClayscapeUsdaDigitalTwin *asset, uint32_t dna_seed, const char *type_name);
bool clayscape_usda_evaluate_leed_theorems(
    ClayscapeUsdaDigitalTwin *asset,
    float daylight_lux,
    float ambient_temp_c,
    float rh_pct,
    float enthalpy_ambient_kj_kg,
    float enthalpy_return_kj_kg,
    float duct_flow_m3_s,
    float duct_area_m2
);
uint32_t clayscape_usda_compute_rule18_checksum(const ClayscapeUsdaDigitalTwin *asset);

#endif /* AUNCIENT_CLAYSCAPE_USDA_DIGITAL_TWIN_H */
