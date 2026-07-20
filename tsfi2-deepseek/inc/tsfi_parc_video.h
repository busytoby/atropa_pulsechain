#ifndef TSFI_PARC_VIDEO_H
#define TSFI_PARC_VIDEO_H

#include <stdint.h>

typedef struct {
    uint16_t cursor_x;
    uint16_t cursor_y;
    uint16_t scan_line;
    int vertical_interrupt_triggered;
} tsfi_parc_video_controller_t;

/* Initialize Alto video scan controller state */
int tsfi_parc_video_init(tsfi_parc_video_controller_t *vc);

/* Process video controller scan lines and update cursor register tracking coordinates */
int tsfi_parc_video_step(tsfi_parc_video_controller_t *vc, int mx, int my);

/* Render the 1-bit Alto screen buffer into a 32-bit color destination canvas */
int tsfi_parc_video_render_frame(
    const tsfi_parc_video_controller_t *vc,
    const uint16_t *display_mem, // 38 words per line * 808 lines
    uint32_t *dest_pixels,
    int dest_w, int dest_h,
    int offset_x, int offset_y,
    uint32_t phosphor_color,
    uint32_t cursor_color,
    const uint16_t *cursor_bitmap
);

#endif // TSFI_PARC_VIDEO_H
