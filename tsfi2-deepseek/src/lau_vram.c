#include "lau_vram.h"
#include <string.h>

void lau_vram_init(LauVRAM *vram) {
    if (!vram) return;
    memset(vram, 0, sizeof(LauVRAM));
    vram->current_fg = 7; // White
    vram->current_bg = 0; // Black
    
    for (int y = 0; y < LAU_VRAM_ROWS; y++) {
        for (int x = 0; x < LAU_VRAM_COLS; x++) {
            vram->grid[y][x].character = ' ';
            vram->grid[y][x].fg_color = vram->current_fg;
            vram->grid[y][x].bg_color = vram->current_bg;
            vram->grid[y][x].attributes = 0;
            vram->dirty_grid[y][x] = true;
        }
    }
    vram->is_dirty = true;
}

static void vram_scroll_up(LauVRAM *vram) {
    // Scroll entire buffer up by 1 line
    for (int y = 0; y < LAU_VRAM_ROWS - 1; y++) {
        memcpy(vram->grid[y], vram->grid[y + 1], LAU_VRAM_COLS * sizeof(LauVRAMCell));
        for (int x = 0; x < LAU_VRAM_COLS; x++) vram->dirty_grid[y][x] = true;
    }
    
    // Clear last line
    for (int x = 0; x < LAU_VRAM_COLS; x++) {
        vram->grid[LAU_VRAM_ROWS - 1][x].character = ' ';
        vram->grid[LAU_VRAM_ROWS - 1][x].fg_color = vram->current_fg;
        vram->grid[LAU_VRAM_ROWS - 1][x].bg_color = vram->current_bg;
        vram->grid[LAU_VRAM_ROWS - 1][x].attributes = 0;
        vram->dirty_grid[LAU_VRAM_ROWS - 1][x] = true;
    }
    vram->is_dirty = true;
}

static void vram_put_char(LauVRAM *vram, char c) {
    if (vram->cursor_x >= LAU_VRAM_COLS) {
        vram->cursor_x = 0;
        vram->cursor_y++;
    }
    if (vram->cursor_y >= LAU_VRAM_ROWS) {
        vram_scroll_up(vram);
        vram->cursor_y = LAU_VRAM_ROWS - 1;
    }
    
    LauVRAMCell *cell = &vram->grid[vram->cursor_y][vram->cursor_x];
    if (cell->character != (uint8_t)c || cell->fg_color != vram->current_fg || cell->bg_color != vram->current_bg || cell->attributes != vram->current_attr) {
        cell->character = (uint8_t)c;
        cell->fg_color = vram->current_fg;
        cell->bg_color = vram->current_bg;
        cell->attributes = vram->current_attr;
        vram->dirty_grid[vram->cursor_y][vram->cursor_x] = true;
        vram->is_dirty = true;
    }
    vram->cursor_x++;
}

static void process_ansi_sgr(LauVRAM *vram) {
    if (vram->ansi_param_count == 0) {
        // Reset
        vram->current_fg = 7;
        vram->current_bg = 0;
        vram->current_attr = 0;
        return;
    }
    
    for (int i = 0; i < vram->ansi_param_count; i++) {
        int code = vram->ansi_params[i];
        if (code == 0) {
            vram->current_fg = 7;
            vram->current_bg = 0;
            vram->current_attr = 0;
        } else if (code == 1) {
            vram->current_attr |= 1; // Bold
        } else if (code >= 30 && code <= 37) {
            vram->current_fg = code - 30; // Basic fg
        } else if (code >= 40 && code <= 47) {
            vram->current_bg = code - 40; // Basic bg
        } else if (code >= 90 && code <= 97) {
            vram->current_fg = code - 90 + 8; // Bright fg
        } else if (code >= 100 && code <= 107) {
            vram->current_bg = code - 100 + 8; // Bright bg
        }
    }
}

