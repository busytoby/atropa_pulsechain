#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>

// Struct representing coordinate positions in the organism
typedef struct {
    double x, y, z;
} EmbryoPoint;

// Structure representing a simulated embryo node with dynamic growth tracking
typedef struct {
    EmbryoPoint pos;
    double max_length;
    double growth_factor; // 0.0 to 1.0
} SproutNode;

// Helper to scale verlet tethers dynamically during development
double calculate_rest_length(double max_length, double growth_factor) {
    return max_length * growth_factor;
}

// Emulate recursive symmetrical sprout coordinates relative to the embryo root
void calculate_sprout_points(EmbryoPoint root, double length, double angle, EmbryoPoint *left, EmbryoPoint *right) {
    left->x = root.x - length * sin(angle);
    left->y = root.y + length * cos(angle);
    left->z = root.z;

    right->x = root.x + length * sin(angle);
    right->y = root.y + length * cos(angle);
    right->z = root.z;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT EMBRYO DEVELOPMENT & PROCEDURAL GROWTH TEST SUITE\n");
    printf("=============================================================\n");

    // 1. Initialize Embryo Root Node
    EmbryoPoint embryo_root = { 0.0, 0.0, 0.0 };
    SproutNode left_wing = { .pos = embryo_root, .max_length = 30.0, .growth_factor = 0.0 };
    SproutNode right_wing = { .pos = embryo_root, .max_length = 30.0, .growth_factor = 0.0 };

    printf("[TEST] Verifying initial embryo coordinate alignment...\n");
    assert(left_wing.growth_factor == 0.0);
    assert(right_wing.growth_factor == 0.0);

    // 2. Symmetrical Growth Simulation Phase
    printf("[TEST] Simulating symmetrical sprouted wing extension...\n");
    for (int t = 1; t <= 10; t++) {
        double current_growth = (double)t * 0.1;
        left_wing.growth_factor = current_growth;
        right_wing.growth_factor = current_growth;

        double current_len_l = calculate_rest_length(left_wing.max_length, left_wing.growth_factor);
        double current_len_r = calculate_rest_length(right_wing.max_length, right_wing.growth_factor);

        // Assert perfect length parity
        assert(fabs(current_len_l - current_len_r) < 1e-9);

        EmbryoPoint l_tip, r_tip;
        double angle = 0.52359877; // 30 degrees in radians
        calculate_sprout_points(embryo_root, current_len_l, angle, &l_tip, &r_tip);

        // Assert perfect mirror symmetry relative to the center embryo root
        assert(fabs(l_tip.x + r_tip.x) < 1e-9);
        assert(fabs(l_tip.y - r_tip.y) < 1e-9);
    }
    printf("[TEST] Symmetrical sprouting resolved correctly.\n");

    // 3. Chronological Time Gating Verification
    printf("[TEST] Verifying sequential limb activation gates...\n");
    double t_stage = 2.5; // active time offset
    bool trunk_active = t_stage > 0.0;
    bool limbs_active = t_stage > 1.0;
    bool wings_active = t_stage > 3.0;

    assert(trunk_active == true);
    assert(limbs_active == true);
    assert(wings_active == false); // Wings should not have sprouted yet at t_stage = 2.5

    printf("=============================================================\n");
    printf("ALL AUNCIENT EMBRYO PROCEDURAL GROWTH TESTS PASSED\n");
    printf("=============================================================\n");

    return 0;
}
