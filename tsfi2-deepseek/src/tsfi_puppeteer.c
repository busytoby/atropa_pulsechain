#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include "tsfi_puppetry.h"

/**
 * @brief TSFi Puppeteer CLI
 * Allows manual or automated control of the apparitions.
 */

void usage(const char *prog) {
    fprintf(stderr, "Usage: %s <id> <type: teddy|alligator|crow|poppy> <action: fly|sit|speak|sick> [param]\n", prog);
}

int main(int argc, char **argv) {
    if (argc < 4) {
        usage(argv[0]);
        return 1;
    }

    int id = atoi(argv[1]);
    const char *type_str = argv[2];
    const char *action = argv[3];
    float param = (argc > 4) ? atof(argv[4]) : 0.0f;

    TsfiPuppetState *pstate = tsfi_puppetry_shm_attach();
    if (!pstate) {
        pstate = tsfi_puppetry_shm_create();
        if (pstate) {
            memset(pstate, 0, sizeof(TsfiPuppetState));
            pstate->magic = TSFI_PUPPET_MAGIC;
            pstate->version = 1;
        }
    }
    if (!pstate) return 1;

    TsfiPuppet *p = &pstate->puppets[id % 4];
    p->active = 1;
    p->scale = 0.6f;

    if (strcmp(type_str, "teddy") == 0) p->animal_type = PUPPET_TYPE_TEDDY;
    else if (strcmp(type_str, "alligator") == 0) p->animal_type = PUPPET_TYPE_ALLIGATOR;
    else if (strcmp(type_str, "crow") == 0) p->animal_type = PUPPET_TYPE_CROW;
    else if (strcmp(type_str, "poppy") == 0) p->animal_type = PUPPET_TYPE_POPPY;

    // Anchor position based on ID quadrant
    float cx = (id % 2 == 0) ? 64.0f : 192.0f;
    float cy = (id < 2) ? 64.0f : 192.0f;

    p->body = (TsfiJoint){cx, cy, 120};
    p->head = (TsfiJoint){cx, cy - 30, 100};
    p->neck = (TsfiJoint){cx, cy - 15, 110};
    p->l_shoulder = (TsfiJoint){cx - 20, cy, 110};
    p->r_shoulder = (TsfiJoint){cx + 20, cy, 110};
    p->l_hip = (TsfiJoint){cx - 15, cy + 20, 110};
    p->r_hip = (TsfiJoint){cx + 15, cy + 20, 110};

    // Default beak positions for the Crow (closed)
    p->beak_upper = (TsfiJoint){cx + 20, cy - 30, 90};
    p->beak_lower = (TsfiJoint){cx + 20, cy - 28, 90};
    
    // Clear sickness unless commanded
    if (strcmp(action, "sick") != 0) {
        p->sickness = 0.0f;
    }

    if (p->animal_type == PUPPET_TYPE_TEDDY) {
        if (strcmp(action, "sick") == 0) {
            p->sickness = param > 0.0f ? param : 1.0f; // Default to full sickness if no param
            printf("[PUPPET] Teddy %d: SYMPATHY active. Sickness=%.2f.\n", id, p->sickness);
        } else if (strcmp(action, "sit") == 0) {
            printf("[PUPPET] Teddy %d: Sitting.\n", id);
        }
        // Always initialize structural defaults for Teddy if not already set
        if (p->stuffing == 0.0f) {
            p->stuffing = 1.0f;
            p->arm_length = 1.0f;
            p->leg_length = 1.0f;
            p->head_size = 1.0f;
            p->ear_size = 1.0f;
        }
    } else if (p->animal_type == PUPPET_TYPE_CROW) {
        if (strcmp(action, "fly") == 0) {
            float flap = sinf(param * 10.0f) * 40.0f;
            p->l_hand = (TsfiJoint){cx - 50, cy - 10 - flap, 90};
            p->r_hand = (TsfiJoint){cx + 50, cy - 10 - flap, 90};
            p->l_foot = (TsfiJoint){cx - 10, cy + 30, 120};
            p->r_foot = (TsfiJoint){cx + 10, cy + 30, 120};
            printf("[PUPPET] Crow %d: Flying (Flap: %.2f)\n", id, flap);
        } else if (strcmp(action, "speak") == 0) {
            // Perch posture
            p->l_hand = (TsfiJoint){cx - 20, cy + 10, 110};
            p->r_hand = (TsfiJoint){cx + 20, cy + 10, 110};
            p->l_foot = (TsfiJoint){cx - 10, cy + 30, 120};
            p->r_foot = (TsfiJoint){cx + 10, cy + 30, 120};
            
            // Beak articulation driven by param
            float jaw_drop = fabsf(sinf(param * 15.0f)) * 15.0f; 
            p->beak_lower.y += jaw_drop;
            
            printf("[PUPPET] Crow %d: Speaking (Jaw Drop: %.2f)\n", id, jaw_drop);
        } else if (strcmp(action, "sick") == 0) {
            p->sickness = param;
            
            // Sick perch: slouched, asymmetric
            p->head.y += 10; // Drooped head
            p->l_hand = (TsfiJoint){cx - 15, cy + 20, 110}; // Weak left wing
            p->r_hand = (TsfiJoint){cx + 25, cy + 5, 110};  // Spastic right wing
            p->l_foot = (TsfiJoint){cx - 10, cy + 30, 120};
            p->r_foot = (TsfiJoint){cx + 10, cy + 30, 120};
            
            printf("[PUPPET] Crow %d: SYMPATHY active. Sickness=%.2f. Structural fracture applied.\n", id, p->sickness);
        } else {
            // Perch
            p->l_hand = (TsfiJoint){cx - 20, cy + 10, 110};
            p->r_hand = (TsfiJoint){cx + 20, cy + 10, 110};
            p->l_foot = (TsfiJoint){cx - 10, cy + 30, 120};
            p->r_foot = (TsfiJoint){cx + 10, cy + 30, 120};
            printf("[PUPPET] Crow %d: Perching.\n", id);
        }
    }

    tsfi_puppetry_shm_close(pstate);
    return 0;
}
