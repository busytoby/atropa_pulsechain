#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "auncient_cactus_schema.h"

// Tripartite Glossary Composition Validation
// 1. VM Register Context: The WinchesterMQ SCSI handshake loop registers mapped to the dynamic USD stage coordinates.
// 2. Mathematical Function: Modular exponential mappings and Verlet soft-body soft constraints.
// 3. Visual / Geometric Manifestation: Warping coordinate offset boundaries for the animated teddy bear geometry.

static int usd_serialize_teddy_bear_stage(const char *filename, const usd_AuncientTeddyBearSchema_t *schema, const usd_auncient_physics_api_t *physics) {
    FILE *f = fopen(filename, "wb");
    if (!f) return 0;
    if (fwrite(schema, sizeof(usd_AuncientTeddyBearSchema_t), 1, f) != 1) {
        fclose(f);
        return 0;
    }
    if (fwrite(physics, sizeof(usd_auncient_physics_api_t), 1, f) != 1) {
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}

static int usd_deserialize_teddy_bear_stage(const char *filename, usd_AuncientTeddyBearSchema_t *schema, usd_auncient_physics_api_t *physics) {
    FILE *f = fopen(filename, "rb");
    if (!f) return 0;
    if (fread(schema, sizeof(usd_AuncientTeddyBearSchema_t), 1, f) != 1) {
        fclose(f);
        return 0;
    }
    if (fread(physics, sizeof(usd_auncient_physics_api_t), 1, f) != 1) {
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}

static int usd_generate_new_teddy_bear_asset(const char *filename, const usd_AuncientTeddyBearSchema_t *schema, const usd_auncient_physics_api_t *physics) {
    FILE *f = fopen(filename, "w");
    if (!f) return 0;
    fprintf(f, "#usda 1.0\n\n");
    fprintf(f, "def \"TeddyBearAsset\" (\n");
    fprintf(f, "    inherits = </AuncientTeddyBearSchema>\n");
    fprintf(f, ") {\n");
    fprintf(f, "    float scale = %f\n", schema->scale);
    fprintf(f, "    float stuffing = %f\n", schema->stuffing);
    fprintf(f, "    float arm_length = %f\n", schema->arm_length);
    fprintf(f, "    float leg_length = %f\n", schema->leg_length);
    fprintf(f, "    float head_size = %f\n", schema->head_size);
    fprintf(f, "    float ear_size = %f\n", schema->ear_size);
    fprintf(f, "    float sickness = %f\n", schema->sickness);
    fprintf(f, "    int missing_eye = %d\n", schema->missing_eye);
    fprintf(f, "    float lighting_angle = %f\n", schema->lighting_angle);
    fprintf(f, "    float mass = %f\n", physics->mass);
    fprintf(f, "    float damping = %f\n", physics->damping);
    fprintf(f, "}\n");
    fclose(f);
    return 1;
}

int main(void) {
    printf("=== RUNNING AUNCIENT TEDDY BEAR USD SYSTEM TESTS ===\n");

    // Initialize Schema structures
    usd_AuncientTeddyBearSchema_t save_teddy_bear;
    usd_init_AuncientTeddyBearSchema(&save_teddy_bear);
    save_teddy_bear.scale = 1.2f;
    save_teddy_bear.stuffing = 1.8f;
    save_teddy_bear.arm_length = 1.1f;
    save_teddy_bear.leg_length = 0.9f;
    save_teddy_bear.sickness = 0.75f;
    save_teddy_bear.missing_eye = 1;
    save_teddy_bear.lighting_angle = 210.0f;

    usd_auncient_physics_api_t save_physics;
    usd_init_auncient_physics_api(&save_physics);
    save_physics.mass = 12.0f;
    save_physics.damping = 0.15f;

    const char *binary_usdc_path = "tests/teddy_bear_slice.usdc";
    const char *ascii_usda_path = "tests/teddy_bear_slice.usda";

    // Test USDC Binary Serialization/Deserialization
    printf("[TEST] Testing USDC Stage Slice Serialization...\n");
    int write_ok = usd_serialize_teddy_bear_stage(binary_usdc_path, &save_teddy_bear, &save_physics);
    assert(write_ok == 1);

    usd_AuncientTeddyBearSchema_t load_teddy_bear;
    usd_auncient_physics_api_t load_physics;
    int read_ok = usd_deserialize_teddy_bear_stage(binary_usdc_path, &load_teddy_bear, &load_physics);
    assert(read_ok == 1);
    assert(fabs(load_teddy_bear.scale - 1.2f) < 1e-5f);
    assert(fabs(load_teddy_bear.stuffing - 1.8f) < 1e-5f);
    assert(fabs(load_teddy_bear.arm_length - 1.1f) < 1e-5f);
    assert(fabs(load_teddy_bear.leg_length - 0.9f) < 1e-5f);
    assert(fabs(load_teddy_bear.sickness - 0.75f) < 1e-5f);
    assert(load_teddy_bear.missing_eye == 1);
    assert(fabs(load_teddy_bear.lighting_angle - 210.0f) < 1e-5f);
    assert(fabs(load_physics.mass - 12.0f) < 1e-5f);
    assert(fabs(load_physics.damping - 0.15f) < 1e-5f);
    printf("   ✓ USDC binary serialization & deserialization verified.\n");

    // Test USDA ASCII Asset Generation
    printf("[TEST] Testing USDA Stage Asset Generation...\n");
    int gen_ok = usd_generate_new_teddy_bear_asset(ascii_usda_path, &save_teddy_bear, &save_physics);
    assert(gen_ok == 1);

    FILE *rf = fopen(ascii_usda_path, "r");
    assert(rf != NULL);
    char buf[128];
    bool found_usda = false;
    bool found_stuffing = false;
    bool found_sickness = false;
    bool found_missing_eye = false;
    while (fgets(buf, sizeof(buf), rf)) {
        if (strstr(buf, "#usda 1.0")) found_usda = true;
        if (strstr(buf, "stuffing = 1.8")) found_stuffing = true;
        if (strstr(buf, "sickness = 0.75")) found_sickness = true;
        if (strstr(buf, "missing_eye = 1")) found_missing_eye = true;
    }
    fclose(rf);
    assert(found_usda);
    assert(found_stuffing);
    assert(found_sickness);
    assert(found_missing_eye);
    printf("   ✓ USDA ASCII scene format output verified.\n");


    // Cleanup generated files
    remove(binary_usdc_path);
    remove(ascii_usda_path);

    printf("=== AUNCIENT TEDDY BEAR USD SYSTEM TESTS COMPLETE (PASS) ===\n");
    return 0;
}
