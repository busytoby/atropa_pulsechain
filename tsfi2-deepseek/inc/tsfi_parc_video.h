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

#endif // TSFI_PARC_VIDEO_H
