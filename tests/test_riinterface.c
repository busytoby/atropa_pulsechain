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

    // Test camera-panning speed PSG audio modulation
    tsfi_riinterface_modulate_psg(&ri, 5.0);
    assert(ri.psg_channel_freq[0] == 600);
    assert(ri.psg_channel_vol[0] == 20);
    assert(ri.psg_channel_pan[0] == 0xF0); // Left channel balance
    assert(ri.psg_channel_pan[4] == 0x0F); // Right channel balance

    // Test VDC DMA block transfer of registers
    tsfi_riinterface_vdc_dma_copy(&ri, 5, 8, 2);
    assert(ri.hudson_vce_color_reg[8] == (20 << 5)); // Copied color value

    // Test Verlet integration applied strictly to simulated FET discharge cycles
    double pos_x[3] = {10.0, 20.0, 30.0};
    double prev_pos_x[3] = {9.5, 19.5, 29.5};
    tsfi_riinterface_discharge_verlet(&ri, pos_x, prev_pos_x, 3, 0.1, 0.99);

    // Verify coordinates evolved according to Verlet math: x + (x - prev) * decay - F
    assert(pos_x[0] > 10.0);
    assert(prev_pos_x[0] == 10.0);

    tsfi_riinterface_world_end(&ri);
    assert(ri.is_world_active == false);

    printf("   ✓ RiWorldBegin and RiSphere mirrored to Hudson VCE and VDC successfully.\n");
    printf("   ✓ Tom Hudson clipLine boundary checks verified successfully.\n");
    printf("   ✓ Camera panning dynamic PSG frequency modulation verified successfully.\n");
    printf("   ✓ VDC hardware DMA block transfers verified successfully.\n");
    printf("   ✓ Soft body physics Verlet FET discharge integration verified successfully.\n");
    printf("=== AUNCIENT RIINTERFACE TESTS COMPLETE (PASS) ===\n");
    return 0;
}
