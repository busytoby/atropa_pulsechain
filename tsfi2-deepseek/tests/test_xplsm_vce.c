#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

// Mock XPLSM VCE runtime memory mapping
#define VCE_REG_SIZE 512

static uint8_t shadow_clut[VCE_REG_SIZE] = {0};
static uint8_t physical_clut[VCE_REG_SIZE] = {0};
static bool vblank_active = false;
static bool dirty_clut = false;

// Emulates XPLSM color packing utility: (R << 6) | (G << 3) | B
uint16_t xplsm_pack_color(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0x7) << 6) | ((g & 0x7) << 3) | (b & 0x7);
}

// Emulates XPLSM byte-writing intercept routine
void xplsm_write_vce(uint16_t offset, uint8_t val) {
    if (offset >= VCE_REG_SIZE) return;
    
    // Write to shadow buffer first to prevent mid-frame tearing
    shadow_clut[offset] = val;
    dirty_clut = true;
    
    // If VBlank is already active, flush immediately
    if (vblank_active) {
        physical_clut[offset] = shadow_clut[offset];
    }
}

// Emulates XPLSM VBlank interrupt handler
void xplsm_trigger_vblank_interrupt() {
    vblank_active = true;
    
    // Flush shadow CLUT to physical CLUT during vertical blanking retrace
    if (dirty_clut) {
        memcpy(physical_clut, shadow_clut, VCE_REG_SIZE);
        dirty_clut = false;
        printf("[XPLSM] VBlank interrupt triggered: Shadow CLUT flushed to VCE.\n");
    }
    
    vblank_active = false;
}

int main() {
    printf("=== TSFi ZMM VM Auncient XPLSM VCE Runtime Test ===\n");

    // 1. Test 9-bit color packing utility
    printf("[XPLSM] Testing pack color API...\n");
    uint16_t packed1 = xplsm_pack_color(4, 6, 2);
    assert(packed1 == 306); // 0x132

    uint16_t packed2 = xplsm_pack_color(7, 7, 7);
    assert(packed2 == 511); // 0x1FF

    // 2. Test VCE shadow buffer writes when VBlank is inactive
    printf("[XPLSM] Simulating VCE write during active frame (VBlank inactive)...\n");
    vblank_active = false;
    
    // Write packed color bytes (VCE offset 0 and 1)
    xplsm_write_vce(0, (packed1 >> 8) & 0xFF);
    xplsm_write_vce(1, packed1 & 0xFF);
    
    // Assert shadow holds the values but physical CLUT does not yet
    assert(shadow_clut[0] == 1);
    assert(shadow_clut[1] == 50); // 306 & 0xFF = 50 (0x32)
    assert(physical_clut[0] == 0);
    assert(physical_clut[1] == 0);
    printf("[XPLSM] Success: Physical VCE remains protected during scan cycles.\n");

    // 3. Trigger VBlank interrupt and assert values are flushed to hardware VCE
    printf("[XPLSM] Simulating retrace interval...\n");
    xplsm_trigger_vblank_interrupt();
    
    assert(physical_clut[0] == 1);
    assert(physical_clut[1] == 50);
    printf("[XPLSM] Success: Registers successfully committed to hardware VCE.\n");

    printf("=== ALL AUNCIENT XPLSM RUNTIME TESTS PASSED ===\n");
    return 0;
}
