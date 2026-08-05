#ifndef AUNCIENT_XDC_DEBUGGER_H
#define AUNCIENT_XDC_DEBUGGER_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_BREAKPOINTS 16

// Struct representing the debugger connection state
typedef struct {
    uint32_t breakpoints[MAX_BREAKPOINTS];
    int breakpoint_count;
    bool single_step_mode;
    bool halted;
    uint32_t last_halt_pc;
} AuncientXDC;

// Initializes the XDC debugger module
void auncient_xdc_init(AuncientXDC *xdc);

// Adds a breakpoint at the specified execution PC
bool auncient_xdc_add_breakpoint(AuncientXDC *xdc, uint32_t pc);

// Evaluates whether the CPU should halt at the current instruction boundary
bool auncient_xdc_check_halt(AuncientXDC *xdc, uint32_t pc, const uint32_t *pki_keys, int key_count);

// Clears all active breakpoints
void auncient_xdc_clear(AuncientXDC *xdc);

#endif // AUNCIENT_XDC_DEBUGGER_H
