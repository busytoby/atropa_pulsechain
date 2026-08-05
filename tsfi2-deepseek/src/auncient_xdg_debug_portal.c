#include "auncient_xdg_debug_portal.h"
#include <stdio.h>
#include <string.h>

void auncient_xdg_portal_init(AuncientXDGPortal *portal) {
    if (!portal) return;
    memset(portal, 0, sizeof(AuncientXDGPortal));
    portal->width = 640;
    portal->height = 480;
    portal->visible = true;
    printf("[XDG PORTAL] Initialized XDG Debugger Presentation Portal (%dx%d).\n", 
           portal->width, portal->height);
}

void auncient_xdg_portal_configure(AuncientXDGPortal *portal, uint32_t width, uint32_t height) {
    if (!portal) return;
    portal->width = width;
    portal->height = height;
    printf("[XDG PORTAL CONFIGURE] Presenter adjusted to dimensions %dx%d.\n", 
           width, height);
}

bool auncient_xdg_portal_render(AuncientXDGPortal *portal, const AuncientXDC *xdc, int64_t accumulator) {
    if (!portal || !xdc) return false;

    // Render debug status info into presentation terminal buffer
    if (xdc->halted) {
        snprintf(portal->console_buffer, sizeof(portal->console_buffer),
                 "=== XPLOS CICS ALU DEBUG HALT ===\n"
                 "PC: 0x%04X\n"
                 "Accumulator: %lld\n"
                 "Active Breakpoints: %d\n"
                 "Status: SUSPENDED",
                 xdc->last_halt_pc, (long long)accumulator, xdc->breakpoint_count);
    } else {
        snprintf(portal->console_buffer, sizeof(portal->console_buffer),
                 "=== XPLOS CICS ALU DEBUG RUN ===\n"
                 "PC: 0x%04X\n"
                 "Accumulator: %lld\n"
                 "Status: RUNNING",
                 xdc->last_halt_pc, (long long)accumulator);
    }

    printf("[XDG PORTAL RENDER] Painted Debug Screen:\n%s\n", portal->console_buffer);
    return true;
}
