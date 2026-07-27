#ifndef AUNCIENT_BALLET_ANIMATOR_H
#define AUNCIENT_BALLET_ANIMATOR_H

#include "auncient_stl_loader.h"
#include <stdbool.h>

// Struct representing an active ballet participant (Newborn Teddy Bear)
typedef struct {
    char bear_name[32];
    char ssn[12];
    float joint_angle_hip;
    float joint_angle_shoulder;
    float verlet_charge_decay[16]; // Rule 10 physics: Verlet charge decay arrays
} AuncientBalletBear;

// Initializes and registers a ballet teddy bear participant using its DNA seed
bool auncient_ballet_register_bear(const char *name, const char *dna_seed_block, AuncientBalletBear *bear_out);

// Animates the bear's joints and Verlet FET potentials over a time step t
void auncient_ballet_step_pose(AuncientBalletBear *bear, float t);

#endif // AUNCIENT_BALLET_ANIMATOR_H
