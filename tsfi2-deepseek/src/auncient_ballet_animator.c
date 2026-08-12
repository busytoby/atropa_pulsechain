#include "auncient_ballet_animator.h"
#include "auncient_timeline_autodin.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

extern void auncient_bridge_entity_to_ssa(const char *entity_name, char *ssn_out, char *site_out, int max_len);

bool auncient_ballet_register_bear(const char *name, const char *dna_seed_block, AuncientBalletBear *bear_out) {
    if (!name || !dna_seed_block || !bear_out) {
        return false;
    }

    strncpy(bear_out->bear_name, name, sizeof(bear_out->bear_name) - 1);
    bear_out->joint_angle_hip = 0.0f;
    bear_out->joint_angle_shoulder = 0.0f;

    // Rule 10: Initialize simulated FET discharge cycle charge levels (all active)
    for (int i = 0; i < 16; i++) {
        bear_out->verlet_charge_decay[i] = 1.0f;
    }

    // Rule 16: Newborn teddy bear maps seed block / DNA deterministically to SSN via bridge
    char site[32] = {0};
    auncient_bridge_entity_to_ssa(name, bear_out->ssn, site, sizeof(site));

    // Confirm that the SSN matches active participant requirements
    if (strlen(bear_out->ssn) == 0) {
        printf("[BALLET REGISTRATION REJECT] Bear '%s' has no verified SSA SSN.\n", name);
        return false;
    }

    printf("[BALLET REGISTRATION SUCCESS] Bear '%s' registered under SSN: %s (Issuance Site: %s)\n", 
           name, bear_out->ssn, site);
    return true;
}

/* Standard Bonnet Three-Term Recurrence Solver for Orderly Legendre Polynomials */
static float auncient_legendre_evaluate(int n, float x) {
    if (x > 1.0f) x = 1.0f;
    if (x < -1.0f) x = -1.0f;

    if (n == 0) return 1.0f;
    if (n == 1) return x;

    float p_prev2 = 1.0f; // P_0(x)
    float p_prev1 = x;    // P_1(x)
    float p_curr = x;

    for (int k = 1; k < n; k++) {
        p_curr = ((2.0f * k + 1.0f) * x * p_prev1 - (float)k * p_prev2) / (float)(k + 1);
        p_prev2 = p_prev1;
        p_prev1 = p_curr;
    }

    return p_curr;
}

void auncient_ballet_step_pose(AuncientBalletBear *bear, float t) {
    if (!bear) return;

    // Bounded phase input x(t) = cos(w * t)
    float x = cosf(t * 2.0f);

    // Orderly Legendre basis joint animations
    bear->joint_angle_hip = 0.4f * auncient_legendre_evaluate(1, x);      // P_1(x) linear sway
    bear->joint_angle_shoulder = 0.6f * auncient_legendre_evaluate(2, x); // P_2(x) quadratic flex

    // Rule 10 physics: Verlet FET discharge cycle charge decay modulated by P_3(x)
    float p3 = auncient_legendre_evaluate(3, x);
    for (int i = 0; i < 16; i++) {
        float rate = 0.05f * (i + 1);
        bear->verlet_charge_decay[i] = expf(-rate * t) * (0.8f + 0.2f * p3);
    }
}

bool auncient_ballet_export_pose_usda(const AuncientStlMesh *base_stl, const AuncientBalletBear *bear, const char *usda_filepath) {
    if (!base_stl || !bear || !usda_filepath) {
        return false;
    }

    AuncientStlMesh deformed;
    deformed.facet_count = base_stl->facet_count;
    deformed.facets = (AuncientStlFacet *)malloc(sizeof(AuncientStlFacet) * deformed.facet_count);
    if (!deformed.facets) {
        return false;
    }
    memcpy(deformed.facets, base_stl->facets, sizeof(AuncientStlFacet) * deformed.facet_count);

    // Apply joint rotation and Verlet FET stretch transitions
    float theta = bear->joint_angle_hip;
    float cos_t = cosf(theta);
    float sin_t = sinf(theta);

    // Rule 10 physics: Apply Verlet charge decay multiplier to stretch transition regions
    float stretch = bear->verlet_charge_decay[0];

    for (uint32_t i = 0; i < deformed.facet_count; i++) {
        for (int v = 0; v < 3; v++) {
            float x = deformed.facets[i].vertices[v][0];
            float z = deformed.facets[i].vertices[v][2];

            // 1. Joint Rotation (XZ plane transformation)
            deformed.facets[i].vertices[v][0] = x * cos_t - z * sin_t;
            deformed.facets[i].vertices[v][2] = x * sin_t + z * cos_t;

            // 2. Verlet soft-body FET stretch transition boundary (stretch coordinates near joint)
            if (deformed.facets[i].vertices[v][2] < 0.0f) {
                deformed.facets[i].vertices[v][2] *= stretch;
            }
        }
    }

    extern bool auncient_bridge_stl_to_usda(const AuncientStlMesh *stl_mesh, const char *usda_filepath);
    bool ok = auncient_bridge_stl_to_usda(&deformed, usda_filepath);
    free(deformed.facets);
    return ok;
}
