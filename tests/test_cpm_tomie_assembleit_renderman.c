#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "cpm_tomie_assembleit_renderman.h"

int main(void) {
    printf("===============================================================\n");
    printf("TESTING ASSEMBLEIT CLAY SCULPTING & RENDERMAN HARDWARE VM ENGINE\n");
    printf("===============================================================\n");

    AssembleitModel model;
    assembleit_init_model(&model);

    /* Build a 4-node tetrahedral chassis */
    assert(assembleit_add_node(&model, 0, 0.0f, 0.0f, 0.0f, true));
    assert(assembleit_add_node(&model, 1, 300.0f, 0.0f, 0.0f, true));
    assert(assembleit_add_node(&model, 2, 150.0f, 259.8f, 0.0f, false));
    assert(assembleit_add_node(&model, 3, 150.0f, 86.6f, 244.9f, false));

    /* Add 6 actuated truss links */
    assert(assembleit_add_strut(&model, 0, 0, 1, 300.0f));
    assert(assembleit_add_strut(&model, 1, 1, 2, 300.0f));
    assert(assembleit_add_strut(&model, 2, 2, 0, 300.0f));
    assert(assembleit_add_strut(&model, 3, 0, 3, 300.0f));
    assert(assembleit_add_strut(&model, 4, 1, 3, 300.0f));
    assert(assembleit_add_strut(&model, 5, 2, 3, 300.0f));

    /* Add synthetic matrix skin vertices */
    assert(assembleit_add_clay_vertex(&model, 150.0f, 86.6f, 280.0f, 0.25f, 0.5f));
    assert(assembleit_add_clay_vertex(&model, 160.0f, 90.0f, 290.0f, 0.5f, 0.75f));

    /* Kinematic step actuation to 400 mm */
    model.struts[0].target_length_mm = 400.0f;
    model.struts[1].target_length_mm = 380.0f;
    assert(assembleit_solve_kinematics(&model, 1.0f));

    /* RenderMan DisplacementShader evaluation (Rule 14) */
    assert(assembleit_apply_displacement_shader(&model, 5.0f));

    /* Morton quadtree index calculation */
    uint32_t morton = assembleit_compute_morton_quadtree_index(&model);
    printf("Computed 24-Bit Morton Quadtree Index: 0x%06X\n", morton);

    /* REYES sub-pixel micropolygon tessellation count */
    size_t micropolygon_count = 0;
    assert(assembleit_evaluate_renderman_micropolygons(&model, &micropolygon_count));
    assert(micropolygon_count == 32);

    /* Synthesize Gerchberg-Saxton 2D Holographic Quadtree */
    assert(assembleit_synthesize_gerchberg_quadtree(&model, 2));
    assert(model.holo_quadtree.node_count == 5);

    /* Save to .dat.bin quadtree storage (Rule 13) */
    const char *test_bin_path = "/tmp/test_assembleit_holo.dat.bin";
    assert(assembleit_save_quadtree_binary(&model, test_bin_path));
    unlink(test_bin_path);

    /* Test BDOS Hardware I/O Ports (0x48 .. 0x4B) */
    assembleit_bdos_write_port(&model, ASSEMBLEIT_PORT_STROKE_TELEMETRY, 100);
    assert(assembleit_solve_kinematics(&model, 2.0f));
    uint8_t stroke_telemetry = assembleit_bdos_read_port(&model, ASSEMBLEIT_PORT_STROKE_TELEMETRY);
    assert(stroke_telemetry == 100);

    uint8_t adc_val = assembleit_bdos_read_port(&model, ASSEMBLEIT_PORT_IMPEDANCE_ADC);
    printf("BDOS PORT 0x49 ADC Value: 0x%02X\n", adc_val);

    uint8_t quad_status = assembleit_bdos_read_port(&model, ASSEMBLEIT_PORT_QUADTREE_STATUS);
    assert(quad_status == 5);

    printf("✓ All AssembleIt RenderMan Hardware VM Tests Passed Successfully!\n");
    printf("===============================================================\n");
    return 0;
}
