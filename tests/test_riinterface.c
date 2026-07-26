#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_riinterface.h"

int main(void) {
    printf("=== RUNNING AUNCIENT RIINTERFACE TESTS ===\n");

    TSFiRiInterface ri;
    tsfi_riinterface_init(&ri);

    assert(ri.is_world_active == false);
    assert(ri.active_sprite_id == 0);

    // Test RiWorldBegin and Hudson VCE initialization
    tsfi_riinterface_world_begin(&ri);
    assert(ri.is_world_active == true);
    assert(ri.hudson_vce_color_reg[0] == 0xF800);

    // Test RiSphere mirroring to Hudson VDC and VCE
    bool ok = tsfi_riinterface_sphere(&ri, 5, 10.0);
    assert(ok == true);
    assert(ri.active_sprite_id == 5);
    assert(ri.hudson_vce_color_reg[5] == (20 << 5)); // Green color scale value

    // Test Tom Hudson clipLine limits checks
    assert(tsfi_riinterface_clip_check(&ri, 100.0, 100.0) == true);
    assert(tsfi_riinterface_clip_check(&ri, 800.0, 100.0) == false); // Out of bounds: clipped

    tsfi_riinterface_world_end(&ri);
    assert(ri.is_world_active == false);

    printf("   ✓ RiWorldBegin and RiSphere mirrored to Hudson VCE and VDC successfully.\n");
    printf("   ✓ Tom Hudson clipLine boundary checks verified successfully.\n");
    printf("=== AUNCIENT RIINTERFACE TESTS COMPLETE (PASS) ===\n");
    return 0;
}
