#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "lau_vram.h"

int main() {
    printf("=== TSFi CompuServe/Vidtex VRAM Escape Unit Test ===\n");

    LauVRAM vram;
    lau_vram_init(&vram);

    // Initial position
    vram.cursor_x = 10;
    vram.cursor_y = 10;

    // Test 1: ESC ESC d clears screen and resets cursor position
    printf("[COMPUSERVE-VRAM] Test 1: ESC ESC d clear screen...\n");
    lau_vram_write_char(&vram, '\x1b');
    lau_vram_write_char(&vram, '\x1b');
    lau_vram_write_char(&vram, 'd');
    assert(vram.cursor_x == 0);
    assert(vram.cursor_y == 0);
    printf("PASS\n");

    // Test 2: ESC ESC I <col+32> <row+32> positions cursor
    printf("[COMPUSERVE-VRAM] Test 2: ESC ESC I positioning...\n");
    lau_vram_write_char(&vram, '\x1b');
    lau_vram_write_char(&vram, '\x1b');
    lau_vram_write_char(&vram, 'I');
    lau_vram_write_char(&vram, 32 + 15);
    lau_vram_write_char(&vram, 32 + 8);
    assert(vram.cursor_x == 15);
    assert(vram.cursor_y == 8);
    printf("PASS\n");

    printf("=== ALL COMPUSERVE VRAM TESTS PASSED ===\n");
    return 0;
}
