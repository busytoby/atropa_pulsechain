#ifndef AUNCIENT_RENDERMAN_RADIOSITY_FORM_FACTORS_THEOREMS_2116_2120_H
#define AUNCIENT_RENDERMAN_RADIOSITY_FORM_FACTORS_THEOREMS_2116_2120_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RADIOSITY_PATCH_ELEMENTS 32
#define HEMICUBE_RESOLUTION 16

typedef struct {
    double emission;
    double reflectivity;
    double unshot_radiosity;
    double total_radiosity;
    double area;
    double centroid[3];
    double normal[3];
} RadiosityElement;

typedef struct {
    RadiosityElement elements[RADIOSITY_PATCH_ELEMENTS];
    double form_factors[RADIOSITY_PATCH_ELEMENTS][RADIOSITY_PATCH_ELEMENTS];
    uint32_t total_hemicube_projections;
    uint32_t cdc6600_gauss_seidel_iterations;
    bool is_energy_conserved;
    bool is_radiosity_converged;
} RendermanRadiosityContext;

typedef struct {
    float in_silicon_radiosity_fidelity;
    float radiosity_strategy_datbin_merkle_ratio;
    float radiosity_solve_latency_ns;
    uint64_t verified_radiosity_saat_clearances;

    bool radiosity_pipeline_verified;
    bool radiosity_strategy_merkle_verified;
    bool radiosity_submicro_latency_verified;
    bool radiosity_lossless_saat_verified;
    bool sovereign_2120_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RendermanRadiosityBeyond2115State;

int cpm_tomie_renderman_radiosity_init(RendermanRadiosityContext *ctx);
int cpm_tomie_renderman_radiosity_solve(RendermanRadiosityContext *ctx, uint32_t iterations);
void auncient_renderman_radiosity_init(RendermanRadiosityBeyond2115State *state);
bool auncient_renderman_radiosity_verify_theorems_2116_2120(RendermanRadiosityBeyond2115State *state);
uint32_t auncient_renderman_radiosity_compute_rule18(const RendermanRadiosityBeyond2115State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_RADIOSITY_FORM_FACTORS_THEOREMS_2116_2120_H */