void lau_vram_write_char(LauVRAM *vram, char c) {
    if (vram->ansi_state == 0) {
        if (c == '\x1b') {
            vram->ansi_state = 1;
        } else if (c == '\r') {
            vram->cursor_x = 0;
        } else if (c == '\n') {
            vram->cursor_y++;
            if (vram->cursor_y >= LAU_VRAM_ROWS) {
                vram_scroll_up(vram);
                vram->cursor_y = LAU_VRAM_ROWS - 1;
            }
        } else if (c == '\b') {
            if (vram->cursor_x > 0) vram->cursor_x--;
        } else if (c >= 32 || c < 0) {
            vram_put_char(vram, c);
        }
    } else if (vram->ansi_state == 1) {
        if (c == '[') {
            vram->ansi_state = 2;
            memset(vram->ansi_params, 0, sizeof(vram->ansi_params));
            vram->ansi_param_count = 0;
        } else {
            // Unhandled Esc sequence
            vram->ansi_state = 0;
        }
    } else if (vram->ansi_state == 2) {
        if (c >= '0' && c <= '9') {
            if (vram->ansi_param_count == 0) vram->ansi_param_count = 1;
            vram->ansi_params[vram->ansi_param_count - 1] = vram->ansi_params[vram->ansi_param_count - 1] * 10 + (c - '0');
        } else if (c == ';') {
            if (vram->ansi_param_count < 8) vram->ansi_param_count++;
        } else {
            // Final byte
            if (c == 'm') {
                process_ansi_sgr(vram);
            } else if (c == 'A') { // Cursor up
                int n = (vram->ansi_param_count > 0 && vram->ansi_params[0] > 0) ? vram->ansi_params[0] : 1;
                vram->cursor_y -= n;
                if (vram->cursor_y < 0) vram->cursor_y = 0;
            } else if (c == 'B') { // Cursor down
                int n = (vram->ansi_param_count > 0 && vram->ansi_params[0] > 0) ? vram->ansi_params[0] : 1;
                vram->cursor_y += n;
                if (vram->cursor_y >= LAU_VRAM_ROWS) vram->cursor_y = LAU_VRAM_ROWS - 1;
            } else if (c == 'C') { // Cursor right
                int n = (vram->ansi_param_count > 0 && vram->ansi_params[0] > 0) ? vram->ansi_params[0] : 1;
                vram->cursor_x += n;
                if (vram->cursor_x >= LAU_VRAM_COLS) vram->cursor_x = LAU_VRAM_COLS - 1;
            } else if (c == 'D') { // Cursor left
                int n = (vram->ansi_param_count > 0 && vram->ansi_params[0] > 0) ? vram->ansi_params[0] : 1;
                vram->cursor_x -= n;
                if (vram->cursor_x < 0) vram->cursor_x = 0;
            } else if (c == 'H' || c == 'f') { // Cursor home
                int y = (vram->ansi_param_count > 0 && vram->ansi_params[0] > 0) ? vram->ansi_params[0] - 1 : 0;
                int x = (vram->ansi_param_count > 1 && vram->ansi_params[1] > 0) ? vram->ansi_params[1] - 1 : 0;
                vram->cursor_y = y; vram->cursor_x = x;
                if (vram->cursor_y >= LAU_VRAM_ROWS) vram->cursor_y = LAU_VRAM_ROWS - 1;
                if (vram->cursor_x >= LAU_VRAM_COLS) vram->cursor_x = LAU_VRAM_COLS - 1;
            } else if (c == 'J') { // Clear screen
                int n = (vram->ansi_param_count > 0) ? vram->ansi_params[0] : 0;
                if (n == 2 || n == 3) { // Clear entire screen
                    for (int y = 0; y < LAU_VRAM_ROWS; y++) {
                        for (int x = 0; x < LAU_VRAM_COLS; x++) {
                            vram->grid[y][x].character = ' ';
                            vram->grid[y][x].fg_color = vram->current_fg;
                            vram->grid[y][x].bg_color = vram->current_bg;
                            vram->grid[y][x].attributes = 0;
                            vram->dirty_grid[y][x] = true;
                        }
                    }
                    if (n == 2) { vram->cursor_x = 0; vram->cursor_y = 0; }
                    vram->is_dirty = true;
                }
            } else if (c == 'K') { // Clear line
                int n = (vram->ansi_param_count > 0) ? vram->ansi_params[0] : 0;
                if (n == 0) { // Cursor to end
                    for (int x = vram->cursor_x; x < LAU_VRAM_COLS; x++) {
                        vram->grid[vram->cursor_y][x].character = ' ';
                        vram->grid[vram->cursor_y][x].fg_color = vram->current_fg;
                        vram->grid[vram->cursor_y][x].bg_color = vram->current_bg;
                        vram->grid[vram->cursor_y][x].attributes = 0;
                        vram->dirty_grid[vram->cursor_y][x] = true;
                    }
                    vram->is_dirty = true;
                }
            }
            vram->ansi_state = 0;
        }
    }
}

void lau_vram_write_string(LauVRAM *vram, const char *str, size_t len) {
    if (!vram || !str) return;
    for (size_t i = 0; i < len; i++) {
        lau_vram_write_char(vram, str[i]);
    }
}