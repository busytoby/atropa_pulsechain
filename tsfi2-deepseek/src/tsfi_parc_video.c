#include "tsfi_parc_video.h"

int tsfi_parc_video_init(tsfi_parc_video_controller_t *vc) {
    if (!vc) return -1;
    vc->cursor_x = 0;
    vc->cursor_y = 0;
    vc->scan_line = 0;
    vc->vertical_interrupt_triggered = 0;
    return 0;
}

int tsfi_parc_video_step(tsfi_parc_video_controller_t *vc, int mx, int my) {
    if (!vc) return -1;

    // Simulate vertical scan lines up to classic Alto portrait resolution (808 lines)
    vc->scan_line = (vc->scan_line + 1) % 808;
    
    // Trigger vertical sync interrupt at start of blanking period (line 800)
    if (vc->scan_line == 800) {
        vc->vertical_interrupt_triggered = 1;
    } else {
        vc->vertical_interrupt_triggered = 0;
    }

    // Hardware cursor registers tracking (clamped to 606x808 resolution bounds)
    vc->cursor_x = (uint16_t)(mx < 0 ? 0 : (mx > 606 ? 606 : mx));
    vc->cursor_y = (uint16_t)(my < 0 ? 0 : (my > 808 ? 808 : my));
    return 0;
}
