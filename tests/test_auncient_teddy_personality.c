#include "../include/auncient_teddy_personality.h"
#include <stdio.h>
#include <assert.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT RENDERMAN TEDDY BEAR PERSONALITY CONFIGURATION TEST\n");
    printf("=============================================================\n");

    teddy_geometry_t geom;

    // Test Trustworthy Mapping
    resolve_teddy_geometry(PERSONALITY_TRUSTWORTHY, &geom);
    assert(geom.head_fwhr == 1.0);
    assert(geom.eye_eccentricity == 1.0);
    assert(geom.eye_scale == 1.0);
    assert(geom.stiffness == 0.3);
    printf("   ✓ Trustworthy parameters resolved: Round Head (1.0), Soft Dynamics (Stiffness: 0.3)\n");

    // Test Aggressive Mapping
    resolve_teddy_geometry(PERSONALITY_AGGRESSIVE, &geom);
    assert(geom.head_fwhr == 0.7);
    assert(geom.eye_eccentricity == 0.5);
    assert(geom.stiffness == 0.9);
    printf("   ✓ Aggressive parameters resolved: Narrow Head (0.7), Rigid Dynamics (Stiffness: 0.9)\n");

    // Test Eerie Mapping
    resolve_teddy_geometry(PERSONALITY_EERIE, &geom);
    assert(geom.head_fwhr == 0.7);
    assert(geom.eye_scale == 1.8);
    assert(geom.damping == 0.1);
    printf("   ✓ Eerie parameters resolved: Narrow Head (0.7), Eerie Dynamics (Damping: 0.1)\n");

    printf("=============================================================\n");
    printf("PERSONALITY CONFIGURATIONS VALIDATED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
