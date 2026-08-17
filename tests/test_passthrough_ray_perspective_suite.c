#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

void test_passthrough_ray_perspective_basic(void) {
    printf("[TEST 1] Testing basic Tangent Pass-Through Ray Perspective Engine...\n");
    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_path = "/tmp/test_passthrough_basic.dat.bin";

    FILE *f = fopen(dat_bin_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    int64_t preserved_x = 5;
    int64_t preserved_y = 11;
    uint64_t arc_z = 100000; // z = 0.1 rad

    AuncientEulerVolume1Chapter8TangentPassThroughRayMetrics ray_metrics = {0};

    bool ok = auncient_euler_volume1_chapter8_tangent_passthrough_ray_engine(
        contract_addr,
        dat_bin_path,
        arc_z,
        preserved_x,
        preserved_y,
        &ray_metrics
    );

    assert(ok == true);
    assert(ray_metrics.ch8_passthrough_engine_sound == true);
    assert(ray_metrics.is_passthrough_ray_continuity_sound == true);
    assert(ray_metrics.passthrough_entry_vector_scaled == 4732707ULL);
    assert(ray_metrics.passthrough_exit_vector_scaled == 5217332ULL);
    assert(ray_metrics.passthrough_ray_intensity_scaled == 4969485ULL);
    assert(ray_metrics.rule9_address_resolution_sound == true);
    assert(ray_metrics.rule13_dat_bin_verified == true);

    remove(dat_bin_path);
    printf("  -> TEST 1 PASSED: Entry, Exit, and Ray Intensity match exact spatial metrics.\n\n");
}

void test_passthrough_ray_angle_refraction(void) {
    printf("[TEST 2] Testing Tangent Pass-Through Ray Angle Engine (切線穿過之角度)...\n");
    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_path = "/tmp/test_passthrough_angle.dat.bin";

    FILE *f = fopen(dat_bin_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    int64_t preserved_x = 5;
    int64_t preserved_y = 11;
    uint64_t arc_z = 100000; // z = 0.1 rad

    AuncientEulerVolume1Chapter8TangentPassThroughAngleMetrics angle_metrics = {0};

    bool ok = auncient_euler_volume1_chapter8_tangent_passthrough_angle_engine(
        contract_addr,
        dat_bin_path,
        arc_z,
        preserved_x,
        preserved_y,
        &angle_metrics
    );

    assert(ok == true);
    assert(angle_metrics.ch8_passthrough_angle_engine_sound == true);
    assert(angle_metrics.is_angle_refraction_sound == true);
    assert(angle_metrics.passthrough_angle_rad_scaled == 217277ULL);  // 0.217277 rad
    assert(angle_metrics.passthrough_angle_deg_scaled == 1244907ULL); // 12.44907 deg
    assert(angle_metrics.normal_incidence_angle_deg_scaled == 7755093ULL); // 77.55093 deg
    assert(angle_metrics.passthrough_angle_deg_scaled + angle_metrics.normal_incidence_angle_deg_scaled == 9000000ULL);

    remove(dat_bin_path);
    printf("  -> TEST 2 PASSED: Pass-through angle + normal incidence angle sum exactly to 90 degrees.\n\n");
}

void test_connected_reality_tangent_reconciliation(void) {
    printf("[TEST 3] Testing Connected Reality Tangent Origin Perspective Engine...\n");
    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_path = "/tmp/test_connected_tangent.dat.bin";

    FILE *f = fopen(dat_bin_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    int64_t preserved_x = 5;
    int64_t preserved_y = 11;
    uint64_t arc_z = 100000; // z = 0.1 rad

    AuncientEulerVolume1Chapter8ConnectedRealityTangentMetrics connected_metrics = {0};

    bool ok = auncient_euler_volume1_chapter8_connected_reality_tangent_engine(
        contract_addr,
        dat_bin_path,
        arc_z,
        preserved_x,
        preserved_y,
        &connected_metrics
    );

    assert(ok == true);
    assert(connected_metrics.ch8_connected_tangent_engine_sound == true);
    assert(connected_metrics.is_disconnected_2d_euler_fail_detected == true);
    assert(connected_metrics.is_originative_perspective_reconciled == true);
    assert(connected_metrics.origin_distance_R_scaled == 5094890ULL);
    assert(connected_metrics.connected_reality_tangent_scaled == 1124623ULL);

    remove(dat_bin_path);
    printf("  -> TEST 3 PASSED: Disconnected 2D Euler fail detected & originative perspective reconciled.\n\n");
}

void test_rule13_invalid_extension_rejection(void) {
    printf("[TEST 4] Testing Rule 13 (.json extension rejection constraint)...\n");
    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *json_path = "/tmp/invalid_quadtree.json";

    AuncientEulerVolume1Chapter8TangentPassThroughRayMetrics metrics = {0};
    bool ok = auncient_euler_volume1_chapter8_tangent_passthrough_ray_engine(
        contract_addr,
        json_path,
        100000,
        5,
        11,
        &metrics
    );

    assert(ok == false);
    printf("  -> TEST 4 PASSED: Engine correctly rejected non-.dat.bin file extension per Rule 13.\n\n");
}

void test_rule9_invalid_address_rejection(void) {
    printf("[TEST 5] Testing Rule 9 (Non-dynamic address resolution constraint)...\n");
    const char *invalid_addr = "static_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_path = "/tmp/test_rule9.dat.bin";

    FILE *f = fopen(dat_bin_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    AuncientEulerVolume1Chapter8TangentPassThroughRayMetrics metrics = {0};
    bool ok = auncient_euler_volume1_chapter8_tangent_passthrough_ray_engine(
        invalid_addr,
        dat_bin_path,
        100000,
        5,
        11,
        &metrics
    );

    assert(ok == true);
    assert(metrics.rule9_address_resolution_sound == false);
    assert(metrics.ch8_passthrough_engine_sound == false);

    remove(dat_bin_path);
    printf("  -> TEST 5 PASSED: Engine marked unsound due to missing dynamic_<address> prefix per Rule 9.\n\n");
}

int main(void) {
    printf("================================================================================\n");
    printf(" UNIT TEST SUITE: PASS-THROUGH RAY PERSPECTIVE ENGINES\n");
    printf("================================================================================\n\n");

    test_passthrough_ray_perspective_basic();
    test_passthrough_ray_angle_refraction();
    test_connected_reality_tangent_reconciliation();
    test_rule13_invalid_extension_rejection();
    test_rule9_invalid_address_rejection();

    printf("================================================================================\n");
    printf(" ALL 5 UNIT TESTS PASSED SUCCESSFULLY! (100%% PASS-THROUGH RAY SOUNDNESS)\n");
    printf("================================================================================\n");

    return 0;
}
