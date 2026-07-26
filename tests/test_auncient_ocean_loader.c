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
    uint8_t buffer[64];
    size_t write_ptr;
    size_t read_ptr;
    bool transfer_active;
    uint32_t transfer_cycles;
} tape_channel_t;

typedef struct {
    loader_stack_t stack;
    tape_channel_t tape;
    uint32_t border_color;
    uint32_t psg_frequency;
    
    // Blame registry
    bool blame_quarantine;
    uint32_t blamed_port;
} ocean_loader_ctx_t;

static uint32_t cycle_border_raster(uint32_t frame_count) {
    if (frame_count % 3 == 0) {
        return COLOR_RED;
    } else if (frame_count % 3 == 1) {
        return COLOR_CYAN;
    } else {
        return COLOR_BLUE;
    }
}

static uint32_t play_loader_arpeggio(uint32_t frame_count) {
    uint32_t note_index = frame_count % 3;
    if (note_index == 0) {
        return 261;
    } else if (note_index == 1) {
        return 329;
    } else {
        return 392;
    }
}

// 1. Dynamic Split-Raster Stripe Generator (modulates border lines based on SCSI bits)
static void update_raster_border(ocean_loader_ctx_t *ctx, uint8_t scsi_byte) {
    // If bit 0 is high, render CYAN stripe, else RED
    if (scsi_byte & 1) {
        ctx->border_color = COLOR_CYAN;
    } else {
        ctx->border_color = COLOR_RED;
    }
}

// 1.5. Asynchronous Uniservo Tape Channel Tick
static void tick_tape_channel(ocean_loader_ctx_t *ctx) {
    tape_channel_t *channel = &ctx->tape;
    if (!channel->transfer_active) return;

    channel->transfer_cycles++;
    if (channel->transfer_cycles >= 5) { // 1 byte every 5 cycles
        channel->transfer_cycles = 0;
        if (channel->read_ptr < channel->write_ptr) {
            uint8_t byte = channel->buffer[channel->read_ptr++];
            update_raster_border(ctx, byte);
            if (ctx->stack.top < STACK_CAPACITY) {
                ctx->stack.data[ctx->stack.top++] = byte;
            }
        } else {
            channel->transfer_active = false;
            ctx->psg_frequency = 330; // Success arpeggio tone E4
        }
    }
}


// 2. PSG Audio Failure Alarm Drone
static void set_psg_alarm(ocean_loader_ctx_t *ctx) {
    ctx->psg_frequency = WARNING_DRONE;
    printf("   [AUDIO] PSG Channel 1 output warning drone (%d Hz).\n", WARNING_DRONE);
}

// 3. SCSI Ingestion Checksum Blame & Revert (Bidirectional)
static bool ingest_scsi_block_bidirectional(ocean_loader_ctx_t *ctx, const uint8_t *data, size_t size, uint8_t expected_checksum, bool forward, uint32_t input_port) {
    // Save stack checkpoint before loading
    ctx->stack.checkpoint_top = ctx->stack.top;

    uint8_t calculated_checksum = 0;
    for (size_t i = 0; i < size; i++) {
        size_t idx = forward ? i : (size - 1 - i);
        uint8_t val = data[idx];
        calculated_checksum ^= val;

        // Dynamic split-raster striped border updates
        update_raster_border(ctx, val);

        // Push loaded bytes onto stack
        if (ctx->stack.top < STACK_CAPACITY) {
            ctx->stack.data[ctx->stack.top++] = val;
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

    bool success = ingest_scsi_block_bidirectional(&loader, valid_block, 4, valid_checksum, true, 5);
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

    success = ingest_scsi_block_bidirectional(&loader, corrupt_block, 4, bad_checksum, true, 5);
    assert(success == false);
    assert(loader.blame_quarantine == true);
    assert(loader.blamed_port == 5);
    assert(loader.stack.top == 6); // Rolled back to success state
    assert(loader.psg_frequency == WARNING_DRONE);
    printf("   ✓ Ingestion rollback, blame quarantine, and PSG audio warning verified.\n");
    fflush(stdout);

    // 3. Asynchronous Uniservo Tape Load Ingestion
    printf("[TEST] Loading data asynchronously via Uniservo tape channel emulation...\n");
    fflush(stdout);
    loader.tape.buffer[0] = 0xAA;
    loader.tape.buffer[1] = 0x55;
    loader.tape.write_ptr = 2;
    loader.tape.read_ptr = 0;
    loader.tape.transfer_active = true;
    loader.tape.transfer_cycles = 0;

    // Main execution loop: step cycles and tick tape channel
    int cycles = 0;
    while (loader.tape.transfer_active && cycles < 100) {
        tick_tape_channel(&loader);
        cycles++;
    }

    // Verify it loaded successfully in the background
    assert(loader.stack.top == 8);
    assert(loader.stack.data[6] == 0xAA);
    assert(loader.stack.data[7] == 0x55);
    assert(loader.tape.transfer_active == false);
    printf("   ✓ Uniservo background tape data load verified. Loaded: 0x%02X, 0x%02X. Cycles: %d\n",
           (uint8_t)loader.stack.data[6], (uint8_t)loader.stack.data[7], cycles);
    fflush(stdout);
    // 4. Backward Ingestion Success
    printf("[TEST] Ingesting valid SCSI block backward (expected pass)...\n");
    fflush(stdout);
    loader.blame_quarantine = false;
    success = ingest_scsi_block_bidirectional(&loader, valid_block, 4, valid_checksum, false, 5);
    assert(success == true);
    assert(loader.blame_quarantine == false);
    printf("   ✓ Backward ingestion and parity check passed.\n");
    fflush(stdout);

    // 5. Test Split-Raster Border and Chiptune Arpeggiator
    printf("[TEST] Testing sequential split-raster border and PSG audio arpeggio loops...\n");
    fflush(stdout);
    assert(cycle_border_raster(0) == COLOR_RED);
    assert(cycle_border_raster(1) == COLOR_CYAN);
    assert(cycle_border_raster(2) == COLOR_BLUE);
    assert(play_loader_arpeggio(0) == 261);
    assert(play_loader_arpeggio(1) == 329);
    assert(play_loader_arpeggio(2) == 392);
    printf("   ✓ Split-raster border colors and C-Major arpeggio frequencies verified.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("HUCOCEAN LOADER TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
