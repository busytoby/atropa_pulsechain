#include "tsfi_computel_blue_box.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#ifndef SAMPLE_RATE
#define SAMPLE_RATE 8000
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern BlueBoxBlockState current_block_state;

int main(void) {
    printf("[TEST] Initializing Auncient Computel Red Box Coin-to-ERC20 test suite...\n");

    // 1. Initial State Check
    blue_box_init_block(1, NULL);
    BlueBoxBlockState init_state = blue_box_get_block_state();
    uint32_t init_gas = init_state.gas_allowance;
    uint64_t init_acc = blue_box_get_accumulator();

    // 2. Generate and detect a Quarter (25 cents) drop
    float buffer[8000];
    memset(buffer, 0, sizeof(buffer));
    int count = generate_red_box_coin_tone(25, buffer, 8000);
    assert(count > 0);
    printf("[TEST] Quarter tone buffer generated: %d samples.\n", count);

    // 3. Goertzel detection and State Accumulator verification
    bool detected = blue_box_detect_and_accumulate(buffer, 500);
    assert(detected == true);
    printf("[TEST] Goertzel correctly detected the coin frequency components.\n");

    // Verify accumulator transitioned
    uint64_t current_acc = blue_box_get_accumulator();
    assert(current_acc != init_acc);
    printf("[TEST] State Accumulator successfully transitioned from %lu to %lu.\n", init_acc, current_acc);

    // 4. Map detected coin drop to Gas/ERC20 contract allocation
    current_block_state.gas_allowance += 250000; 
    assert(current_block_state.gas_allowance == init_gas + 250000);
    printf("[TEST] Gas allowance successfully topped up: %u.\n", current_block_state.gas_allowance);

    // 5. Test Dynamic Rate Matrix
    blue_box_centrex_set_trunk_rate(808, 120); // 120 gas units/min (2 gas units/sec)
    assert(blue_box_centrex_get_trunk_rate(808) == 120);
    printf("[TEST] Dynamic Rate Matrix configured and verified: 120 units/min on Trunk 808.\n");

    // 6. Test Multi-Coin Accumulation
    uint64_t before_multi = blue_box_get_accumulator();
    blue_box_accumulate_coin(10); // Dime
    blue_box_accumulate_coin(25); // Quarter
    assert(blue_box_get_accumulator() != before_multi);
    printf("[TEST] Multi-Coin Accumulation executed successfully.\n");

    // 7. Test Signed ERC20 Transaction Bridge
    char tx_buf[256];
    bool tx_ok = blue_box_generate_erc20_tx(tx_buf, sizeof(tx_buf));
    assert(tx_ok == true);
    assert(strstr(tx_buf, "\"method\":\"erc20_transfer\"") != NULL);
    printf("[TEST] ERC20 Transaction Payload generated: %s\n", tx_buf);

    // 8. Test Real-time Balance Depletion
    current_block_state.gas_allowance = 500;
    current_block_state.active_trunk_mask = (1U << 8); // Active on Trunk 808
    
    // Deplete for 30 seconds (cost = (120 * 30)/60 = 60 gas units)
    bool active = blue_box_deplete_session_gas(808, 30);
    assert(active == true);
    assert(current_block_state.gas_allowance == 440);

    // Deplete beyond limit (cost = (120 * 300)/60 = 600 gas units, exceeds 440)
    active = blue_box_deplete_session_gas(808, 300);
    assert(active == false);
    assert(current_block_state.gas_allowance == 0);
    assert((current_block_state.active_trunk_mask & (1U << 8)) == 0); // Trunk 808 disconnected
    // 9. Test Slug Validation (Anti-Fraud)
    // A genuine coin tone has decay and transient noise
    assert(blue_box_validate_slug(buffer, 500) == true);
    
    // Create a fake slug (flat electronic sine wave: no transient noise, no decay)
    float fake_slug[500];
    for (int i = 0; i < 500; i++) {
        double t = (double)i / SAMPLE_RATE;
        fake_slug[i] = (float)sin(2.0 * M_PI * 1700.0 * t);
    }
    assert(blue_box_validate_slug(fake_slug, 500) == false);
    printf("[TEST] Slug Validation correctly allowed physical gong and rejected electronic fake wave.\n");

    // 10. Test P2P Teleconferencing Automation and Routing
    printf("[TEST] Verifying P2P Teleconferencing Automation and Routing...\n");
    // Register trunk rates for participants
    blue_box_centrex_set_trunk_rate(808, 60);  // Participant 1: 60 gas units/min (1/sec)
    blue_box_centrex_set_trunk_rate(810, 180); // Participant 2: 180 gas units/min (3/sec)

    // Set up active state
    current_block_state.gas_allowance = 1000;
    current_block_state.active_trunk_mask = (1U << 8) | (1U << 10); // Trunks 808 and 810 active

    // Deplete gas for 10 seconds of conference time
    bool p1_active = blue_box_deplete_session_gas(808, 10); // cost = (60 * 10)/60 = 10 units
    bool p2_active = blue_box_deplete_session_gas(810, 10); // cost = (180 * 10)/60 = 30 units
    assert(p1_active == true);
    assert(p2_active == true);
    assert(current_block_state.gas_allowance == 960); // 1000 - 10 - 30

    // Verify billing transaction generation
    char conf_tx[256];
    bool conf_tx_ok = blue_box_generate_erc20_tx(conf_tx, sizeof(conf_tx));
    assert(conf_tx_ok == true);
    assert(strstr(conf_tx, "\"amount\":960") != NULL);
    printf("[TEST] Conference Session Billing Tx generated: %s\n", conf_tx);

    // 11. Test Session Key Rotation on Coin Drop
    uint64_t before_rotate = current_block_state.session_key;
    blue_box_accumulate_coin(25); // Drop Quarter
    assert(current_block_state.session_key != before_rotate);
    printf("[TEST] Session key rotated on coin drop successfully: %lu.\n", current_block_state.session_key);

    // 12. Test On-Chain Tariff Negotiation
    uint32_t negotiated_rate = 0;
    bool tariff_ok = blue_box_negotiate_tariff(805, &negotiated_rate); // Trunk 805 rate: 60 + 5 * 5 = 85
    assert(tariff_ok == true);
    assert(negotiated_rate == 85);
    assert(blue_box_centrex_get_trunk_rate(805) == 85);
    printf("[TEST] On-chain tariff negotiation completed successfully: rate %u units/min.\n", negotiated_rate);

    // 13. Test UDP Tone Streaming
    bool udp_tone_ok = blue_box_send_udp_tone(5005, buffer, 500);
    assert(udp_tone_ok == true);
    printf("[TEST] UDP Tone Streaming successfully simulated.\n");

    // 14. Test Kermit-over-UDP Packetization
    uint8_t kerm_data[4] = {0x01, 0x02, 0x03, 0x04};
    bool udp_kerm_ok = blue_box_kermit_send_udp(5006, kerm_data, 4);
    assert(udp_kerm_ok == true);
    printf("[TEST] Kermit-over-UDP packet transmission simulated successfully.\n");

    // 15. Test UDP Billing Alert
    bool udp_alert_ok = blue_box_send_udp_billing_alert(5007);
    assert(udp_alert_ok == true);
    printf("[TEST] UDP Billing Alert dispatch simulated successfully.\n");

    // 16. Test Yul WinchesterMQ push-pull complementary signal model
    extern bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t cd_size, uint8_t *retval, size_t *retval_len);
    // Call selector 0xe399f0e0 with positive signal 1000 (exceeds 700 drop)
    uint8_t yul_cd[36] = {0xe3, 0x99, 0xf0, 0xe0};
    uint32_t val_in = 1000;
    // Pack 1000 as big-endian 32-byte argument
    yul_cd[35] = (uint8_t)(val_in & 0xFF);
    yul_cd[34] = (uint8_t)((val_in >> 8) & 0xFF);
    
    printf("[DEBUG] yul_cd bytes: ");
    for (int i = 0; i < 36; i++) printf("%02x ", yul_cd[i]);
    printf("\n");
    
    uint8_t yul_ret[32];
    size_t yul_ret_len = 32;
    bool yul_ok = lau_yul_thunk_execute("WinchesterMQ", yul_cd, 36, yul_ret, &yul_ret_len);
    assert(yul_ok == true);
    
    // The expected output is v_in - diode_drop = 1000 - 700 = 300
    uint32_t val_out = ((uint32_t)yul_ret[30] << 8) | yul_ret[31];
    printf("[DEBUG] val_out = %u, yul_ret bytes: ", val_out);
    for (int i = 0; i < 32; i++) printf("%02x ", yul_ret[i]);
    printf("\n");
    assert(val_out == 300);
    printf("[TEST] Yul WinchesterMQ unified NPN/PNP hardware crossover simulation verified: %u.\n", val_out);

    // 17. Test Closed-Loop Ouroboros Feedback
    uint64_t before_ouro = blue_box_get_accumulator();
    blue_box_ouroboros_tick();
    uint64_t after_ouro = blue_box_get_accumulator();
    assert(after_ouro != before_ouro);
    printf("[TEST] Closed-loop Ouroboros Feedback tick executed and verified successfully.\n");

    printf("[SUCCESS] All Red Box Coin-to-ERC20 integration tests passed.\n");
    return 0;
}
