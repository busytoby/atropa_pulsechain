#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    tsfi_io_printf(stdout, "=== TSFi Native IO: Vulkan ReBAR Visual Grep Validation ===\n");

    LauVRAM* vram = (LauVRAM*)lau_malloc_wired(sizeof(LauVRAM));
    lau_vram_init(vram);

    for (int r = 0; r < LAU_VRAM_ROWS; r++) {
        for (int c = 0; c < LAU_VRAM_COLS; c++) {
            vram->grid[r][c].character = 'A' + (rand() % 26);
            vram->grid[r][c].fg_color = rand() % 255;
            vram->grid[r][c].bg_color = rand() % 255;
            vram->grid[r][c].attributes = rand() % 255;
        }
    }

    const char* pattern = "COCKPIT_LOCKED";
    size_t pat_len = strlen(pattern);
    int target_row = 15;
    int target_col = 42;

    for (size_t i = 0; i < pat_len; i++) {
        vram->grid[target_row][target_col + i].character = pattern[i];
        vram->grid[target_row][target_col + i].fg_color = 0xFF; 
        vram->grid[target_row][target_col + i].attributes = 0xAA;
    }

    size_t match_r[10];
    size_t match_c[10];

    size_t match_count = tsfi_vram_grep(vram, pattern, pat_len, match_r, match_c, 10);

    tsfi_io_printf(stdout, "Found %zu visual matches in 32-bit ReBAR structure.\n", match_count);

    if (match_count == 1 && match_r[0] == (size_t)target_row && match_c[0] == (size_t)target_col) {
        tsfi_io_printf(stdout, "  -> Successfully bypassed ANSI/Color attributes to locate: '%s' at Row %zu, Col %zu\n", pattern, match_r[0], match_c[0]);
        tsfi_io_printf(stdout, "[SUCCESS] ReBAR Native Sweeper is fully operational.\n");
    } else {
        tsfi_io_printf(stderr, "[FAIL] Visual grep failed to isolate the pattern in the 2D grid.\n");
        lau_free(vram);
        return 1;
    }

    lau_free(vram);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}