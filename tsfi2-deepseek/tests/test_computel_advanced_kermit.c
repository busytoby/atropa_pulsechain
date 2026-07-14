#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "../src/tsfi_computel_advanced_kermit.c"

int main(void) {
    printf("[TEST] Initializing Auncient Computel Advanced Kermit test suite...\n");

    AdvancedKermitState state;
    init_advanced_kermit(&state, 0xABCDE12345ULL);

    // 1. Verify FEC Parity & Recovery
    uint8_t p1[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t p2[8] = {10, 20, 30, 40, 50, 60, 70, 80};
    uint8_t parity[8];
    uint8_t recovered[8];

    compute_fec_parity(p1, p2, parity, 8);
    recover_fec_packet(p1, parity, recovered, 8);

    // Verify recovery matching exactly
    for (int i = 0; i < 8; i++) {
        assert(recovered[i] == p2[i]);
    }
    printf("[TEST] FEC Recovery verified successfully.\n");

    // 2. Verify Rate Adaptation
    uint32_t init_delay = state.current_delay_us;
    // Sending ACK should reduce delay
    handle_transmission_feedback(&state, true);
    assert(state.current_delay_us < init_delay);

    // Sending NAK should increase delay
    uint32_t lower_delay = state.current_delay_us;
    handle_transmission_feedback(&state, false);
    assert(state.current_delay_us > lower_delay);
    printf("[TEST] Dynamic rate backoff verified successfully.\n");

    // 3. Verify Key Rotation (Threshold = 4 successful ACKs)
    uint64_t init_key = state.active_key;
    handle_transmission_feedback(&state, true); // Count 2
    handle_transmission_feedback(&state, true); // Count 3
    handle_transmission_feedback(&state, true); // Count 4 -> Rotates key!
    assert(state.active_key != init_key);
    printf("[TEST] Ephemeral key rotation verified successfully.\n");

    // 4. Verify Sliding Window availability bounds
    state.last_ack = 0;
    state.next_seq = 0;
    assert(is_window_available(&state) == true);

    state.next_seq = 8; // Fill window (window_size = 8)
    assert(is_window_available(&state) == false);

    state.last_ack = 2; // Slide window forward by 2
    assert(is_window_available(&state) == true);
    printf("[TEST] Sliding Window boundaries verified successfully.\n");

    printf("[SUCCESS] All Advanced Kermit optimization tests passed successfully.\n");
    return 0;
}
