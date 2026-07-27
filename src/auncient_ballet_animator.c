#include "auncient_ballet_animator.h"
#include "auncient_timeline_autodin.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

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
