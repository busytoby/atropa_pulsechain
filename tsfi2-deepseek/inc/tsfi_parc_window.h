#ifndef TSFI_PARC_WINDOW_H
#define TSFI_PARC_WINDOW_H

#include <stdint.h>

#define MAX_WINDOWS 8

typedef struct {
    int id;
    int x;
    int y;
    int w;
    int h;
    char title[64];
    int is_active;
    int scroll_offset;
} st_window_t;

typedef struct {
    st_window_t windows[MAX_WINDOWS];
    int window_count;
    int active_win_id;
    int show_popup_menu;
    int popup_x;
    int popup_y;
} tsfi_parc_window_manager_t;

/* Initialize window manager with standard Alto defaults */
int tsfi_parc_wm_init(tsfi_parc_window_manager_t *wm);

/* Create/open an overlapping window */
int tsfi_parc_wm_open_window(tsfi_parc_window_manager_t *wm, const char *title, int x, int y, int w, int h);

/* Render Alto desktop, icons, overlapping windows, scrollbars, and menus to pixel buffer */
int tsfi_parc_wm_render(tsfi_parc_window_manager_t *wm, uint32_t *pixels, int width, int height, float t, uint32_t st_result, int pup_len, uint16_t pup_checksum, int tx_count);

#endif // TSFI_PARC_WINDOW_H
