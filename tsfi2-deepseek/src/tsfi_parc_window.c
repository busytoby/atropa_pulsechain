#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "tsfi_parc_window.h"
#include "tsfi_parc_bitblt.h"
#include "tsfi_paint.h"
#include "tsfi_cade_imf.h"

int tsfi_parc_wm_init(tsfi_parc_window_manager_t *wm) {
    if (!wm) return -1;
    wm->window_count = 0;
    wm->active_win_id = -1;
    wm->show_popup_menu = 0;
    wm->popup_x = 0;
    wm->popup_y = 0;
    return 0;
}

int tsfi_parc_wm_open_window(tsfi_parc_window_manager_t *wm, const char *title, int x, int y, int w, int h) {
    if (!wm || wm->window_count >= MAX_WINDOWS) return -1;
    int idx = wm->window_count++;
    st_window_t *win = &wm->windows[idx];
    win->id = idx;
    win->x = x;
    win->y = y;
    win->w = w;
    win->h = h;
    strncpy(win->title, title, sizeof(win->title) - 1);
    win->is_active = (idx == 0) ? 1 : 0;
    win->scroll_offset = 0;
    wm->active_win_id = idx;
    return idx;
}

int tsfi_parc_wm_render(tsfi_parc_window_manager_t *wm, uint32_t *pixels, int width, int height, float t, uint32_t st_result, int pup_len, uint16_t pup_checksum, int tx_count) {
    if (!wm || !pixels || width <= 0 || height <= 0) return -1;
    (void)tx_count;

    // 1. Alto Halftone Desktop Grid Background removed to prevent obscuring the Menorah trunk

    // 2. Draw Retro Desktop Icons: Filing Cabinet & Floppy Disk
    // Draw filing cabinet icon at left
    int f_cx = 45;
    int f_cy = 405;
    for (int dy = 0; dy < 15; dy++) {
        for (int dx = 0; dx < 14; dx++) {
            int px = f_cx + dx;
            int py = f_cy + dy;
            if (px >= 0 && px < width && py >= 0 && py < height) {
                if (dy == 0 || dy == 14 || dx == 0 || dx == 13 || dy == 5 || dy == 10) {
                    pixels[py * width + px] = 0xFFc5a059; // Gold outline
                }
            }
        }
    }
    tsfi_quantel_paintbox_typographer(pixels, width, height, f_cx + 1, f_cy + 18, "CABINET", 0xFFe6dfd3, 5.5f);

    // 3. Draw Overlapping Windows
    for (int w_idx = 0; w_idx < wm->window_count; w_idx++) {
        st_window_t *win = &wm->windows[w_idx];

        // Draw shadow (Xerox retro style: solid dark offset)
        for (int y = win->y + 4; y < win->y + win->h + 4; y++) {
            for (int x = win->x + 4; x < win->x + win->w + 4; x++) {
                if (x >= 35 && x < width - 35 && y >= 398 && y < 508) {
                    pixels[y * width + x] = 0xFF050403; // Shadow color
                }
            }
        }

        // Draw Window Body background
        for (int y = win->y; y < win->y + win->h; y++) {
            for (int x = win->x; x < win->x + win->w; x++) {
                if (x >= 35 && x < width - 35 && y >= 398 && y < 508) {
                    pixels[y * width + x] = 0xFF1c1815; // Dark cream/parchment window body
                }
            }
        }

        // Draw Double Window Border outline for high fidelity
        for (int offset = 0; offset < 2; offset++) {
            for (int x = win->x + offset; x < win->x + win->w - offset; x++) {
                if (x >= 35 && x < width - 35) {
                    if (win->y + offset >= 398 && win->y + offset < 508) pixels[(win->y + offset) * width + x] = 0xFF8c7241;
                    if (win->y + win->h - 1 - offset >= 398 && win->y + win->h - 1 - offset < 508) pixels[(win->y + win->h - 1 - offset) * width + x] = 0xFF8c7241;
                }
            }
            for (int y = win->y + offset; y < win->y + win->h - offset; y++) {
                if (y >= 398 && y < 508) {
                    if (win->x + offset >= 35 && win->x + offset < width - 35) pixels[y * width + (win->x + offset)] = 0xFF8c7241;
                    if (win->x + win->w - 1 - offset >= 35 && win->x + win->w - 1 - offset < width - 35) pixels[y * width + (win->x + win->w - 1 - offset)] = 0xFF8c7241;
                }
            }
        }

        // Draw Title Bar (Filled solid if active window)
        int title_h = 12;
        uint32_t title_bg = win->is_active ? 0xFF8c7241 : 0xFF12100e;
        for (int y = win->y + 1; y < win->y + title_h; y++) {
            for (int x = win->x + 1; x < win->x + win->w - 1; x++) {
                if (x >= 35 && x < width - 35 && y >= 398 && y < 508) {
                    pixels[y * width + x] = title_bg;
                }
            }
        }
        tsfi_quantel_paintbox_typographer(pixels, width, height, win->x + 5, win->y + 8, win->title, 0xFFe6dfd3, 6.5f);

        // Window Control Handle box on top right
        int cb_x = win->x + win->w - 11;
        int cb_y = win->y + 2;
        for (int dy = 0; dy < 8; dy++) {
            for (int dx = 0; dx < 8; dx++) {
                int px = cb_x + dx;
                int py = cb_y + dy;
                if (px >= 35 && px < width - 35 && py >= 398 && py < 508) {
                    if (dy == 0 || dy == 7 || dx == 0 || dx == 7 || dy == dx || dy == 7 - dx) {
                        pixels[py * width + px] = 0xFFe6dfd3;
                    }
                }
            }
        }

        // Draw Xerox classic left scrollbar strip
        int sb_w = 8;
        for (int y = win->y + title_h; y < win->y + win->h - 1; y++) {
            for (int x = win->x + 1; x < win->x + sb_w; x++) {
                if (x >= 35 && x < width - 35 && y >= 398 && y < 508) {
                    if ((x + y) % 2 == 0) pixels[y * width + x] = 0xFF12100e;
                }
            }
        }
        // Scrollbar active handle elevator slider
        int slider_y = win->y + title_h + 5 + (int)(10.0f * (1.0f + sinf(t)));
        for (int y = slider_y; y < slider_y + 8; y++) {
            for (int x = win->x + 1; x < win->x + sb_w; x++) {
                if (x >= 35 && x < width - 35 && y >= 398 && y < 508) {
                    pixels[y * width + x] = 0xFFc5a059; // Gold slider handle
                }
            }
        }

        // Draw custom body text based on Window ID
        char text_buf[128];
        if (win->id == 0) { // CADE IMF Register & BFS Directory
            tsfi_quantel_paintbox_typographer(pixels, width, height, win->x + 12, win->y + 18, "BFS DB: VOL_01", 0xFFc5a059, 5.5f);
            snprintf(text_buf, sizeof(text_buf), "CYL 100 HD 1 SEC %d", (int)(t * 2) % 12);
            tsfi_quantel_paintbox_typographer(pixels, width, height, win->x + 12, win->y + 28, text_buf, 0xFFc5a059, 5.5f);
            snprintf(text_buf, sizeof(text_buf), "DATA CHK: 0x%04X", (uint16_t)(0xFD4A + (int)t));
            tsfi_quantel_paintbox_typographer(pixels, width, height, win->x + 12, win->y + 38, text_buf, 0xFFc5a059, 5.5f);
        } else if (win->id == 1) { // Smalltalk VM
            tsfi_quantel_paintbox_typographer(pixels, width, height, win->x + 12, win->y + 18, "CLASS: TigerSensor", 0xFFc5a059, 5.5f);
            tsfi_quantel_paintbox_typographer(pixels, width, height, win->x + 12, win->y + 28, "MSG: #processData", 0xFFc5a059, 5.5f);
            snprintf(text_buf, sizeof(text_buf), "EVAL: %u (STACK OK)", st_result);
            tsfi_quantel_paintbox_typographer(pixels, width, height, win->x + 12, win->y + 38, text_buf, 0xFFc5a059, 5.5f);
        } else if (win->id == 2) { // PUP Netlog & Metcalfe CSMA/CD Simulation
            if ((int)(t * 2) % 4 == 0) {
                tsfi_quantel_paintbox_typographer(pixels, width, height, win->x + 12, win->y + 18, "NET: COLLISION ACTIVE", 0xFFc5a059, 5.5f);
                snprintf(text_buf, sizeof(text_buf), "BACKOFF: %d SLOTS", 1 << ((int)(t * 2.5f) % 6));
                tsfi_quantel_paintbox_typographer(pixels, width, height, win->x + 12, win->y + 28, text_buf, 0xFFc5a059, 5.5f);
            } else {
                tsfi_quantel_paintbox_typographer(pixels, width, height, win->x + 12, win->y + 18, "NET: PUP TRANSMIT", 0xFFc5a059, 5.5f);
                snprintf(text_buf, sizeof(text_buf), "PACKET LEN: %d B", pup_len);
                tsfi_quantel_paintbox_typographer(pixels, width, height, win->x + 12, win->y + 28, text_buf, 0xFFc5a059, 5.5f);
            }
            snprintf(text_buf, sizeof(text_buf), "CHECKSUM: 0x%04X", pup_checksum);
            tsfi_quantel_paintbox_typographer(pixels, width, height, win->x + 12, win->y + 38, text_buf, 0xFFc5a059, 5.5f);
        }
    }

    // 4. Pop-up context menu (classic Xerox right-click select)
    if (wm->show_popup_menu) {
        int m_x = wm->popup_x;
        int m_y = wm->popup_y;
        int m_w = 70;
        int m_h = 24;

        for (int y = m_y; y < m_y + m_h; y++) {
            for (int x = m_x; x < m_x + m_w; x++) {
                if (x >= 35 && x < width - 35 && y >= 398 && y < 508) {
                    pixels[y * width + x] = 0xFFe6dfd3;
                }
            }
        }
        for (int x = m_x; x < m_x + m_w; x++) {
            if (x >= 35 && x < width - 35) {
                if (m_y >= 398 && m_y < 508) pixels[m_y * width + x] = 0xFF12100e;
                if (m_y + m_h - 1 >= 398 && m_y + m_h - 1 < 508) pixels[(m_y + m_h - 1) * width + x] = 0xFF12100e;
            }
        }
        tsfi_quantel_paintbox_typographer(pixels, width, height, m_x + 4, m_y + 8, "Inspect Obj", 0xFF12100e, 6.0f);
        tsfi_quantel_paintbox_typographer(pixels, width, height, m_x + 4, m_y + 18, "Send Message", 0xFF12100e, 6.0f);
    }

    return 0;
}
