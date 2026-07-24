#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define VGPR_COUNT 256
#define SCSI_BUF_SIZE 64

typedef struct {
    uint32_t border_color;
    uint32_t psg_frequency;
    uint8_t scsi_buffer[SCSI_BUF_SIZE];
} huc_system_t;

typedef struct {
    float registers[VGPR_COUNT] __attribute__((aligned(64)));
} vgpr_bank_t;

// Promotes Hudson Soft system registers directly to VGPR lanes
static void promote_huc_to_vgpr(const huc_system_t *huc, vgpr_bank_t *vgprs) {
    // Map SCSI byte buffer directly to VGPR lanes 0..63
    for (int i = 0; i < SCSI_BUF_SIZE; i++) {
        vgprs->registers[i] = (float)huc->scsi_buffer[i];
    }
    
    // Map border color to VGPR lane 64
    vgprs->registers[64] = (float)huc->border_color;
    
    // Map PSG frequency to VGPR lane 65
    vgprs->registers[65] = (float)huc->psg_frequency;
}

// Demotes VGPR lanes back to Hudson Soft system registers
static void demote_vgpr_to_huc(const vgpr_bank_t *vgprs, huc_system_t *huc) {
    // Retrieve SCSI bytes from VGPR lanes 0..63
    for (int i = 0; i < SCSI_BUF_SIZE; i++) {
        huc->scsi_buffer[i] = (uint8_t)vgprs->registers[i];
    }
    
    // Retrieve border color from VGPR lane 64
    huc->border_color = (uint32_t)vgprs->registers[64];
    
    // Retrieve PSG frequency from VGPR lane 65
    huc->psg_frequency = (uint32_t)vgprs->registers[65];
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT HUDSON SOFT TO VGPR REGISTER MIRRORING SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    huc_system_t huc;
    memset(&huc, 0, sizeof(huc_system_t));
    huc.border_color = 0xFFFF; // Cyan
    huc.psg_frequency = 261;  // C4 note
    for (int i = 0; i < SCSI_BUF_SIZE; i++) {
        huc.scsi_buffer[i] = (uint8_t)(i * 2);
    }

    vgpr_bank_t vgprs;
    memset(&vgprs, 0, sizeof(vgpr_bank_t));

    // 1. Test Ingestion Promotion
    printf("[TEST] Promoting Hudson registers to VGPR float registers...\n");
    fflush(stdout);
    promote_huc_to_vgpr(&huc, &vgprs);

    // Verify promotion mappings
    assert(vgprs.registers[0] == 0.0f);
    assert(vgprs.registers[1] == 2.0f);
    assert(vgprs.registers[64] == (float)0xFFFF);
    assert(vgprs.registers[65] == 261.0f);
    printf("   ✓ Promotion successful. Mapped SCSI bytes and peripheral registers.\n");
    fflush(stdout);

    // 2. Perform simulated vector math operations on VGPR lanes
    printf("[TEST] Executing vector computations in VGPR bank...\n");
    fflush(stdout);
    
    // Vector multiply SCSI lanes
    for (int i = 0; i < SCSI_BUF_SIZE; i++) {
        vgprs.registers[i] *= 2.0f; // Scale input signal
    }
    
    // Update border color in lane 64 to RED (0x00FF)
    vgprs.registers[64] = (float)0x00FF;
    
    // Double the PSG frequency in lane 65
    vgprs.registers[65] *= 2.0f; 

    // 3. Test Demotion recovery
    printf("[TEST] Demoting VGPR registers back to Hudson peripheral registers...\n");
    fflush(stdout);
    demote_vgpr_to_huc(&vgprs, &huc);

    // Verify modifications resolved correctly
    assert(huc.scsi_buffer[1] == 4); // 2 * 2
    assert(huc.border_color == 0x00FF);
    assert(huc.psg_frequency == 522); // 261 * 2
    printf("   ✓ Demotion successful. Peripheral status registers synchronized.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("HUDSON TO VGPR MIRRORING TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
