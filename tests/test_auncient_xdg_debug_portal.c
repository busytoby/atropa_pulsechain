#include "../src/auncient_xdg_debug_portal.h"
#include "../src/auncient_xdc_debugger.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT XDG DEBUG PORTAL VERIFICATION SUITE\n");
    printf("=============================================================\n");

    AuncientXDC xdc;
    auncient_xdc_init(&xdc);
    auncient_xdc_add_breakpoint(&xdc, 0x50);

    AuncientXDGPortal portal;
    auncient_xdg_portal_init(&portal);

    // 1. Test XDG configure resizing event
    auncient_xdg_portal_configure(&portal, 1024, 768);
    assert(portal.width == 1024);
    assert(portal.height == 768);
    printf("   ✓ Resizing event registered.\n");

    uint32_t pki_keys[4] = { 1, 2, 3, 4 };

    // 2. Run execution until breakpoint is hit (suspended state)
    bool halted = auncient_xdc_check_halt(&xdc, 0x50, pki_keys, 4);
    assert(halted == true);
    assert(xdc.halted == true);

    // 3. Render state on XDG Debug Portal
    bool render_ok = auncient_xdg_portal_render(&portal, &xdc, 9999);
    assert(render_ok == true);
    assert(strstr(portal.console_buffer, "Status: SUSPENDED") != NULL);
    assert(strstr(portal.console_buffer, "Accumulator: 9999") != NULL);
    printf("   ✓ Suspended status rendering verified.\n");

    // 4. Clear breakpoints and verify running state rendering
    auncient_xdc_clear(&xdc);
    render_ok = auncient_xdg_portal_render(&portal, &xdc, 12345);
    assert(render_ok == true);
    assert(strstr(portal.console_buffer, "Status: RUNNING") != NULL);
    assert(strstr(portal.console_buffer, "Accumulator: 12345") != NULL);
    printf("   ✓ Running status rendering verified.\n");

    printf("=============================================================\n");
    printf("ALL XDG DEBUG PORTAL VERIFICATION TESTS PASSED SUCCESS\n");
    printf("=============================================================\n");
    return 0;
}
