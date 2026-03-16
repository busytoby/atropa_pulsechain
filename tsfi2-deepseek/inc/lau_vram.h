#ifndef LAU_VRAM_H
#define LAU_VRAM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define LAU_VRAM_COLS 160
#define LAU_VRAM_ROWS 60

// 32-bit cell structure
// Bits 0-7: ASCII/UTF-8 character
// Bits 8-15: Foreground color (ANSI 8-bit or simple index)
// Bits 16-23: Background color
// Bits 24-31: Attributes (Bold, Underline, Blink, Inverse)
typedef struct {
    uint8_t character;
    uint8_t fg_color;
    uint8_t bg_color;
    uint8_t attributes;
} LauVRAMCell;

typedef struct {
    LauVRAMCell grid[LAU_VRAM_ROWS][LAU_VRAM_COLS];
    bool dirty_grid[LAU_VRAM_ROWS][LAU_VRAM_COLS];
    
    int cursor_x;
    int cursor_y;
    bool is_dirty;
    
    // ANSI State Machine 
    int ansi_state; // 0=Normal, 1=Esc, 2=CSI
    int ansi_params[8];
    int ansi_param_count;
    
    // Current formatting
    uint8_t current_fg;
    uint8_t current_bg;
    uint8_t current_attr;
    
    // Saved cursor
    int saved_x;
    int saved_y;
} LauVRAM;

void lau_vram_init(LauVRAM *vram);
void lau_vram_write_char(LauVRAM *vram, char c);
void lau_vram_write_string(LauVRAM *vram, const char *str, size_t len);

#endif // LAU_VRAM_H
