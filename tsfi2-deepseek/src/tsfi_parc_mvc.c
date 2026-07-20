#include "tsfi_parc_mvc.h"

void tsfi_parc_mvc_init(tsfi_parc_mvc_dispatcher_t *mvc, tsfi_parc_window_manager_t *wm, tsfi_parc_smalltalk_vm_t *vm) {
    if (!mvc) return;
    mvc->wm = wm;
    mvc->vm = vm;
    mvc->dispatched_count = 0;
}

int tsfi_parc_mvc_dispatch(tsfi_parc_mvc_dispatcher_t *mvc, const tsfi_parc_mvc_event_t *event) {
    if (!mvc || !event || !mvc->wm) return -1;

    mvc->dispatched_count++;

    if (event->type == MVC_EVENT_CLICK) {
        // Find which window layout bounds contain click coordinate to switch active controller view
        for (int i = 0; i < mvc->wm->window_count; i++) {
            st_window_t *win = &mvc->wm->windows[i];
            if (event->x >= win->x && event->x < win->x + win->w &&
                event->y >= win->y && event->y < win->y + win->h) {
                mvc->wm->active_win_id = win->id;
                win->is_active = 1;
            } else {
                win->is_active = 0;
            }
        }
    } else if (event->type == MVC_EVENT_KEY && mvc->vm) {
        // Route character key inputs directly to class method stack models
        mvc->vm->heap[0].fields[0] = (uint32_t)event->key_char;
    }

    return 0;
}
