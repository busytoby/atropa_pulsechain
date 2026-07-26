#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_geomsubset.h"

int main(void) {
    printf("=== RUNNING AUNCIENT GEOMSUBSET BINDING TESTS ===\n");

    TSFiGeomSubset subset;
    tsfi_geomsubset_init(&subset, "/World/Torus/GoldStripes");

    assert(strcmp(subset.subset_path, "/World/Torus/GoldStripes") == 0);
    assert(subset.face_count == 0);

    int faces[5] = {0, 4, 8, 12, 16};
    assert(tsfi_geomsubset_set_faces(&subset, faces, 5));
    assert(subset.face_count == 5);

    // Bind material path
    tsfi_geomsubset_bind_material(&subset, "/World/Materials/GoldFoil");
    assert(strcmp(subset.material_binding_path, "/World/Materials/GoldFoil") == 0);

    // Verify face checks
    assert(tsfi_geomsubset_has_face(&subset, 4) == true);
    assert(tsfi_geomsubset_has_face(&subset, 12) == true);
    assert(tsfi_geomsubset_has_face(&subset, 5) == false);

    printf("   ✓ GeomSubset face indices boundaries verified.\n");
    printf("   ✓ Material paths bindings verified successfully.\n");
    printf("=== AUNCIENT GEOMSUBSET BINDING TESTS COMPLETE (PASS) ===\n");
    return 0;
}
