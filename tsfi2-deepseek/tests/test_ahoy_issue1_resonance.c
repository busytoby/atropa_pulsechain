#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_helmholtz.h"
#include "tsfi_merkle.h"
#include "tsfi_math.h"
#include "tsfi_io.h"

// Emulating C64 memory-mapped I/O for SID chip
#define SID_BASE 54272
// Voice 1 Registers
#define SID_V1_FREQ_LO (SID_BASE + 0)
#define SID_V1_FREQ_HI (SID_BASE + 1)
#define SID_V1_CTRL     (SID_BASE + 4)
#define SID_V1_ATT_DEC  (SID_BASE + 5)
#define SID_V1_SUT_REL  (SID_BASE + 6)
// Voice 2 Registers
#define SID_V2_FREQ_LO (SID_BASE + 7)
#define SID_V2_FREQ_HI (SID_BASE + 8)
#define SID_V2_CTRL     (SID_BASE + 11)
#define SID_V2_ATT_DEC  (SID_BASE + 12)
#define SID_V2_SUT_REL  (SID_BASE + 13)
// Voice 3 Registers
#define SID_V3_FREQ_LO (SID_BASE + 14)
#define SID_V3_FREQ_HI (SID_BASE + 15)
#define SID_V3_CTRL     (SID_BASE + 18)
#define SID_V3_ATT_DEC  (SID_BASE + 19)
#define SID_V3_SUT_REL  (SID_BASE + 20)

#define SID_VOLUME      (SID_BASE + 24)

// Simple emulation of POKE and PEEK
static uint8_t c64_ram[65536];

void poke(uint16_t addr, uint8_t val) {
    c64_ram[addr] = val;
}

uint8_t peek(uint16_t addr) {
    return c64_ram[addr];
}

// 3-Voice Polyphonic Sequence (Triads)
static const uint16_t melody_v1[4] = {0x1128, 0x159B, 0x1A40, 0x2250}; // Bass root
static const uint16_t melody_v2[4] = {0x159B, 0x1A40, 0x2250, 0x2B78}; // Thirds
static const uint16_t melody_v3[4] = {0x1A40, 0x2250, 0x2B78, 0x35E0}; // Fifths/Octaves
static int melody_step = 0;

void trigger_interrupt_music_maker() {
    // Read the current step notes for all 3 voices
    uint16_t f1 = melody_v1[melody_step];
    uint16_t f2 = melody_v2[melody_step];
    uint16_t f3 = melody_v3[melody_step];
    
    // Voice 1 Poke
    poke(SID_V1_FREQ_LO, f1 & 0xFF);
    poke(SID_V1_FREQ_HI, (f1 >> 8) & 0xFF);
    poke(SID_V1_CTRL, 0x11); // Gate on (Triangle)
    
    // Voice 2 Poke
    poke(SID_V2_FREQ_LO, f2 & 0xFF);
    poke(SID_V2_FREQ_HI, (f2 >> 8) & 0xFF);
    poke(SID_V2_CTRL, 0x21); // Gate on (Sawtooth)
    
    // Voice 3 Poke
    poke(SID_V3_FREQ_LO, f3 & 0xFF);
    poke(SID_V3_FREQ_HI, (f3 >> 8) & 0xFF);
    poke(SID_V3_CTRL, 0x41); // Gate on (Pulse)
    
    melody_step = (melody_step + 1) % 4;
}

int main() {
    printf("=== TSFi2 & Ahoy! Magazine Issue 2 Polyphonic Resonance Integration Test ===\n");
    tsfi_helmholtz_init();

    // 1. Emulate Memory initialization (POKEs from Interrupt Music Maker Part II)
    printf("[AHOY!] Initializing Virtual 3-Voice C64 SID Registers...\n");
    poke(SID_VOLUME, 0x0F); // Max volume
    
    poke(SID_V1_ATT_DEC, 0x0A);
    poke(SID_V1_SUT_REL, 0x88);
    
    poke(SID_V2_ATT_DEC, 0x25);
    poke(SID_V2_SUT_REL, 0x77);
    
    poke(SID_V3_ATT_DEC, 0x12);
    poke(SID_V3_SUT_REL, 0x99);

    // 2. Prepare 512 KiB Manifold
    void *manifold = calloc(1, 512 * 1024);
    uint8_t *l0 = (uint8_t*)manifold;
    uint8_t *l1 = (uint8_t*)manifold + (1023 * 256);
    l0[0] = 0xAA; l0[1] = 0x01; // Seed manifold
    l1[0] = 0xBB; l1[1] = 0x02;

    uint8_t root_fw[32];
    uint8_t receipt_fw[32];
    uint32_t epoch = 1;

    // 3. Play the polyphonic chords through simulated C64 interrupts & compute acoustic proof roots
    printf("[AHOY!] Playing 3-voice polyphony and generating resonance proofs...\n");
    for (int i = 0; i < 4; i++) {
        trigger_interrupt_music_maker();
        
        // Retrieve voice frequencies from virtual registers
        uint16_t f1 = (peek(SID_V1_FREQ_HI) << 8) | peek(SID_V1_FREQ_LO);
        uint16_t f2 = (peek(SID_V2_FREQ_HI) << 8) | peek(SID_V2_FREQ_LO);
        uint16_t f3 = (peek(SID_V3_FREQ_HI) << 8) | peek(SID_V3_FREQ_LO);
        
        // Combine frequencies using fine-structure scaling factors
        uint64_t combined_k = ((uint64_t)f1 * 137) + ((uint64_t)f2 * 109) + ((uint64_t)f3 * 97);
        
        // Run Helmholtz reduction to compile Acoustic Proof of State
        tsfi_helmholtz_reduce_11(root_fw, receipt_fw, NULL, NULL, manifold, epoch, combined_k, NULL, NULL);
        
        printf("  Step %d -> V1: %d, V2: %d, V3: %d -> Root: ", i, f1, f2, f3);
        for(int j=0; j<8; j++) printf("%02x", root_fw[j]);
        printf("...\n");
        
        epoch++;
    }

    printf("[PASS] Integrated Interrupt Music Maker Part II with TSFi2 Helmholtz kernel.\n");

    free(manifold);
    
    // Check for memory leaks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_ahoy", NULL);

    return 0;
}
