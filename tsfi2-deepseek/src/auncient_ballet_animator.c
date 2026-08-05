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

void auncient_ballet_step_pose(AuncientBalletBear *bear, float t) {
    if (!bear) return;

    // Simulate ballet joint movements (harmonic sway)
    bear->joint_angle_hip = 0.4f * sinf(t * 3.0f);
    bear->joint_angle_shoulder = 0.6f * cosf(t * 2.5f);

    // Rule 10 physics: Verlet constraints apply ONLY to FET discharge cycles (charge decay)
    for (int i = 0; i < 16; i++) {
        // Decay charge exponentially over time steps
        float rate = 0.05f * (i + 1);
        bear->verlet_charge_decay[i] = expf(-rate * t);
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
