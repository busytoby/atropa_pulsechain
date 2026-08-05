#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_window.h"
#include "tsfi2-deepseek/inc/tsfi_parc_window.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_current_page = 0;
static int g_total_pages = 0;

bool hathitrust_open_reader_window(const char *book_title, int total_pages) {
    if (!book_title || total_pages <= 0) return false;
    
    tsfi_parc_window_manager_t wm;
    if (tsfi_parc_wm_init(&wm) != 0) {
        return false;
    }
    
    // Open dynamic reader window inside virtual screen coordinates
    char title_buf[128];
    snprintf(title_buf, sizeof(title_buf), "Auncient Reader: %s", book_title);
    if (tsfi_parc_wm_open_window(&wm, title_buf, 50, 50, 400, 300) != 0) {
        return false;
    }
    
    g_current_page = 1;
    g_total_pages = total_pages;
    
    // Allocate virtual framebuffer for rendering
    uint32_t *framebuffer = calloc(640 * 480, sizeof(uint32_t));
    if (!framebuffer) return false;
    
    // Render standard Alto desktop environment to pixel buffer
    tsfi_parc_wm_render(&wm, framebuffer, 640, 480, 1.0f, 0, 0, 0, 0);
    free(framebuffer);
    
    return true;
}

int hathitrust_process_scsi_key_event(uint32_t keycode) {
    // keycode 32 -> d/D (forward page turn)
    // keycode 30 -> a/A (backward page turn)
    if (keycode == 32) {
        if (g_current_page < g_total_pages) {
            g_current_page++;
        }
        return 1;
    } else if (keycode == 30) {
        if (g_current_page > 1) {
            g_current_page--;
        }
        return -1;
    }
    return 0;
}
