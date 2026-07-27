#include "../src/auncient_xdc_debugger.h"
#include "../src/auncient_timeline_autodin.h"
#include <stdio.h>
#include <assert.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT XDC DEBUGGER VERIFICATION SUITE\n");
    printf("=============================================================\n");

    AuncientXDC xdc;
    auncient_xdc_init(&xdc);

    // Set breakpoints at PC 0x10 and PC 0x20
    assert(auncient_xdc_add_breakpoint(&xdc, 0x10) == true);
    assert(auncient_xdc_add_breakpoint(&xdc, 0x20) == true);
    assert(xdc.breakpoint_count == 2);

    uint32_t valid_pki[4] = { 0x1111, 0x2222, 0x3333, 0x4444 };
    uint32_t invalid_pki[3] = { 0x1111, 0x2222, 0x3333 };

    // 1. Verify check_halt on non-breakpoint PC
    printf("[TEST] Checking PC 0x05 (expected run)...\n");
    bool halt_none = auncient_xdc_check_halt(&xdc, 0x05, valid_pki, 4);
    assert(halt_none == false);
    assert(xdc.halted == false);

    // 2. Verify check_halt on breakpoint PC with invalid PKI keys (expected failure on WMQ dispatch)
    printf("[TEST] Checking PC 0x10 with invalid PKI keys...\n");
    bool halt_break_bad = auncient_xdc_check_halt(&xdc, 0x10, invalid_pki, 3);
    assert(halt_break_bad == true);
    assert(xdc.halted == true);
    assert(xdc.last_halt_pc == 0x10);

    // Reset halted state
    xdc.halted = false;

    // 3. Verify check_halt on breakpoint PC with valid PKI keys (expected success on WMQ dispatch)
    printf("[TEST] Checking PC 0x20 with valid PKI keys...\n");
    bool halt_break_good = auncient_xdc_check_halt(&xdc, 0x20, valid_pki, 4);
    assert(halt_break_good == true);
    assert(xdc.halted == true);
    assert(xdc.last_halt_pc == 0x20);

    // 4. Verify clear function
    auncient_xdc_clear(&xdc);
    assert(xdc.breakpoint_count == 0);
    assert(xdc.halted == false);

    printf("=============================================================\n");
    printf("ALL XDC DEBUGGER VERIFICATION TESTS PASSED SUCCESS\n");
    printf("=============================================================\n");
    return 0;
}
