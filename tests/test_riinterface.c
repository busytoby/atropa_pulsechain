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

    // Test periodic timer interrupts (IRQs)
    tsfi_riinterface_tick_irq(&ri, 0.010);
    assert(ri.irq_active == false);

    tsfi_riinterface_tick_irq(&ri, 0.010); // Exceeds 15ms total
    assert(ri.irq_active == true);
    assert(ri.irq_counter == 1);

    // Test complete 8-step sequence execution loop mapped to Hudson hardware
    TSFiRiInterface loop_ri;
    double loop_pos_x[1] = {10.0};
    double loop_prev_pos_x[1] = {9.5};
    tsfi_riinterface_run_8step_loop(&loop_ri, 4.0, loop_pos_x, loop_prev_pos_x, 1);

    // Assert that hardware states across all 8 sequence stages were set correctly
    assert(loop_ri.irq_counter == 1);
    assert(loop_ri.psg_channel_freq[0] == 550);
    assert(loop_ri.active_sprite_id == 1);
    assert(loop_ri.is_world_active == false); // Reset by RiEnd

    // Test custom 5-bit PSG wavetable writes
    uint8_t wave_src[32];
    for (int i = 0; i < 32; i++) wave_src[i] = (uint8_t)(i + 10);
    tsfi_riinterface_write_wavetable(&ri, 2, wave_src);
    assert(ri.psg_channel_wavetable[2][0] == 10);
    assert(ri.psg_channel_wavetable[2][31] == (41 & 0x1F)); // 5-bit mask clamp

    // Test PSG LFO enable controls
    tsfi_riinterface_enable_lfo(&ri, 0x80, 0x3F);
    assert(ri.psg_lfo_ctrl == 0x80);
    assert(ri.psg_lfo_freq == 0x3F);

    // Test PSG LFO phase displacement controls
    tsfi_riinterface_set_lfo_phase(&ri, 0x4B);
    assert(ri.psg_lfo_phase == 0x4B);

    // Test PSG white noise generator enable on channel 5
    tsfi_riinterface_enable_noise(&ri, 5, true, 0x1A);
    assert(ri.psg_noise_ctrl[1] == (0x80 | 0x1A));

    // Test VDC background scrolling
    tsfi_riinterface_scroll_background(&ri, 128, 64);
    assert(ri.vdc_scroll_x == 128);
    assert(ri.vdc_scroll_y == 64);

    // Test PSG Direct D/A (DDA) voice stream writes
    tsfi_riinterface_write_dda(&ri, 1, 0x1F); // Maximum D/A value
    assert(ri.psg_channel_dda[1] == 0x1F);

    tsfi_riinterface_write_dda(&ri, 3, 0x25); // Over-limit: clamped
    assert(ri.psg_channel_dda[3] == (0x25 & 0x1F));

    // Test VDC sprite collision flag triggers
    tsfi_riinterface_check_sprite_collision(&ri, 10.0, 10.0, 12.0, 10.0, 5.0); // Inside limit
    assert(ri.vdc_collision_flag == true);

    tsfi_riinterface_check_sprite_collision(&ri, 10.0, 10.0, 20.0, 10.0, 5.0); // Outside limit
    assert(ri.vdc_collision_flag == false);

    tsfi_riinterface_world_end(&ri);
    assert(ri.is_world_active == false);

    printf("   ✓ RiWorldBegin and RiSphere mirrored to Hudson VCE and VDC successfully.\n");
    printf("   ✓ Tom Hudson clipLine boundary checks verified successfully.\n");
    printf("   ✓ Camera panning dynamic PSG frequency modulation verified successfully.\n");
    printf("   ✓ VDC hardware DMA block transfers verified successfully.\n");
    printf("   ✓ Soft body physics Verlet FET discharge integration verified successfully.\n");
    printf("   ✓ Hudson soft periodic timer IRQ interrupts verified successfully.\n");
    printf("   ✓ End-to-end 8-step RenderMan execution sequence verified successfully.\n");
    printf("   ✓ Custom 5-bit PSG audio wavetables verified successfully.\n");
    printf("   ✓ PSG hardware LFO frequency modulation controls verified successfully.\n");
    printf("   ✓ PSG hardware LFO phase displacement controls verified successfully.\n");
    printf("   ✓ PSG white noise generator frequency controls verified successfully.\n");
    printf("   ✓ VDC background scroll registers verified successfully.\n");
    printf("   ✓ PSG Direct D/A real-time audio streams verified successfully.\n");
    printf("   ✓ VDC sprite collision interrupt flags verified successfully.\n");
    printf("=== AUNCIENT RIINTERFACE TESTS COMPLETE (PASS) ===\n");
    return 0;
}
