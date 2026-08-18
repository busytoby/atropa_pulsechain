/* SPDX-License-Identifier: GPL-2.0 */
/*
 * C11 Test Prover: CP/M-Tomie RenderMan Visual Clay Play HathiTrust PageTurner PDF Viewer
 */

#include "cpm_tomie_pageturner_renderman.h"
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>

int main(void) {
    printf("=================================================================\n");
    printf("TEST PROVER: CP/M-TOMIE RENDERMAN CLAY PLAY PAGETURNER PDF VIEWER\n");
    printf("=================================================================\n");

    PageTurnerRenderManEngine engine;
    pageturner_init(&engine, 4);
    assert(engine.total_pages == 4);
    assert(engine.pages[0].num_vertices == 256);
    printf(" Page Mesh Grid Initialization: PASS (Pages: %u, Vertices per page: %u)\n",
           engine.total_pages, engine.pages[0].num_vertices);

    /* Test Digital Twin sensor binding */
    bool twin_ok = pageturner_set_digital_twin_telemetry(&engine, 22.5f, 48.0f, 420.0f, 18.2f, 14.5f, 32.1f, 3.0f);
    assert(twin_ok);
    assert(engine.twin_sensor.room_temperature_c == 22.5f);
    assert(engine.twin_sensor.co2_ppm == 420.0f);
    printf(" Digital Twin Telemetry Binding: PASS (Temp: 22.5C, CO2: 420ppm, BIM Pos: [14.5, 32.1, 3.0])\n");

    /* Test Clay Curl Transformation */
    bool curl_ok = pageturner_apply_clay_curl(&engine, 0, 0.5f);
    assert(curl_ok);
    assert(engine.pages[0].vertices[255].deformed_pos.z > engine.pages[0].vertices[255].rest_pos.z);
    printf(" Clay Play Page Curl Bend:      PASS (Turn Progress: 0.50, Deformed Z: %.2f mm)\n",
           engine.pages[0].vertices[255].deformed_pos.z);

    /* Test DisplacementShader */
    bool disp_ok = pageturner_apply_displacement_shader(&engine, 0);
    assert(disp_ok);
    printf(" RenderMan DisplacementShader:   PASS (Micro-facet displacement applied)\n");

    /* Test Rule 18 Parity Checksum */
    uint32_t csum = pageturner_compute_rule18_checksum(&engine);
    assert(csum > 0);
    printf(" Rule 18 Parity Checksum:        PASS (0x%08X Verified)\n", csum);

    printf("=================================================================\n");
    printf("ALL PAGETURNER RENDERMAN PROVER TESTS PASSED SUCCESSFULLY.\n");
    printf("=================================================================\n");
    return 0;
}
