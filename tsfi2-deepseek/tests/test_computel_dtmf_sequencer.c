#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "../src/tsfi_computel_dtmf_sequencer.c"

int main(void) {
    printf("[TEST] Initializing Auncient Computel DTMF Sequencer test suite...\n");

    TrunkSelectorState state;
    init_trunk_selector(&state);

    DtmfSequencer seq;
    init_dtmf_sequencer(&seq);

    // Feed key sequence '8', '0', '8', '#' to dial trunk 808
    feed_dtmf_digit(&seq, &state, '8');
    feed_dtmf_digit(&seq, &state, '0');
    feed_dtmf_digit(&seq, &state, '8');
    assert(seq.command_executed == false);

    feed_dtmf_digit(&seq, &state, '#');
    assert(seq.command_executed == true);

    // Verify Trunk 800 and 808 are now bridged and busy
    assert(state.trunks[0].is_busy == true);
    assert(state.trunks[8].is_busy == true);

    // Reset command executed flag
    seq.command_executed = false;

    // Feed sequence '*', '0', '#' to execute master reset override
    feed_dtmf_digit(&seq, &state, '*');
    feed_dtmf_digit(&seq, &state, '0');
    feed_dtmf_digit(&seq, &state, '#');
    assert(seq.command_executed == true);

    // Verify trunks are reset to idle (except for initially busy ones like FSK trunk 805)
    assert(state.trunks[0].is_busy == false);
    assert(state.trunks[8].is_busy == false);

    printf("[SUCCESS] All Computel DTMF command sequencer tests passed successfully.\n");
    return 0;
}
