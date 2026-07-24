#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define STACK_CAPACITY 32
#define COLOR_RED      0x00FF
#define COLOR_CYAN     0xFFFF
#define COLOR_BLUE     0xFF0000
#define WARNING_DRONE  110 // Hz low drone

typedef struct {
    int64_t data[STACK_CAPACITY];
    int top;
    int checkpoint_top;
} loader_stack_t;

typedef struct {
    loader_stack_t stack;
    uint32_t border_color;
    uint32_t psg_frequency;
    
    // Blame registry
    bool blame_quarantine;
    uint32_t blamed_port;
} ocean_loader_ctx_t;

// 1. Dynamic Split-Raster Stripe Generator (modulates border lines based on SCSI bits)
static void update_raster_border(ocean_loader_ctx_t *ctx, uint8_t scsi_byte) {
    // If bit 0 is high, render CYAN stripe, else RED
    if (scsi_byte & 1) {
        ctx->border_color = COLOR_CYAN;
    } else {
        ctx->border_color = COLOR_RED;
    }
}

// 2. PSG Audio Failure Alarm Drone
static void set_psg_alarm(ocean_loader_ctx_t *ctx) {
    ctx->psg_frequency = WARNING_DRONE;
    printf("   [AUDIO] PSG Channel 1 output warning drone (%d Hz).\n", WARNING_DRONE);
}

// 3. SCSI Ingestion Checksum Blame & Revert
static bool ingest_scsi_block(ocean_loader_ctx_t *ctx, const uint8_t *data, size_t size, uint8_t expected_checksum, uint32_t input_port) {
    // Save stack checkpoint before loading
    ctx->stack.checkpoint_top = ctx->stack.top;
    
    uint8_t calculated_checksum = 0;
    for (size_t i = 0; i < size; i++) {
        calculated_checksum ^= data[i];
        
        // Dynamic split-raster striped border updates
        update_raster_border(ctx, data[i]);
        
        // Push loaded bytes onto stack
        if (ctx->stack.top < STACK_CAPACITY) {
            ctx->stack.data[ctx->stack.top++] = data[i];
        }
    }

    if (calculated_checksum != expected_checksum) {
        // Verification failed: Trigger rollback, log blame quarantine, play alarm drone
        ctx->stack.top = ctx->stack.checkpoint_top; // Revert stack
        ctx->blame_quarantine = true;
        ctx->blamed_port = input_port;
        set_psg_alarm(ctx);
        printf("   [BLAME] Checksum mismatch! Port %d quarantined. Stack rolled back.\n", input_port);
        return false;
    }

    // Success arpeggio frequency update (C-Major C4 261Hz)
    ctx->psg_frequency = 261;
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT HUCOCEAN LOADER SIMULATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    ocean_loader_ctx_t loader;
    memset(&loader, 0, sizeof(ocean_loader_ctx_t));
    loader.border_color = COLOR_BLUE;
    loader.psg_frequency = 261;

    // Push initial baseline stack
    loader.stack.data[loader.stack.top++] = 100;
    loader.stack.data[loader.stack.top++] = 200;

    // 1. Success Ingestion
    printf("[TEST] Ingesting valid SCSI block (expected pass)...\n");
    fflush(stdout);
    uint8_t valid_block[4] = { 0x12, 0x34, 0x56, 0x78 };
    uint8_t valid_checksum = 0x12 ^ 0x34 ^ 0x56 ^ 0x78;

    bool success = ingest_scsi_block(&loader, valid_block, 4, valid_checksum, 5);
    assert(success == true);
    assert(loader.blame_quarantine == false);
    assert(loader.stack.top == 6);
    assert(loader.psg_frequency == 261);
    printf("   ✓ Ingest successful. Border color: 0x%06X.\n", loader.border_color);
    fflush(stdout);

    // 2. Failing Ingestion with Blame & Rollback
    printf("[TEST] Ingesting corrupted SCSI block (expected rollback & quarantine)...\n");
    fflush(stdout);
    uint8_t corrupt_block[4] = { 0x11, 0x22, 0x33, 0x44 };
    uint8_t bad_checksum = 0xFF; // Mismatch

    success = ingest_scsi_block(&loader, corrupt_block, 4, bad_checksum, 5);
    assert(success == false);
    assert(loader.blame_quarantine == true);
    assert(loader.blamed_port == 5);
    assert(loader.stack.top == 6); // Rolled back to success state
    assert(loader.psg_frequency == WARNING_DRONE);
    printf("   ✓ Ingestion rollback, blame quarantine, and PSG audio warning verified.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("HUCOCEAN LOADER TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
