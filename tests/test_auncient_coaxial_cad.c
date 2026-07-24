#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

// Coaxial Path Layout representation
typedef struct {
    double d;          // Center conductor diameter (mm)
    double D;          // Outer shield diameter (mm)
    double epsilon_r;  // Dielectric constant of insulator
    double spacing;    // Distance to adjacent coaxial path (mm)
} coax_path_t;

typedef enum {
    CAD_RULE_FAIL,
    CAD_RULE_PASS
} cad_status_t;

// -------------------------------------------------------------
// Coaxial CAD Design Rule Checker (DRC)
// -------------------------------------------------------------
cad_status_t cad_verify_coaxial_path(const coax_path_t *path, double *calc_z0) {
    if (path->epsilon_r <= 0.0 || path->d <= 0.0 || path->D <= path->d) {
        return CAD_RULE_FAIL;
    }

    // 1. Calculate characteristic impedance: Z0 = (138.0 / sqrt(epsilon_r)) * log10(D / d)
    double sqrt_er = sqrt(path->epsilon_r);
    *calc_z0 = (138.0 / sqrt_er) * log10(path->D / path->d);

    // Impedance must match 50 Ohms within +/- 2 Ohms tolerance
    if (*calc_z0 < 48.0 || *calc_z0 > 52.0) {
        return CAD_RULE_FAIL;
    }

    // 2. Cross-coupling spacing check: spacing must be at least 3x the shield diameter
    double required_spacing = 3.0 * path->D;
    if (path->spacing < required_spacing) {
        return CAD_RULE_FAIL;
    }

    return CAD_RULE_PASS;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT COAXIAL CAD AUDITOR VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // 1. Compliant path configuration (Z0 near 50 Ohms, wide spacing)
    // d = 1.0, D = 3.3, epsilon_r = 2.2 (Solid PTFE)
    // Z0 = (138.0 / sqrt(2.2)) * log10(3.3 / 1.0) = 93.04 * 0.5185 = 48.24 Ohms
    coax_path_t path_ok = {
        .d = 1.0,
        .D = 3.3,
        .epsilon_r = 2.2,
        .spacing = 10.0 // > 3 * 3.3 (9.9 mm)
    };

    double z0 = 0.0;
    cad_status_t status = cad_verify_coaxial_path(&path_ok, &z0);
    assert(status == CAD_RULE_PASS);
    printf("   ✓ Compliant path passed: Z0 = %.2f Ohms, spacing = %.2f mm\n", z0, path_ok.spacing);
    fflush(stdout);

    // 2. Mismatched impedance (D is too large, resulting in high Z0)
    coax_path_t path_high_z0 = {
        .d = 1.0,
        .D = 6.0,
        .epsilon_r = 2.2,
        .spacing = 20.0
    };

    status = cad_verify_coaxial_path(&path_high_z0, &z0);
    assert(status == CAD_RULE_FAIL);
    printf("   ✓ Impedance mismatch check verified: Z0 = %.2f Ohms (Out of bounds)\n", z0);
    fflush(stdout);

    // 3. Spacing violation (Adjacent lines too close, causing crosstalk risk)
    coax_path_t path_crosstalk = {
        .d = 1.0,
        .D = 3.3,
        .epsilon_r = 2.2,
        .spacing = 5.0 // < 9.9 mm
    };

    status = cad_verify_coaxial_path(&path_crosstalk, &z0);
    assert(status == CAD_RULE_FAIL);
    printf("   ✓ Crosstalk spacing violation caught: Spacing of %.2f mm rejected.\n", path_crosstalk.spacing);
    fflush(stdout);

    printf("=============================================================\n");
    printf("COAXIAL CAD AUDITOR TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
