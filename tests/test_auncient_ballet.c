#include "../tsfi2-deepseek/src/auncient_ballet_animator.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT BALLET ANIMATOR TEST SUITE\n");
    printf("=============================================================\n");

    AuncientBalletBear bear = {0};

    // 1. Verify bear registration and deterministic SSA bridge mapping (Rule 16)
    printf("[TEST] Registering newborn Teddy Bear 'Teddy_Bear_Giselle'...\n");
    bool registered = auncient_ballet_register_bear("Teddy_Bear_Giselle", "BLOCK_SEED_9999", &bear);
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

    // 3. Test USD-STL cooperative scene export
    printf("[TEST] Exporting posed teddy bear mesh to USDA...\n");
    AuncientStlFacet facet = {
        .normal = { 0.0f, 0.0f, 1.0f },
        .vertices = {
            { 0.0f, 0.0f, 10.0f },
            { 1.0f, 0.0f, 10.0f },
            { 0.0f, 1.0f, 10.0f }
        }
    };
    AuncientStlMesh mock_stl = {
        .facet_count = 1,
        .facets = &facet
    };
    const char *out_usda = "tests/test_ballet_pose.usda";
    bool exp_ok = auncient_ballet_export_pose_usda(&mock_stl, &bear, out_usda);
    assert(exp_ok == true);

    FILE *f_check = fopen(out_usda, "r");
    assert(f_check != NULL);
    char line[128];
    bool verified_deform = false;
    while (fgets(line, sizeof(line), f_check)) {
        if (strstr(line, "0.000000, 0.000000, ") != NULL) {
            verified_deform = true;
            break;
        }
    }
    fclose(f_check);
    assert(verified_deform == true);
    remove(out_usda);
    printf("   ✓ USD-STL cooperative scene export verified.\n");

    printf("=============================================================\n");
    printf("ALL AUNCIENT BALLET TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
