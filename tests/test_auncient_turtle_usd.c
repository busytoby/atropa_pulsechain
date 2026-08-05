#include "../tsfi2-deepseek/src/auncient_turtle_usd.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT TURTLE GRAPHICS TO PIXAR USD BRIDGE TEST SUITE\n");
    printf("=============================================================\n");

    const char *usda_path = "tests/test_turtle_fractal.usda";
    const char *ri_usda_path = "tests/test_turtle_ri_fractal.usda";
    
    // 1. Generate L-System Koch Curve at depth 2 (90-degree angle delta)
    printf("[TEST] Generating L-System Koch Curve at depth 2 to USDA...\n");
    bool gen_ok = auncient_turtle_generate_usd("F", 2, 10.0f, 90.0f, usda_path);
    assert(gen_ok == true);
    printf("   ✓ USDA file generated successfully.\n");

    // 2. Read back and verify file headers
    FILE *file = fopen(usda_path, "r");
    assert(file != NULL);
    
    char line[256];
    bool found_usda_header = false;
    bool found_basis_curves = false;
    bool found_points = false;

    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "#usda 1.0")) {
            found_usda_header = true;
        }
        if (strstr(line, "def BasisCurves \"TurtleFractal\"")) {
            found_basis_curves = true;
        }
        if (strstr(line, "point3f[] points")) {
            found_points = true;
        }
    }
    fclose(file);

    assert(found_usda_header);
    assert(found_basis_curves);
    assert(found_points);
    printf("   ✓ USDA format compliance verified successfully.\n");

    // 3. Test RenderMan RiInterface and Hudson hardware integrated generator
    printf("[TEST] Initializing RenderMan RiInterface context...\n");
    TSFiRiInterface ri;
    tsfi_riinterface_init(&ri);
    ri.clip_min_x = 0.0;
    ri.clip_min_y = 0.0;
    ri.clip_max_x = 150.0; // Restrict bounds to force clipping
    ri.clip_max_y = 150.0;

    printf("[TEST] Generating RenderMan-audited turtle fractal (expected clipping)...\n");
    bool ri_ok = auncient_turtle_generate_usd_with_ri(&ri, "F", 2, 20.0f, 90.0f, ri_usda_path);
    assert(ri_ok == true);

    // Verify color registry was initialized via RiWorldBegin
    assert(ri.hudson_vce_color_reg[0] == 0xF800);

    // Verify PSG sound modulation updated registers based on step length (velocity = 20)
    assert(ri.psg_channel_freq[0] > 0);

    printf("   ✓ RenderMan RiInterface and Hudson hardware loop integration verified successfully.\n");

    // 4. Test VGPR Promotion
    printf("[TEST] Promoting turtle coordinate registers to VGPR lanes...\n");
    TurtleVGPRState vgpr;
    auncient_turtle_promote_to_vgpr(120.0f, 110.0f, 0.0f, 1.57f, &vgpr);
    assert(vgpr.vgpr_x[0] == 120.0f);
    assert(vgpr.vgpr_x[1] == 110.0f);
    assert(vgpr.vgpr_x[2] == 0.0f);
    assert(vgpr.vgpr_x[3] == 1.57f);
    printf("   ✓ VGPR register promotion verified successfully.\n");

    // 5. Test Quadtree Serialization (.dat.bin format compliance)
    printf("[TEST] Slicing turtle coordinates to binary quadtree (.dat.bin)...\n");
    const char *dat_bin_path = "tests/test_turtle_quadtree.dat.bin";
    bool slice_ok = auncient_turtle_slice_to_quadtree(dat_bin_path);
    assert(slice_ok == true);
    printf("   ✓ Binary quadtree slice serialized successfully.\n");

    printf("=============================================================\n");
    printf("ALL TURTLE USD BRIDGE TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
