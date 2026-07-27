#include "../src/auncient_ballet_animator.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT BALLET ANIMATOR TEST SUITE\n");
    printf("=============================================================\n");

    AuncientBalletBear bear = {0};

    // 1. Verify bear registration and deterministic SSA bridge mapping (Rule 16)
    printf("[TEST] Registering newborn Teddy Bear 'Teddy_Giselle'...\n");
    bool registered = auncient_ballet_register_bear("Teddy_Giselle", "BLOCK_SEED_9999", &bear);
    assert(registered == true);
    assert(bear.verlet_charge_decay[0] == 1.0f);
    printf("   ✓ Bear successfully registered with SSN: %s\n", bear.ssn);

    // 2. Step pose and verify Verlet FET discharge cycles (Rule 10)
    printf("[TEST] Stepping ballet pose to t=2.0s...\n");
    auncient_ballet_step_pose(&bear, 2.0f);
    assert(bear.joint_angle_hip != 0.0f);
    
    // Verlet potential must have decayed
    assert(bear.verlet_charge_decay[0] < 1.0f);
    printf("   ✓ Verlet FET discharge potential decayed to: %.4f\n", bear.verlet_charge_decay[0]);

    printf("=============================================================\n");
    printf("ALL AUNCIENT BALLET TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
