#include "../tsfi2-deepseek/src/auncient_edsac_firewall.h"
#include "../tsfi2-deepseek/src/auncient_timeline_autodin.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT EDSAC-AUTODIN COMPILER FIREWALL TEST SUITE\n");
    printf("=============================================================\n");

    const char *firewall_rules_tape = 
        "A 80 F\n"    // Allow HTTP (port 80) - short word modifier
        "A 443 D\n"   // Allow HTTPS (port 443) - long/relocatable modifier
        "S 22 F\n";   // Drop SSH (port 22) - short word modifier

    uint32_t valid_pki[4] = { 0x1111, 0x2222, 0x3333, 0x4444 };
    uint32_t invalid_pki[3] = { 0x1111, 0x2222, 0x3333 };

    // 1. Verify default-reject initialization without authorization
    printf("[TEST] Booting firewall without AUTODIN authorization (expected block)...\n");
    bool boot_fail = auncient_firewall_init(firewall_rules_tape, 500, valid_pki, 4, NULL);
    assert(boot_fail == false);
    printf("   ✓ Blocked unauthorized boot successful.\n");

    // Grant AUTODIN authorization
    auncient_autodin_edsac_authorize(true);

    // 2. Verify boot rejection on insufficient PKI keys (< 4 keys)
    printf("[TEST] Booting firewall with insufficient PKI keys (expected block)...\n");
    bool boot_fail_pki = auncient_firewall_init(firewall_rules_tape, 500, invalid_pki, 3, NULL);
    assert(boot_fail_pki == false);
    printf("   ✓ Blocked insufficient PKI boot successful.\n");

    // 3. Test analyzer classification at Initial Orders 1
    // Setup analyzer that prohibits 'S' (Drop/Reject) opcodes
    AuncientAnalyzer analyzer;
    auncient_analyzer_init(&analyzer, 1 << ('S' - 'A'));

    printf("[TEST] Booting firewall with prohibited opcode in tape (expected analyzer block)...\n");
    bool boot_fail_analyzer = auncient_firewall_init(firewall_rules_tape, 500, valid_pki, 4, &analyzer);
    assert(boot_fail_analyzer == false);
    printf("   ✓ Analyzer classification block verified.\n");

    // 3b. Verify 6-bit FIELDATA qualification values are correctly outputted
    printf("[TEST] Performing 6-bit FIELDATA Initial Orders 1 primary data qualification...\n");
    uint32_t sample_instruction = (('F' & 0xFF) << 24) | (80 << 2);
    bool qualify_pass = auncient_analyzer_classify(&analyzer, &sample_instruction, 1);
    assert(qualify_pass == true);
    printf("   ✓ 6-bit data qualification verified.\n");

    // 4. Boot successfully with valid credentials and no prohibited opcodes (using NULL or empty analyzer)
    printf("[TEST] Booting firewall with valid credentials (expected pass)...\n");
    bool boot_pass = auncient_firewall_init(firewall_rules_tape, 500, valid_pki, 4, NULL);
    assert(boot_pass == true);
    printf("   ✓ Firewall successfully booted.\n");

    // 5. Evaluate packets
    AuncientPacket p1 = { .source_ip = 0x0A000001, .dest_port = 80, .payload_checksum = 0xAA55, .key_count = 4 };
    AuncientPacket p2 = { .source_ip = 0x0A000001, .dest_port = 22, .payload_checksum = 0xBB66, .key_count = 4 };
    AuncientPacket p3 = { .source_ip = 0x0A000001, .dest_port = 8080, .payload_checksum = 0xCC77, .key_count = 4 }; 
    AuncientPacket p4 = { .source_ip = 0x0A000001, .dest_port = 443, .payload_checksum = 0xDD88, .key_count = 4 };

    printf("[TEST] Evaluating HTTP packet (expected ALLOW)...\n");
    assert(auncient_firewall_eval_packet(&p1) == true);

    printf("[TEST] Evaluating SSH packet (expected DENY)...\n");
    assert(auncient_firewall_eval_packet(&p2) == false);

    printf("[TEST] Evaluating untracked HTTP-ALT packet (expected default DENY)...\n");
    assert(auncient_firewall_eval_packet(&p3) == false);

    printf("[TEST] Evaluating HTTPS packet (expected ALLOW)...\n");
    assert(auncient_firewall_eval_packet(&p4) == true);
    printf("   ✓ Packet evaluation controls verified.\n");

    // 6. Relocate rules using Initial Orders 2 coordinate shifts
    printf("[TEST] Testing Initial Orders 2 coordinate shift rule relocation...\n");
    auncient_firewall_relocate_rules(1000);

    // Verify HTTPS allowed target relocated to 1443
    AuncientPacket p4_old = { .source_ip = 0x0A000001, .dest_port = 443, .payload_checksum = 0xDD88, .key_count = 4 };
    AuncientPacket p4_new = { .source_ip = 0x0A000001, .dest_port = 1443, .payload_checksum = 0xDD88, .key_count = 4 };

    printf("[TEST] Evaluating old HTTPS port after relocation (expected default DENY)...\n");
    assert(auncient_firewall_eval_packet(&p4_old) == false);

    printf("[TEST] Evaluating relocated HTTPS port (expected ALLOW)...\n");
    assert(auncient_firewall_eval_packet(&p4_new) == true);

    printf("[TEST] Evaluating HTTP port after relocation (expected ALLOW as 'F' modifier does not shift)...\n");
    assert(auncient_firewall_eval_packet(&p1) == true);
    printf("   ✓ Relocatable rule shifting verified.\n");

    // 8. Test Formal Initial Orders 1 & AUTODIN Nonce Verification Gate
    printf("[TEST] Evaluating Initial Orders 1 formal AUTODIN Nonce prerequisite gate...\n");
    uint32_t test_insts[2] = {
        0x41000140, // Op='A', Permitted
        0x53000058  // Op='S', Permitted
    };
    uint32_t prev_n = 100000;
    uint32_t curr_n = (uint32_t)(((693ULL * 100000ULL) / 1000ULL) + ((31ULL * 100000ULL) / 32ULL) + 1ULL);
    uint32_t sec = 3;
    uint32_t chi = 17;

    uint64_t acc = 1ULL;
    uint64_t base = (uint64_t)curr_n % 953467954ULL;
    for (uint32_t e = sec; e > 0; e--) {
        acc = (acc * base) % 953467954ULL;
    }
    uint32_t valid_rcv = (uint32_t)((acc + (uint64_t)chi) % 953467954ULL);

    AuncientInitialOrders1GateContext valid_ctx = {
        .cycle_index_n = 2,
        .current_nonce = curr_n,
        .previous_nonce = prev_n,
        .autodin_receipt = valid_rcv,
        .auth_secret = sec,
        .auth_chi = chi,
        .prohibited_opcodes = 8388608 // Prohibit 'X'
    };

    uint32_t ruling = 99;
    bool gate_ok = auncient_initial_orders_1_verify_autodin_prerequisite(&valid_ctx, test_insts, 2, &ruling);
    assert(gate_ok == true && ruling == 0);

    // Test forged receipt (must reject)
    AuncientInitialOrders1GateContext forged_ctx = valid_ctx;
    forged_ctx.autodin_receipt = 12345;
    gate_ok = auncient_initial_orders_1_verify_autodin_prerequisite(&forged_ctx, test_insts, 2, &ruling);
    assert(gate_ok == false && ruling == 1);

    // Test broken recurrence (must reject)
    AuncientInitialOrders1GateContext broken_rec_ctx = valid_ctx;
    broken_rec_ctx.previous_nonce = 999999;
    gate_ok = auncient_initial_orders_1_verify_autodin_prerequisite(&broken_rec_ctx, test_insts, 2, &ruling);
    assert(gate_ok == false && ruling == 3);

    // Test prohibited opcode 'X' (must reject)
    uint32_t bad_insts[2] = { 0x41000140, 0x58000010 /* Op='X' */ };
    gate_ok = auncient_initial_orders_1_verify_autodin_prerequisite(&valid_ctx, bad_insts, 2, &ruling);
    assert(gate_ok == false && ruling == 2);
    printf("   ✓ Initial Orders 1 formal AUTODIN Nonce prerequisite gate verified.\n");

    // 9. Test Radical Nonce Series Trajectory Prover under EDSAC Sequence
    printf("[TEST] Testing Radical Nonce Series Trajectory Integral Prover under EDSAC...\n");
    AuncientRadicalTrajectoryMetrics traj_m = {0};
    bool traj_ok = auncient_edsac_radical_nonce_trajectory_prover(
        100000, 10, 17, 23, &traj_m
    );
    assert(traj_ok == true && traj_m.bijective_trajectory_sound == true);
    assert(traj_m.recovered_u == traj_m.forward_phase_integral_u);
    assert(traj_m.recovered_v == traj_m.back_phase_integral_v);
    assert(traj_m.symm_product_spi == (traj_m.forward_phase_integral_u + traj_m.back_phase_integral_v));
    // 10. Test VIA 6522 TOTIENT ACID Transaction & Rollback Prover
    printf("[TEST] Testing VIA 6522 TOTIENT ACID Compliance & Rollback (Initial 0 Axiom)...\n");
    AuncientTotientAcidMetrics acid_clean_m = {0};
    bool acid_clean_ok = auncient_via6522_totient_acid_prover(24537, 19623, 24537, false, &acid_clean_m);
    assert(acid_clean_ok == true && acid_clean_m.exhaustive_acid_sound == true);
    assert(acid_clean_m.initial_totient_val == 0);
    assert(acid_clean_m.committed_totient_val == 0);
    assert(acid_clean_m.atomicity_guaranteed == true);
    assert(acid_clean_m.consistency_modpow_sound == true);

    // Test Simulated Hardware Fault & Atomic Rollback
    AuncientTotientAcidMetrics acid_fault_m = {0};
    bool acid_fault_ok = auncient_via6522_totient_acid_prover(24537, 19623, 24537, true, &acid_fault_m);
    assert(acid_fault_ok == true && acid_fault_m.exhaustive_acid_sound == true);
    assert(acid_fault_m.committed_totient_val == acid_fault_m.shadow_totient_val);
    assert(acid_fault_m.committed_totient_val == 0);
    assert(acid_fault_m.durability_rollback_verified == true);
    printf("   ✓ VIA 6522 TOTIENT ACID Compliance & Rollback verified (Clean & Fault-Recovered states = 0).\n");

    // 11. Test GLM H-Bridge + WinchesterMQ SwiGLU Prover
    printf("[TEST] Testing GLM H-Bridge + WinchesterMQ SwiGLU Prover (k=3, 7/8 scale)...\n");
    AuncientGlmSwigluMetrics glm_swiglu_m = {0};
    bool glm_swiglu_ok = auncient_glm_hbridge_swiglu_prover(2.0f, 3, 0.25f, &glm_swiglu_m);
    assert(glm_swiglu_ok == true && glm_swiglu_m.overall_glm_swiglu_sound == true);
    assert(glm_swiglu_m.v_diff_potential > 0.0f);
    assert(glm_swiglu_m.wmq_gate_factor >= 0.875f && glm_swiglu_m.wmq_gate_factor <= 1.0f);
    assert(glm_swiglu_m.swiglu_output_val > 0.0f);
    printf("   ✓ GLM H-Bridge+WMQ SwiGLU verified (V_diff=%.3fV, Gate=%.3f, Out=%.3f, DispMod=%u).\n",
           glm_swiglu_m.v_diff_potential, glm_swiglu_m.wmq_gate_factor, glm_swiglu_m.swiglu_output_val, glm_swiglu_m.displacement_wrap_mod);

    // 12. Test GLM 2D-RoPE Quantel Paintbox Transform & Inverse Prover
    printf("[TEST] Testing GLM 2D-RoPE Quantel Paintbox Transform & Inverse Prover...\n");
    AuncientGlm2dRoPEMetrics glm_rope_m = {0};
    bool glm_rope_ok = auncient_glm_2d_rope_prover(1331.0f, 991.0f, 0.523598775f /* 30 deg */, &glm_rope_m);
    assert(glm_rope_ok == true && glm_rope_m.overall_2drope_sound == true);
    assert(glm_rope_m.orthogonal_norm_preserved == true);
    assert(glm_rope_m.inverse_reconstruction_sound == true);
    printf("   ✓ GLM 2D-RoPE verified (Rot=[%.2f, %.2f], Rec=[%.2f, %.2f], DispMod=%u).\n",
           glm_rope_m.u_rotated, glm_rope_m.v_rotated, glm_rope_m.u_recovered, glm_rope_m.v_recovered, glm_rope_m.displacement_wrap_mod);

    // 13. Test GLM Bidirectional Blank-Infilling Prover
    printf("[TEST] Testing GLM Bidirectional Blank-Infilling Prover (EDSAC Opcode/Address Infilling)...\n");
    AuncientGlmInfillingMetrics glm_infill_m = {0};
    uint32_t raw_inst_sample = 0x41000140; // Op='A', Addr=80, Mod=0
    bool glm_infill_ok = auncient_glm_blank_infilling_prover(
        100000, 1, raw_inst_sample, 65 /* 'A' */, 80, &glm_infill_m
    );
    assert(glm_infill_ok == true && glm_infill_m.overall_infilling_sound == true);
    assert(glm_infill_m.extracted_opcode == 65);
    assert(glm_infill_m.extracted_address == 80);
    assert(glm_infill_m.derived_parity_bit == 1);
    printf("   ✓ GLM Blank-Infilling verified (Op=%u, Addr=%u, Mod=%u, Parity=%u, DispMod=%u).\n",
           glm_infill_m.extracted_opcode, glm_infill_m.extracted_address, glm_infill_m.extracted_modifier, glm_infill_m.derived_parity_bit, glm_infill_m.displacement_wrap_mod);

    // 14. Test GLM Infilled TOTIENT Cooperative Strategy Prover (Initial 0 Axiom)
    printf("[TEST] Testing GLM Infilled TOTIENT Cooperative Strategy (Initial 0 Axiom)...\n");
    AuncientGlmTotientMetrics glm_tot_m = {0};
    bool glm_tot_ok = auncient_glm_infilled_totient_prover(0, 1331001, 991220, false, &glm_tot_m);
    assert(glm_tot_ok == true && glm_tot_m.overall_totient_infill_sound == true);
    assert(glm_tot_m.initial_totient == 0);
    assert(glm_tot_m.staged_totient == 0);
    assert(glm_tot_m.committed_totient == 0);
    printf("   ✓ GLM Infilled TOTIENT verified (Initial=%lu, Infilled_u=%lu, Staged=%lu, Committed=%lu, DispMod=%u).\n",
           glm_tot_m.initial_totient, glm_tot_m.infilled_u, glm_tot_m.staged_totient, glm_tot_m.committed_totient, glm_tot_m.displacement_wrap_mod);

    printf("=============================================================\n");
    printf("ALL EDSAC-AUTODIN COMPILER FIREWALL TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
