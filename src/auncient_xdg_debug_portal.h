#ifndef AUNCIENT_XDG_DEBUG_PORTAL_H
#define AUNCIENT_XDG_DEBUG_PORTAL_H

#include "auncient_xdc_debugger.h"
#include <stdint.h>
#include <stdbool.h>

// Represents the XDG debugger presentation layout
typedef struct {
    uint32_t width;
    uint32_t height;
    bool visible;
    char console_buffer[1024];
} AuncientXDGPortal;

// Initializes the XDG debugger portal
void auncient_xdg_portal_init(AuncientXDGPortal *portal);

// Triggers an XDG configure resize event, dynamically adjusting the debug view size
void auncient_xdg_portal_configure(AuncientXDGPortal *portal, uint32_t width, uint32_t height);

// Paints the visual debugger status screen based on the active XDC register state
bool auncient_xdg_portal_render(AuncientXDGPortal *portal, const AuncientXDC *xdc, int64_t accumulator);

#endif // AUNCIENT_XDG_DEBUG_PORTAL_H
