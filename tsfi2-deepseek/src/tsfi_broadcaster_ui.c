#include "tsfi_broadcaster.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief Helper to write a string directly to the char_BASE.
 */
static void BASE_puts(LauBroadcaster *lb, int y, int x, const char *str) {
    if (y < 0 || y >= BROADCASTER_ROWS) return;
    int len = (int)strlen(str);
    for (int i = 0; i < len; i++) {
        int cx = x + i;
        if (cx >= 0 && cx < BROADCASTER_COLS) {
            uint32_t cp = (uint32_t)str[i];
            if (lb->char_BASE[y][cx] != cp) {
                lb->char_BASE[y][cx] = cp;
                tsfi_broadcaster_dirty_char(lb, cx, y);
            }
        }
    }
}

/**
 * @brief Rasterize Telemetry Overlay.
 */
void tsfi_broadcaster_ui_draw(LauBroadcaster *lb) {
    char buf[32];

    // Overlay positioned at top-right
    int start_x = BROADCASTER_COLS - 20;

    // 1. FPS
    snprintf(buf, sizeof(buf), "FPS: %.1f", lb->current_fps);
    BASE_puts(lb, 0, start_x, buf);

    // 2. Latency
    snprintf(buf, sizeof(buf), "LAT: %.1fms", lb->current_latency_ms);
    BASE_puts(lb, 1, start_x, buf);

    // 3. Reconnects
    snprintf(buf, sizeof(buf), "RCN: %d", lb->reconnect_count);
    BASE_puts(lb, 2, start_x, buf);

    // 4. CPU Load
    snprintf(buf, sizeof(buf), "CPU: %.1f%%", lb->cpu_load);
    BASE_puts(lb, 3, start_x, buf);

    // 5. GPU Load
    snprintf(buf, sizeof(buf), "GPU: %.1f%%", lb->gpu_load);
    BASE_puts(lb, 4, start_x, buf);
}
