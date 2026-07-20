#ifndef TSFI_PARC_MVC_H
#define TSFI_PARC_MVC_H

#include "tsfi_parc_window.h"
#include "tsfi_parc_smalltalk.h"

#define MVC_EVENT_KEY   1
#define MVC_EVENT_MOVE  2
#define MVC_EVENT_CLICK 3

typedef struct {
    int type;
    int x;
    int y;
    char key_char;
} tsfi_parc_mvc_event_t;

typedef struct {
    tsfi_parc_window_manager_t *wm;
    tsfi_parc_smalltalk_vm_t *vm;
    int dispatched_count;
} tsfi_parc_mvc_dispatcher_t;

/* Initialize MVC event dispatcher linked to window manager and Smalltalk VM */
void tsfi_parc_mvc_init(tsfi_parc_mvc_dispatcher_t *mvc, tsfi_parc_window_manager_t *wm, tsfi_parc_smalltalk_vm_t *vm);

/* Dispatch an event to active target controllers, updating models and triggering redraws */
int tsfi_parc_mvc_dispatch(tsfi_parc_mvc_dispatcher_t *mvc, const tsfi_parc_mvc_event_t *event);

#endif // TSFI_PARC_MVC_H
