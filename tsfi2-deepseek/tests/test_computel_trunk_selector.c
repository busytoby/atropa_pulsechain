#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "../src/tsfi_computel_trunk_selector.c"

int main(void) {
    printf("[TEST] Initializing Auncient Computel Trunk Selector test suite...\n");
    
    TrunkSelectorState state;
    init_trunk_selector(&state);
    
    // Assert defaults
    assert(state.active_bridge_a == 0xFFFF);
    assert(state.trunks[0].trunk_id == 800);
    assert(strcmp(state.trunks[0].line_type, "Quiet Line") == 0);
    
    // Scan quiet
    int16_t found = scan_available_trunks(&state, TONE_QUIET);
    assert(found == 800);
    
    // Scan FSK
    found = scan_available_trunks(&state, TONE_FSK);
    // Trunk 805 has FSK but is busy, so scan should skip it and return -1 since no other FSK is available
    assert(found == -1);
    
    // Establish bridge
    bool bridge_ok = establish_loopback_bridge(&state, 800, 808);
    assert(bridge_ok == true);
    assert(state.active_bridge_a == 800);
    assert(state.active_bridge_b == 808);
    assert(state.trunks[0].is_busy == true);
    assert(state.trunks[8].is_busy == true);
    
    printf("[SUCCESS] All Computel Trunk Selector & Loopback tests passed successfully.\n");
    return 0;
}
