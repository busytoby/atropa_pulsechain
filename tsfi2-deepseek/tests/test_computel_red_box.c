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
extern uint64_t lau_yul_thunk_sload(uint64_t key);
extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);

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
    
    // Call resetFirewall (0x3c130093) selector to reset threat counters and block flag
    uint8_t reset_cd[4] = {0x3c, 0x13, 0x00, 0x93};
    uint8_t reset_ret[32];
    size_t reset_ret_len = 32;
    lau_yul_thunk_execute("WinchesterMQ", reset_cd, 4, reset_ret, &reset_ret_len);
    
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

    // 18. Test AGC-Driven Firewall (Threshold Safety Block)
    // Send 4 consecutive high-voltage signals (e.g. 5000) to trigger firewall threat count
    yul_cd[35] = 0x88; // 5000 & 0xFF
    yul_cd[34] = 0x13; // (5000 >> 8) & 0xFF
    
    for (int i = 0; i < 4; i++) {
        yul_ret_len = 32;
        lau_yul_thunk_execute("WinchesterMQ", yul_cd, 36, yul_ret, &yul_ret_len);
    }
    // The 4th call should trigger the firewall block, muting v_out to 0
    uint32_t val_mute = ((uint32_t)yul_ret[30] << 8) | yul_ret[31];
    assert(val_mute == 0);
    printf("[TEST] AGC-driven Firewall triggered and successfully muted signal.\n");

    // 19. Test PLL Phase-Lock tracking
    extern uint64_t lau_yul_thunk_sload(uint64_t key);
    uint64_t pll_dev = lau_yul_thunk_sload(0xF125);
    assert(pll_dev != 0);
    printf("[TEST] PLL Phase-Lock tracking deviation registered: %lu.\n", pll_dev);

    // 20. Test BTC Script Dual Stack verification (A side standard crypto, B side PLL telemetry)
    uint8_t dummy_sig[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    uint8_t dummy_pubkey[4] = {0xCA, 0xFE, 0xBA, 0xBE};
    
    // Set a high max deviation limit so it passes
    bool ds_ok = blue_box_verify_dual_stack(dummy_sig, 4, dummy_pubkey, 4, 1ULL << 63);
    assert(ds_ok == true);
    
    // Set a very low max deviation limit (e.g. 50) so it fails
    bool ds_fail = blue_box_verify_dual_stack(dummy_sig, 4, dummy_pubkey, 4, 50);
    assert(ds_fail == false);
    
    printf("[TEST] BTC Script Dual Stack verification (A-side & B-side matching) validated successfully.\n");

    // 21. Test MF/FSK Tone Generator
    float tone_samples[160];
    bool tone_ok = blue_box_generate_tone(700, 900, tone_samples, 160);
    assert(tone_ok == true);
    assert(tone_samples[0] == 0.0);
    uint64_t reg_f1 = lau_yul_thunk_sload(0xF150);
    uint64_t reg_f2 = lau_yul_thunk_sload(0xF151);
    assert(reg_f1 == 700);
    assert(reg_f2 == 900);
    printf("[TEST] MF/FSK Tone Generator validated and verified in VM registers.\n");
    
    // 22. Test Visual Coverage & Symmetry Telemetry Classifier
    float x_coords[5] = {-0.5f, 0.0f, 0.5f, -0.5f, 0.5f};
    float y_coords[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float coverage = 0.0f, symmetry = 0.0f;
    bool visual_ok = blue_box_evaluate_visual_coverage(x_coords, y_coords, 5, &coverage, &symmetry);
    assert(visual_ok == true);
    assert(coverage > 0.0f);
    assert(symmetry > 0.0f);
    printf("[TEST] Visual Coverage (%.4f) & Symmetry (%.4f) telemetry verification passed.\n", coverage, symmetry);

    // 23. Test MF Dialing Sequence State Machine & Router
    bool d_kp = blue_box_dial_mf_digit('K');
    assert(d_kp == true);
    uint64_t reg_state = lau_yul_thunk_sload(0xF130);
    assert(reg_state == 1);
    
    blue_box_dial_mf_digit('8');
    blue_box_dial_mf_digit('0');
    blue_box_dial_mf_digit('8');
    uint64_t reg_hash = lau_yul_thunk_sload(0xF131);
    assert(reg_hash == 808);
    
    bool d_st = blue_box_dial_mf_digit('S');
    assert(d_st == true);
    reg_state = lau_yul_thunk_sload(0xF130);
    assert(reg_state == 2);
    printf("[TEST] MF Dialing sequence state machine (KP + 808 + ST) verified in VM registers.\n");
    
    // 24. Test Formant Vowel Vocable Synthesizer
    float vowel_samples[160];
    bool vowel_ok = blue_box_synthesize_vowel('A', vowel_samples, 160);
    assert(vowel_ok == true);
    assert(vowel_samples[0] == 0.0f);
    printf("[TEST] Formant Vowel Vocable Synthesizer ('A' formants F1/F2/F3) verified.\n");

    // 25. Test Wink-Start Handshaking State Machine
    bool wink_valid = blue_box_trigger_wink(200);
    assert(wink_valid == true);
    uint64_t reg_wink = lau_yul_thunk_sload(0xF135);
    assert(reg_wink == 1);
    
    bool wink_invalid = blue_box_trigger_wink(500);
    assert(wink_invalid == false);
    reg_wink = lau_yul_thunk_sload(0xF135);
    assert(reg_wink == 0);
    printf("[TEST] Wink-Start handshaking pulse limits verified in VM registers.\n");
    
    // 26. Test Trunk Line Splitting & 2600 Hz Notch Filter
    float raw_samples[160];
    float filtered_samples[160];
    for (int i = 0; i < 160; i++) {
        raw_samples[i] = sin(2.0 * M_PI * 2600.0 * i / 8000.0);
    }
    
    bool split_ok = blue_box_apply_notch_filter(raw_samples, filtered_samples, 160, false);
    assert(split_ok == true);
    uint64_t reg_split = lau_yul_thunk_sload(0xF136);
    assert(reg_split == 0);
    assert(filtered_samples[10] == raw_samples[10]);
    
    split_ok = blue_box_apply_notch_filter(raw_samples, filtered_samples, 160, true);
    assert(split_ok == true);
    reg_split = lau_yul_thunk_sload(0xF136);
    assert(reg_split == 1);
    // Double precision absolute check for attenuation
    double f_val = (double)filtered_samples[150];
    double abs_val = f_val >= 0.0 ? f_val : -f_val;
    assert(abs_val < 0.05); 
    printf("[TEST] Trunk Line Splitting & 2600 Hz Notch Filter verified.\n");

    // 27. Test BGP Peer Table Management & Precedence Routing
    bool peer_ok = blue_box_add_bgp_peer(0x0A000001, 65001, 3, 20);
    assert(peer_ok == true);
    peer_ok = blue_box_add_bgp_peer(0x0A000002, 65002, 1, 45);
    assert(peer_ok == true);
    
    uint32_t peer_prec = 0, peer_lat = 0;
    bool peer_found = blue_box_get_bgp_peer(0x0A000001, &peer_prec, &peer_lat);
    assert(peer_found == true);
    assert(peer_prec == 3);
    assert(peer_lat == 20);
    
    uint32_t ips[5];
    uint32_t match_count = blue_box_query_bgp_peers_by_precedence(1, ips, 5);
    assert(match_count == 1);
    assert(ips[0] == 0x0A000002);
    printf("[TEST] BGP Peer Table inserts and precedence-level queries verified.\n");

    // 28. Test GGUF Layer BGP Routing
    bool route_ok = blue_box_add_gguf_layer_route("blk.0.self_attn", 0x0A000001, 5000000);
    assert(route_ok == true);
    route_ok = blue_box_add_gguf_layer_route("blk.0.self_attn", 0x0A000002, 10000000);
    assert(route_ok == true);
    
    uint32_t selected_peer = blue_box_select_gguf_layer_peer("blk.0.self_attn");
    assert(selected_peer == 0x0A000001);
    printf("[TEST] GGUF Layer BGP cost-vector routing path selection verified.\n");

    // 29. Test Black Box Line Voltage Clamping & Billing Status
    uint32_t volt = 0;
    bool bill_act = true;
    bool bb_ok = blue_box_simulate_black_box(300.0f, &volt, &bill_act);
    assert(bb_ok == true);
    assert(volt == 11);
    assert(bill_act == false);
    uint64_t reg_volt = lau_yul_thunk_sload(0xF150);
    uint64_t reg_bill = lau_yul_thunk_sload(0xF151);
    assert(reg_volt == 11);
    assert(reg_bill == 0);
    
    bb_ok = blue_box_simulate_black_box(2000.0f, &volt, &bill_act);
    assert(bb_ok == true);
    assert(volt == 32);
    assert(bill_act == true);
    reg_volt = lau_yul_thunk_sload(0xF150);
    reg_bill = lau_yul_thunk_sload(0xF151);
    assert(reg_volt == 32);
    assert(reg_bill == 1);
    printf("[TEST] Black Box Line Voltage clamping and answer-supervision billing bypass verified.\n");

    // 30. Test Hook Flash Signaling Detection & Flash Counting
    bool flash_det = false;
    uint32_t flash_cnt = 0;
    bool hf_ok = blue_box_trigger_hook_flash(700, &flash_det, &flash_cnt);
    assert(hf_ok == true);
    assert(flash_det == true);
    assert(flash_cnt == 1);
    uint64_t reg_flash_det = lau_yul_thunk_sload(0xF152);
    uint64_t reg_flash_cnt = lau_yul_thunk_sload(0xF153);
    assert(reg_flash_det == 1);
    assert(reg_flash_cnt == 1);
    
    hf_ok = blue_box_trigger_hook_flash(200, &flash_det, &flash_cnt);
    assert(hf_ok == true);
    assert(flash_det == false);
    assert(flash_cnt == 1);
    
    hf_ok = blue_box_trigger_hook_flash(1200, &flash_det, &flash_cnt);
    assert(hf_ok == true);
    assert(flash_det == false);
    assert(flash_cnt == 0);
    printf("[TEST] Hook Flash timing window and session flash registers verified.\n");

    // 31. Test AI Agent Black Box Driver
    uint32_t temp_volt = 0;
    bool temp_bill = false;
    blue_box_simulate_black_box(2000.0f, &temp_volt, &temp_bill);
    assert(temp_bill == true);
    
    uint32_t ai_cmd = 0;
    bool ai_ok = blue_box_run_ai_driver(true, &ai_cmd);
    assert(ai_ok == true);
    assert(ai_cmd == 1);
    
    uint64_t reg_ai_enable = lau_yul_thunk_sload(0xF155);
    uint64_t reg_ai_cmd = lau_yul_thunk_sload(0xF156);
    uint64_t reg_bill_after = lau_yul_thunk_sload(0xF151);
    uint64_t reg_volt_after = lau_yul_thunk_sload(0xF150);
    assert(reg_ai_enable == 1);
    assert(reg_ai_cmd == 1);
    assert(reg_bill_after == 0);
    assert(reg_volt_after == 11);
    printf("[TEST] AI Agent Driver automated billing suppression verified.\n");

    // 32. Test AI Speech Sequencer
    char speech_seq[16];
    bool speech_ok = blue_box_run_ai_speech_sequencer(2, speech_seq, sizeof(speech_seq));
    assert(speech_ok == true);
    assert(strcmp(speech_seq, "OI") == 0);
    uint64_t reg_speech_state = lau_yul_thunk_sload(0xF170);
    uint64_t reg_speech_vowel = lau_yul_thunk_sload(0xF171);
    assert(reg_speech_state == 2);
    assert(reg_speech_vowel == (uint64_t)'O');
    printf("[TEST] AI Speech Sequencer vowel transition pipelines verified.\n");

    // 33. Test Unified Singular Telecom Dispatch Tick
    lau_yul_thunk_sstore(0xF180, 0);
    lau_yul_thunk_sstore(0xF155, 1);
    uint32_t v_setup = 0;
    bool b_setup = false;
    blue_box_simulate_black_box(2000.0f, &v_setup, &b_setup);
    assert(b_setup == true);
    
    lau_yul_thunk_sstore(0xF199, 10000);
    
    bool tick_ok = blue_box_unified_tick(500);
    assert(tick_ok == true);
    
    uint64_t reg_ticks = lau_yul_thunk_sload(0xF180);
    uint64_t reg_gas = lau_yul_thunk_sload(0xF199);
    uint64_t reg_tx = lau_yul_thunk_sload(0xF145);
    
    assert(reg_ticks == 500);
    assert(reg_gas < 10000);
    assert(reg_tx > 0);
    printf("[TEST] Unified Telecom Dispatch Tick pipeline executed and verified successfully.\n");

    // 34. Test Green Box Coin Disposition Control Actions
    uint32_t green_act = 0;
    bool green_ok = blue_box_trigger_green_box(1700, 500, &green_act);
    assert(green_ok == true);
    assert(green_act == 1);
    uint64_t reg_green = lau_yul_thunk_sload(0xF185);
    assert(reg_green == 1);
    
    green_ok = blue_box_trigger_green_box(2200, 500, &green_act);
    assert(green_ok == true);
    assert(green_act == 2);
    reg_green = lau_yul_thunk_sload(0xF185);
    assert(reg_green == 2);
    printf("[TEST] Green Box Coin disposition control actions verified.\n");

    // 35. Test Green Box ERC20 compatible transaction thunks
    char payload_collect[256];
    char payload_refund[256];
    bool g_tx_ok = blue_box_green_box_to_erc20(1, payload_collect, sizeof(payload_collect));
    assert(g_tx_ok == true);
    assert(strstr(payload_collect, "erc20_collect") != NULL);
    
    g_tx_ok = blue_box_green_box_to_erc20(2, payload_refund, sizeof(payload_refund));
    assert(g_tx_ok == true);
    assert(strstr(payload_refund, "erc20_refund") != NULL);
    printf("[TEST] Green Box ERC20 compatible transaction thunks generated and verified.\n");

    // 36. Test Green Box Coin Disposition with Diyat fee splitting
    lau_yul_thunk_sstore(0xF195, 0);
    lau_yul_thunk_sstore(0xF196, 5);
    lau_yul_thunk_sstore(0xF187, 0);
    lau_yul_thunk_sstore(0xF199, 1000);
    current_block_state.gas_allowance = 100;
    
    uint32_t diyat_act = 0;
    bool diyat_ok = blue_box_trigger_green_box_diyat(2200, 500, &diyat_act);
    assert(diyat_ok == true);
    assert(diyat_act == 2);
    
    uint64_t fee_pool = lau_yul_thunk_sload(0xF195);
    uint64_t refund_pool = lau_yul_thunk_sload(0xF187);
    uint64_t total_gas = lau_yul_thunk_sload(0xF199);
    
    assert(fee_pool == 5);
    assert(refund_pool == 95);
    assert(total_gas == 1095);
    printf("[TEST] Green Box Coin disposition with Diyat fee splitting verified.\n");

    // 37. Test QING Coaxial Session and PLL Broadcast Synchronization
    lau_yul_thunk_sstore(0xF18C, 100);
    uint32_t freq_lock = 0;
    bool sync_ok = blue_box_sync_qing_coaxial(5, 1000, &freq_lock);
    assert(sync_ok == true);
    assert(freq_lock == 1050);
    
    uint64_t reg_users = lau_yul_thunk_sload(0xF18B);
    uint64_t reg_pll_lock = lau_yul_thunk_sload(0xF125);
    assert(reg_users == 5);
    assert(reg_pll_lock == 1050);
    printf("[TEST] QING Coaxial session and PLL broadcast sync verified.\n");

    // 38. Test Green Box Autonomous Agent and RDBMS Synchronization
    lau_yul_thunk_sstore(0xF135, 1);
    lau_yul_thunk_sstore(0xF121, 0);
    lau_yul_thunk_sstore(0xF136, 0);
    lau_yul_thunk_sstore(0xF18E, 1);
    lau_yul_thunk_sstore(0xF196, 5);
    
    uint32_t agent_act = 0;
    bool agent_ok = blue_box_run_green_agent(&agent_act);
    assert(agent_ok == true);
    assert(agent_act == 1);
    
    uint64_t reg_agent_state = lau_yul_thunk_sload(0xF191);
    assert(reg_agent_state == 2);
    
    uint64_t sync_hash = 0;
    bool db_sync_ok = blue_box_sync_green_agent_rdbms(&sync_hash);
    assert(db_sync_ok == true);
    uint64_t reg_db_hash = lau_yul_thunk_sload(0xF192);
    assert(reg_db_hash == sync_hash);
    
    FILE *f_check = fopen("assets/rdbms_tables.json", "r");
    if (!f_check) {
        f_check = fopen("../assets/rdbms_tables.json", "r");
    }
    assert(f_check != NULL);
    fclose(f_check);
    printf("[TEST] Green Box autonomous agent logic and RDBMS sync (disk serialization) verified.\n");

    // 39. Test Dynamic Validator Bidding Registry and Selection
    lau_yul_thunk_sstore(0xF180, 0);
    lau_yul_thunk_sstore(0xF197, 3);
    lau_yul_thunk_sstore(0xF198, 99);
    
    assert(blue_box_add_validator_bid(1, 10, 45) == true);
    assert(blue_box_add_validator_bid(2, 5, 50) == true);
    
    lau_yul_thunk_sstore(0xF180, 70000);
    uint32_t selected_id = 0;
    assert(blue_box_select_validator_route(&selected_id) == true);
    assert(selected_id == 1);
    
    lau_yul_thunk_sstore(0xF180, 310000);
    assert(blue_box_select_validator_route(&selected_id) == true);
    assert(selected_id == 99);
    printf("[TEST] Dynamic validator bidding, adaptive decay, and fallback routing verified.\n");

    printf("[SUCCESS] All Red Box Coin-to-ERC20 integration tests passed.\n");
    return 0;
}
