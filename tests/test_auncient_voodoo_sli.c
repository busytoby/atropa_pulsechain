#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#define CARD_COUNT 2
#define FRAME_HEIGHT 8

typedef struct {
    uint32_t card_id;
    uint32_t lines_processed;
} voodoo_card_t;

typedef struct {
    voodoo_card_t cards[CARD_COUNT];
    uint32_t total_routed_pixels;
} voodoo_sli_pipeline_t;

// -------------------------------------------------------------
// Voodoo Scan-Line Interleaving (SLI) Workload Router
// -------------------------------------------------------------
uint32_t route_sli_scanline(voodoo_sli_pipeline_t *pipeline, uint32_t y) {
    // 3Dfx Voodoo SLI Best Practice:
    // Route even scanlines (y % 2 == 0) to Card 0, and odd scanlines to Card 1
    uint32_t target_card = y % CARD_COUNT;
    
    pipeline->cards[target_card].lines_processed++;
    pipeline->total_routed_pixels++;

    // Return the card ID that processed this line
    return target_card;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT VOODOO DUAL-CARD SLI ROUTING SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    voodoo_sli_pipeline_t pipeline = {
        .cards = {
            { .card_id = 0, .lines_processed = 0 },
            { .card_id = 1, .lines_processed = 0 }
        },
        .total_routed_pixels = 0
    };

    printf("[TEST] Routing scanline pixel writes across the SLI pipeline...\n");
    fflush(stdout);

    // Route pixels for scanlines 0 through 7
    for (uint32_t y = 0; y < FRAME_HEIGHT; y++) {
        uint32_t card_id = route_sli_scanline(&pipeline, y);
        
        // Assert that even scanlines go to Card 0, and odd scanlines go to Card 1
        assert(card_id == (y % 2));
    }

    // Verify workload distribution
    printf("   Card 0 (Even Scanlines) processed: %u lines\n", pipeline.cards[0].lines_processed);
    printf("   Card 1 (Odd Scanlines) processed:  %u lines\n", pipeline.cards[1].lines_processed);
    fflush(stdout);

    assert(pipeline.cards[0].lines_processed == 4);
    assert(pipeline.cards[1].lines_processed == 4);
    assert(pipeline.total_routed_pixels == 8);

    printf("   ✓ Dual-card workload balance verified successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("VOODOO SLI ROUTING TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
