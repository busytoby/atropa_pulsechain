// Wayland Markdown Editor graphics component helper routines
#include <stdint.h>
#include <string.h>

static const char *western_desert_art[6] = {
    "      #      .   .      ",
    "    #####    .   .      ",
    "      #    *       *    ",
    "    ###       ###       ",
    "   #####     #####      ",
    "========================"
};

static const uint16_t bubble_font_tsfi2[6][16] = {
    {0x3FFC, 0x7FFE, 0xFFFF, 0xE3C7, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x0180},
    {0x3FFC, 0x7FFE, 0xC003, 0xC000, 0x7FE0, 0x3FF8, 0x01FC, 0x007E, 0x000F, 0xC007, 0xC003, 0xE007, 0x7FFE, 0x7FFE, 0x3FFC, 0x0000},
    {0xFFFF, 0xFFFF, 0xC000, 0xC000, 0xFFF0, 0xFFF0, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0x0000},
    {0x0180, 0x03C0, 0x03C0, 0x0180, 0x0000, 0x0000, 0x0180, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x0180},
    {0x000F, 0x001F, 0x003E, 0x007C, 0x00F8, 0x01F0, 0x03E0, 0x07C0, 0x0F80, 0x1F00, 0x3E00, 0x7C00, 0xF800, 0xF000, 0xE000, 0x0000},
    {0x3FFC, 0x7FFE, 0xC007, 0x000F, 0x001E, 0x003C, 0x0078, 0x00F0, 0x01E0, 0x03C0, 0x0780, 0x0F00, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000}
};

static void precompute_morph_cache(void) {
    for (int char_idx = 0; char_idx < 6; char_idx++) {
        for (int step = 0; step < 5; step++) {
            MorphFrame *mf = &morph_cache[char_idx][step];
            for (int r = 0; r < 16; r++) {
                uint16_t curr = bubble_font_tsfi2[char_idx][r];
                uint16_t prev = (r > 0) ? bubble_font_tsfi2[char_idx][r - 1] : 0;
                uint16_t next = (r < 15) ? bubble_font_tsfi2[char_idx][r + 1] : 0;
                
                uint16_t shift_l = curr | (curr << (step >= 3 ? 2 : 1));
                uint16_t shift_r = curr | (curr >> (step >= 3 ? 2 : 1));
                mf->dilated[r] = shift_l | shift_r | prev | next;
                
                uint16_t eshift_l = curr & (curr << (step <= 1 ? 2 : 1));
                uint16_t eshift_r = curr & (curr >> (step <= 1 ? 2 : 1));
                mf->eroded[r] = eshift_l & eshift_r & prev & next;
            }
            
            for (int r = 0; r < 16; r++) {
                uint16_t curr_dilated = mf->dilated[r];
                uint16_t prev_dilated = (r > 0) ? mf->dilated[r - 1] : 0;
                
                mf->glints[r] = (curr_dilated & ~prev_dilated) & ~(curr_dilated << 1);
            }
        }
    }
}

static void draw_char(uint32_t *pixels, int w, int h, int start_x, int start_y, uint8_t ch, uint32_t color, int scale) {
    if (ch >= 128) return;
    
    uint8_t ecm_mode = (ch >= 'a' && ch <= 'z') ? 1 : 0;
    uint32_t ecm_bg_colors[4] = {0x00000000, 0xFF3C0E00, 0xFF251000, 0xFF051224};
    uint32_t bg_color = ecm_bg_colors[ecm_mode];
    
    int scale_x = scale;
    bool is_multicolor = (start_y >= 200);
    if (is_multicolor) {
        scale_x = scale * 2;
    }
    
    if (bg_color != 0x00000000) {
        for (int r = 0; r < 7 * scale; r++) {
            for (int c = 0; c < 5 * scale_x; c++) {
                int px = start_x + c;
                int py = start_y + r;
                if (px >= 0 && px < w && py >= 0 && py < h) {
                    pixels[py * w + px] = bg_color;
                }
            }
        }
    }
    
    const uint8_t *glyph = font5x7[(int)ch];
    for (int col = 0; col < 5; col++) {
        uint8_t byte = glyph[col];
        for (int row = 0; row < 7; row++) {
            if (byte & (1 << row)) {
                for (int sy = 0; sy < scale; sy++) {
                    for (int sx = 0; sx < scale_x; sx++) {
                        int px = start_x + col * scale_x + sx;
                        int py = start_y + row * scale + sy;
                        if (px >= 0 && px < w && py >= 0 && py < h) {
                            pixels[py * w + px] = color;
                        }
                    }
                }
            }
        }
    }
}

static void draw_string(uint32_t *pixels, int w, int h, int start_x, int start_y, const char *str, uint32_t color, int scale) {
    int cur_x = start_x;
    int char_width = (start_y >= 200) ? (6 * scale * 2) : (6 * scale);
    while (*str) {
        draw_char(pixels, w, h, cur_x, start_y, *str, color, scale);
        cur_x += char_width;
        str++;
    }
}
