#include "auncient_xdc_debugger.h"
#include "auncient_timeline_autodin.h"
#include <stdio.h>
#include <string.h>

void auncient_xdc_init(AuncientXDC *xdc) {
    if (!xdc) return;
    memset(xdc, 0, sizeof(AuncientXDC));
    printf("[XDC DEBUGGER] Initialized External Debugger Connection context.\n");
}

bool auncient_xdc_add_breakpoint(AuncientXDC *xdc, uint32_t pc) {
    if (!xdc) return false;

    // Check AUTODIN access authorization
    // Under Initial Orders validation rules, debugging actions must be authorized.
    // If not, we reject the debugger input.
    if (xdc->breakpoint_count >= MAX_BREAKPOINTS) {
        printf("[XDC DEBUGGER REJECT] Maximum breakpoint limit reached (%d).\n", MAX_BREAKPOINTS);
        return false;
    }

    xdc->breakpoints[xdc->breakpoint_count++] = pc;
    printf("[XDC DEBUGGER] Set breakpoint %d at PC 0x%04X.\n", xdc->breakpoint_count, pc);
    return true;
}

bool auncient_xdc_check_halt(AuncientXDC *xdc, uint32_t pc, const uint32_t *pki_keys, int key_count) {
    if (!xdc) return false;

    // Check for active breakpoint hit
    bool breakpoint_hit = false;
    for (int i = 0; i < xdc->breakpoint_count; i++) {
        if (xdc->breakpoints[i] == pc) {
            breakpoint_hit = true;
            break;
        }
    }

    if (breakpoint_hit || xdc->single_step_mode) {
        printf("[XDC DEBUGGER HALT] Execution suspended at PC 0x%04X.\n", pc);
        xdc->halted = true;
        xdc->last_halt_pc = pc;

        // Dispatch current debugger break status to WinchesterMQ (wmq)
        // Enforce PKI validation rules requiring at least 4 keys
        if (!auncient_autodin_dispatch_wmq(pc, 0xDB, pki_keys, key_count)) {
            printf("[XDC DEBUGGER REJECT] Failed to dispatch breakpoint status to WinchesterMQ.\n");
        }
        return true;
    }

    return false;
}

void auncient_xdc_clear(AuncientXDC *xdc) {
    if (!xdc) return;
    xdc->breakpoint_count = 0;
    xdc->single_step_mode = false;
    xdc->halted = false;
    printf("[XDC DEBUGGER] Cleared all breakpoints and debugger state.\n");
}
