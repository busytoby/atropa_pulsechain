#include "../tsfi2-deepseek/src/auncient_edsac_firewall.h"
#include "../tsfi2-deepseek/src/auncient_timeline_autodin.h"
#include "../tsfi2-deepseek/inc/auncient_harvard_computation_lab.h"
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

    // 15. Test GLM Multi-Task Compatibility Prover for Zorse
    printf("[TEST] Testing GLM Multi-Task Compatibility for Zorse (Sub-1000ns VDSO Latency Guard Gate)...\n");
    AuncientGlmZorseMetrics glm_zorse_m = {0};
    bool glm_zorse_ok = auncient_glm_zorse_multitask_prover(
        7 /* R7 */, 875, 128 /* bytes */, 80 /* ns */, &glm_zorse_m
    );
    assert(glm_zorse_ok == true && glm_zorse_m.overall_zorse_multitask_sound == true);
    assert(glm_zorse_m.short_mask_infill_ok == true);
    assert(glm_zorse_m.long_mask_synthesis_ok == true);
    assert(glm_zorse_m.vdso_latency_gate_passed == true);
    printf("   ✓ GLM Multi-Task for Zorse verified (Reg=R%u, Infill=%u, DivLen=%zu, VDSO=%uns, DispMod=%u).\n",
           glm_zorse_m.target_reg_idx, glm_zorse_m.infilled_reg_val, glm_zorse_m.division_length_bytes, glm_zorse_m.vdso_latency_ns, glm_zorse_m.displacement_wrap_mod);

    // 16. Test GLM 2D Positional Encoding Prover for Zorse
    printf("[TEST] Testing GLM 2D Positional Encoding for Zorse (COBOL AST Hierarchy)...\n");
    AuncientGlmZorse2DPosMetrics glm_pos2d_m = {0};
    uint32_t test_linear_pos = (2 * 64) + 17; // D2, token 17 (total 145)
    bool glm_pos2d_ok = auncient_glm_zorse_2d_position_prover(
        test_linear_pos, 2 /* D2 */, 64 /* tokens per div */, 80 /* ns */, &glm_pos2d_m
    );
    assert(glm_pos2d_ok == true && glm_pos2d_m.overall_2d_position_sound == true);
    assert(glm_pos2d_m.pos_1_inter_division == 2);
    assert(glm_pos2d_m.pos_2_intra_division == 17);
    assert(glm_pos2d_m.reconstructed_linear_pos == test_linear_pos);
    assert(glm_pos2d_m.positional_bijection_sound == true);
    assert(glm_pos2d_m.vdso_latency_gate_passed == true);
    printf("   ✓ GLM 2D Positional for Zorse verified (Pos1=D%u, Pos2=%u, Reconstructed=%u, VDSO=%uns, DispMod=%u).\n",
           glm_pos2d_m.pos_1_inter_division, glm_pos2d_m.pos_2_intra_division, glm_pos2d_m.reconstructed_linear_pos, glm_pos2d_m.vdso_latency_ns, glm_pos2d_m.displacement_wrap_mod);

    // 17. Test GLM FET Link Dynamics Prover (Accumulator Redirection Model)
    printf("[TEST] Testing GLM FET Link Dynamics (Monotonic Thermodynamic Dissipation)...\n");
    AuncientGlmFetLinkMetrics glm_fet_m = {0};
    bool glm_fet_ok = auncient_glm_fet_link_dynamics_prover(
        1000000 /* Saat */, 50 /* decay */, 5 /* steps */, 3 /* k=3 */, &glm_fet_m
    );
    assert(glm_fet_ok == true && glm_fet_m.overall_fet_link_sound == true);
    assert(glm_fet_m.final_charge_mu < glm_fet_m.initial_charge_mu);
    assert(glm_fet_m.monotonic_dissipation_ok == true);
    assert(glm_fet_m.inverse_link_recovery_sound == true);
    assert(glm_fet_m.accumulator_redirection_sound == true);
    printf("   ✓ GLM FET Link Dynamics verified (Initial=%lu, Final=%lu, Reconstructed=%lu, DispMod=%u).\n",
           glm_fet_m.initial_charge_mu, glm_fet_m.final_charge_mu, glm_fet_m.reconstructed_charge_mu, glm_fet_m.displacement_wrap_mod);

    // 18. Test Accumulator Valves upon Zero TOTIENT ACID Compliance & Rollback
    printf("[TEST] Testing Accumulator Valves upon Zero TOTIENT (ACID Transactional Rollback)...\n");
    AuncientTotientValveAcidMetrics clean_valve_m = {0};
    bool clean_valve_ok = auncient_glm_totient_valve_acid_prover(
        0, 1000000 /* Saat */, 991220 /* v */, false, 3 /* k=3 */, &clean_valve_m
    );
    assert(clean_valve_ok == true && clean_valve_m.overall_valve_acid_sound == true);
    assert(clean_valve_m.initial_totient_val == 0);
    assert(clean_valve_m.staged_flow_val == 0);
    assert(clean_valve_m.committed_totient_val == 0);

    AuncientTotientValveAcidMetrics fault_valve_m = {0};
    bool fault_valve_ok = auncient_glm_totient_valve_acid_prover(
        0, 1000000 /* Saat */, 991220 /* v */, true, 3 /* k=3 */, &fault_valve_m
    );
    assert(fault_valve_ok == true && fault_valve_m.overall_valve_acid_sound == true);
    assert(fault_valve_m.committed_totient_val == 0);
    assert(fault_valve_m.durability_rollback_verified == true);
    printf("   ✓ Accumulator Valves upon Zero TOTIENT verified (Clean & Fault-Recovered states = 0, DispMod=%u).\n",
           clean_valve_m.displacement_wrap_mod);

    // 19. Test GLM 2D Bidirectional Attention & Block-Causal Mask Prover
    printf("[TEST] Testing GLM 2D Bidirectional Attention & Block-Causal Masking (Part-A/Part-B Isolation)...\n");
    AuncientGlmAttnMaskMetrics attn_m1 = {0}, attn_m2 = {0}, attn_m3 = {0}, attn_m4 = {0};
    
    // Case 1: Part-A (0) attending backwards to Part-A (3) -> ALLOW (0)
    bool ok_a1 = auncient_glm_bidirectional_attention_mask_prover(8, 4, 0, 3, &attn_m1);
    assert(ok_a1 == true && attn_m1.attention_decision == 0);

    // Case 2: Part-A (1) attempting to peek at Part-B target (5) -> PROHIBITED (3)
    bool ok_a2 = auncient_glm_bidirectional_attention_mask_prover(8, 4, 1, 5, &attn_m2);
    assert(ok_a2 == true && attn_m2.attention_decision == 3);

    // Case 3: Part-B (5) attending to context in Part-A (2) -> ALLOW (0)
    bool ok_a3 = auncient_glm_bidirectional_attention_mask_prover(8, 4, 5, 2, &attn_m3);
    assert(ok_a3 == true && attn_m3.attention_decision == 0);

    // Case 4: Part-B (5) attempting to attend forward to Part-B (6) -> PROHIBITED (3)
    bool ok_a4 = auncient_glm_bidirectional_attention_mask_prover(8, 4, 5, 6, &attn_m4);
    assert(ok_a4 == true && attn_m4.attention_decision == 3);

    printf("   ✓ GLM 2D Bidirectional Attention verified (Part-A Bidirectional=ALLOW, Part-A Peek=DENY, Part-B Context=ALLOW, Future Causal=DENY).\n");

    // 20. Test GLM Interleaved RMSNorm Scale Invariance & Unit Norm Prover
    printf("[TEST] Testing GLM Interleaved RMSNorm (Scale Invariance & Unit Norm Invariants)...\n");
    float x_in[4] = { 1.0f, 2.0f, -1.0f, -2.0f };
    AuncientGlmRmsNormMetrics rms_m = {0};
    bool ok_rms = auncient_glm_interleaved_rmsnorm_prover(x_in, 4, 2.5f /* alpha */, &rms_m);
    assert(ok_rms == true && rms_m.overall_rmsnorm_sound == true);
    assert(rms_m.scale_invariance_sound == true);
    assert(rms_m.unit_norm_sound == true);
    printf("   ✓ GLM Interleaved RMSNorm verified (RMS_orig=%.3f, RMS_scaled=%.3f, Out_RMS=%.3f, DispMod=%u).\n",
           rms_m.original_rms, rms_m.scaled_rms, rms_m.output_norm_rms, rms_m.displacement_wrap_mod);

    // 21. Test Primary-Secondary Accumulator Synthesis & Cascaded ACID Prover
    printf("[TEST] Testing Primary-Secondary Accumulator Synthesis (Cascaded ACID Invariance)...\n");
    AuncientSecondaryAccumulatorMetrics clean_synth_m = {0};
    bool ok_synth_clean = auncient_glm_secondary_accumulator_synthesis_prover(
        1000000 /* Saat */, 1000, 2000, 991220, false, 3 /* k=3 */, &clean_synth_m
    );
    assert(ok_synth_clean == true && clean_synth_m.overall_synthesis_sound == true);
    assert(clean_synth_m.primary_root_immutable_ok == true);
    assert(clean_synth_m.secondary_mu_valve == 0);

    AuncientSecondaryAccumulatorMetrics fault_synth_m = {0};
    bool ok_synth_fault = auncient_glm_secondary_accumulator_synthesis_prover(
        1000000 /* Saat */, 1000, 2000, 991220, true, 3 /* k=3 */, &fault_synth_m
    );
    assert(ok_synth_fault == true && fault_synth_m.overall_synthesis_sound == true);
    assert(fault_synth_m.primary_charge_mu0 == 1000000);
    assert(fault_synth_m.committed_secondary_rms == 0);
    assert(fault_synth_m.committed_secondary_valve == 0);
    printf("   ✓ Primary-Secondary Accumulator Synthesis verified (Primary Root=1M Saat, Sec_RMS=%lu, Fault Rollback=0, DispMod=%u).\n",
           clean_synth_m.secondary_mu_rms, clean_synth_m.displacement_wrap_mod);

    // 22. Test Transitive Secondary Accumulator Chain & Multi-Depth Rollback Prover
    printf("[TEST] Testing Transitive Secondary Chain (S1->S2->S3 Multi-Depth Rollback)...\n");
    AuncientTransitiveSecondaryMetrics clean_chain_m = {0};
    bool ok_chain_clean = auncient_glm_transitive_secondary_chain_prover(
        1000000 /* Saat */, 1000, 2000, 64 /* voxels */, 0 /* clean */, 3 /* k=3 */, &clean_chain_m
    );
    assert(ok_chain_clean == true && clean_chain_m.overall_chain_sound == true);
    assert(clean_chain_m.root_preserved_sound == true);
    assert(clean_chain_m.comm_s2 == 0);
    assert(clean_chain_m.comm_s3 == (64 * 256));

    AuncientTransitiveSecondaryMetrics fault_chain_m = {0};
    bool ok_chain_fault = auncient_glm_transitive_secondary_chain_prover(
        1000000 /* Saat */, 1000, 2000, 64 /* voxels */, 3 /* fault at leaf S3 */, 3 /* k=3 */, &fault_chain_m
    );
    assert(ok_chain_fault == true && fault_chain_m.overall_chain_sound == true);
    assert(fault_chain_m.root_charge_mu0 == 1000000);
    assert(fault_chain_m.comm_s1 == 0);
    assert(fault_chain_m.comm_s2 == 0);
    assert(fault_chain_m.comm_s3 == 0);
    printf("   ✓ Transitive Secondary Chain verified (Root=1M Saat, S1_RMS=%lu, S2_Valve=0, S3_SVDAG=%lu, Multi-Depth Rollback=0, DispMod=%u).\n",
           clean_chain_m.s1_rms, clean_chain_m.s3_svdag, clean_chain_m.displacement_wrap_mod);

    // 23. Test Universal Accumulator ACID Compliance & Transactional Rollback Prover
    printf("[TEST] Testing Universal Accumulator ACID Compliance (Shadow Isolation & Rollback)...\n");
    AuncientUniversalAccumulatorAcidMetrics clean_acid_m = {0};
    bool ok_acid_clean = auncient_glm_universal_accumulator_acid_prover(
        1000000 /* Saat */, 50 /* decay */, false /* clean commit */, 3 /* k=3 */, &clean_acid_m
    );
    assert(ok_acid_clean == true && clean_acid_m.overall_acid_sound == true);
    assert(clean_acid_m.isolation_sound == true);
    assert(clean_acid_m.consistency_inverse_sound == true);
    assert(clean_acid_m.atomicity_sound == true);
    assert(clean_acid_m.durability_sound == true);
    assert(clean_acid_m.committed_mu == clean_acid_m.staged_mu);

    AuncientUniversalAccumulatorAcidMetrics fault_acid_m = {0};
    bool ok_acid_fault = auncient_glm_universal_accumulator_acid_prover(
        1000000 /* Saat */, 50 /* decay */, true /* simulate fault */, 3 /* k=3 */, &fault_acid_m
    );
    assert(ok_acid_fault == true && fault_acid_m.overall_acid_sound == true);
    assert(fault_acid_m.isolation_sound == true);
    assert(fault_acid_m.atomicity_sound == true);
    assert(fault_acid_m.durability_sound == true);
    assert(fault_acid_m.committed_mu == fault_acid_m.shadow_mu);
    assert(fault_acid_m.committed_mu == 1000000);
    printf("   ✓ Universal Accumulator ACID verified (Init=1M Saat, Staged=%lu, Rec=%lu, Fault Rollback=%lu, DispMod=%u).\n",
           clean_acid_m.staged_mu, clean_acid_m.reconstructed_mu, fault_acid_m.committed_mu, clean_acid_m.displacement_wrap_mod);

    // 24. Test Harvard Computation Laboratory Suite (Mark I Wheel, Tape Ctrl, Bessel Recurrence, Cam Commutator)
    printf("[TEST] Testing Harvard Computation Laboratory (Mark I Wheels, Tape Latch, Bessel J0/J1, Commutator)...\n");
    AuncientHarvardLabMetrics harvard_clean_m = {0};
    bool ok_harvard_clean = auncient_harvard_computation_lab_prover(
        1000000 /* Saat */, 32768 /* x = 0.5 in Q16 */, false /* clean tape */, &harvard_clean_m
    );
    assert(ok_harvard_clean == true && harvard_clean_m.overall_harvard_sound == true);
    assert(harvard_clean_m.detent_sound == true);
    assert(harvard_clean_m.tape_execution_sound == true);
    assert(harvard_clean_m.recurrence_sound == true);
    assert(harvard_clean_m.commutator_t9_zero_sound == true);
    assert(harvard_clean_m.wheel_value_low == 1401876ULL);

    AuncientHarvardLabMetrics harvard_fault_m = {0};
    bool ok_harvard_fault = auncient_harvard_computation_lab_prover(
        1000000 /* Saat */, 32768 /* x = 0.5 in Q16 */, true /* simulate tape fault */, &harvard_fault_m
    );
    assert(ok_harvard_fault == true && harvard_fault_m.overall_harvard_sound == true);
    assert(harvard_fault_m.rollback_interlock_sound == true);
    printf("   ✓ Harvard Computation Lab verified (Wheel_Sum=%lu, Carry=%u, J0_Q16=%ld, J1_Q16=%ld, T9_Residual=0, DispMod=%u).\n",
           harvard_clean_m.wheel_value_low, harvard_clean_m.carry_overflow, harvard_clean_m.bessel_j0_fixed, harvard_clean_m.bessel_j1_fixed, harvard_clean_m.displacement_wrap_mod);

    // 25. Test Harvard H-Bridge Coupled Legendre 3-Term Recurrence Prover
    printf("[TEST] Testing Harvard H-Bridge Coupled Legendre Recurrence (Uniform Bound |P_n(x)| <= 1)...\n");
    AuncientHarvardLegendreMetrics leg_m = {0};
    bool ok_leg = auncient_harvard_legendre_recurrence_prover(
        32768 /* x = 0.5 in Q16 */, 4 /* max degree n = 4 */, 3 /* k=3 */, &leg_m
    );
    assert(ok_leg == true && leg_m.overall_legendre_sound == true);
    assert(leg_m.gating_sound == true);
    assert(leg_m.uniform_bound_sound == true);
    assert(leg_m.g_wmq_factor >= 875 && leg_m.g_wmq_factor <= 1000);
    assert(leg_m.p_degree[0] == 65536); // P_0 = 1.0
    printf("   ✓ Harvard Legendre Recurrence verified (x_in=0.5, G_wmq=%ld, P0=%ld, P1=%ld, P2=%ld, P3=%ld, P4=%ld, DispMod=%u).\n",
           leg_m.g_wmq_factor, leg_m.p_degree[0], leg_m.p_degree[1], leg_m.p_degree[2], leg_m.p_degree[3], leg_m.p_degree[4], leg_m.displacement_wrap_mod);

    // 26. Test Ballistic Orbit Recursive Zero-Copy Valve Prover (MIND Leaf [0..1023] Topography)
    printf("[TEST] Testing Ballistic Orbit Recursive Zero-Copy Valve (MIND Leaves & Rollback)...\n");
    AuncientBallisticOrbitValveMetrics clean_orbit_m = {0};
    bool ok_orbit_clean = auncient_ballistic_orbit_valve_prover(
        65536 /* r0 = 1.0 in Q16 */, 4096 /* dt = 1/16 */, 16384 /* v0 = 0.25 */, false /* clean */, 3 /* k=3 */, &clean_orbit_m
    );
    assert(ok_orbit_clean == true && clean_orbit_m.overall_orbit_sound == true);
    assert(clean_orbit_m.shadow_isolation_sound == true);
    assert(clean_orbit_m.valve_zero_flux_sound == true);
    assert(clean_orbit_m.committed_radius_q16 == clean_orbit_m.final_radius_q16);

    AuncientBallisticOrbitValveMetrics fault_orbit_m = {0};
    bool ok_orbit_fault = auncient_ballistic_orbit_valve_prover(
        65536 /* r0 = 1.0 in Q16 */, 4096 /* dt = 1/16 */, 16384 /* v0 = 0.25 */, true /* simulate fault */, 3 /* k=3 */, &fault_orbit_m
    );
    assert(ok_orbit_fault == true && fault_orbit_m.overall_orbit_sound == true);
    assert(fault_orbit_m.rollback_sound == true);
    assert(fault_orbit_m.committed_radius_q16 == 65536);
    printf("   ✓ Ballistic Orbit Valve verified (Periapsis=65536, Final_R=%ld, Fault_Rollback=%ld, DispMod=%u).\n",
           clean_orbit_m.final_radius_q16, fault_orbit_m.committed_radius_q16, clean_orbit_m.displacement_wrap_mod);

    // 27. Test Harvard 1946 Multiplier & Mechanical Dog Latch Prover
    printf("[TEST] Testing Harvard 1946 Multiplier (9-Step Digit Commutator & Mechanical Dog Latch)...\n");
    AuncientHarvard1946MultiplierMetrics clean_mult_m = {0};
    bool ok_mult_clean = auncient_harvard_1946_multiplier_prover(
        1000000 /* Saat */, 875, false /* clean */, 3 /* k=3 */, &clean_mult_m
    );
    assert(ok_mult_clean == true && clean_mult_m.overall_1946_sound == true);
    assert(clean_mult_m.commutator_sound == true);
    assert(clean_mult_m.shadow_detent_sound == true);
    assert(clean_mult_m.accumulated_product == 875000000ULL);

    AuncientHarvard1946MultiplierMetrics fault_mult_m = {0};
    bool ok_mult_fault = auncient_harvard_1946_multiplier_prover(
        1000000 /* Saat */, 875, true /* simulate tape tear */, 3 /* k=3 */, &fault_mult_m
    );
    assert(ok_mult_fault == true && fault_mult_m.overall_1946_sound == true);
    assert(fault_mult_m.mechanical_latch_sound == true);
    assert(fault_mult_m.committed_output == 1000000ULL);
    printf("   ✓ Harvard 1946 Multiplier verified (Product=%lu, Fault_Rollback=%lu, DispMod=%u).\n",
           clean_mult_m.accumulated_product, fault_mult_m.committed_output, clean_mult_m.displacement_wrap_mod);

    // 28. Test Harvard 1946 Functional Table Interpolator Tape Prover
    printf("[TEST] Testing Harvard 1946 Functional Interpolator (Forward Differences & Rollback)...\n");
    AuncientHarvard1946InterpolatorMetrics clean_interp_m = {0};
    bool ok_interp_clean = auncient_harvard_1946_interpolator_prover(
        0 /* x_0 = 0 */, 32768 /* dx = 0.5 in Q16 */, false /* clean */, 3 /* k=3 */, &clean_interp_m
    );
    assert(ok_interp_clean == true && clean_interp_m.overall_interpolator_sound == true);
    assert(clean_interp_m.difference_sound == true);
    assert(clean_interp_m.shadow_isolation_sound == true);
    assert(clean_interp_m.committed_output_q16 == clean_interp_m.interpolated_val_q16);

    AuncientHarvard1946InterpolatorMetrics fault_interp_m = {0};
    bool ok_interp_fault = auncient_harvard_1946_interpolator_prover(
        0 /* x_0 = 0 */, 32768 /* dx = 0.5 in Q16 */, true /* simulate tape skew */, 3 /* k=3 */, &fault_interp_m
    );
    assert(ok_interp_fault == true && fault_interp_m.overall_interpolator_sound == true);
    assert(fault_interp_m.rollback_sound == true);
    assert(fault_interp_m.committed_output_q16 == 65536);
    printf("   ✓ Harvard 1946 Interpolator verified (Interp_Val=%ld, Fault_Rollback=%ld, DispMod=%u).\n",
           clean_interp_m.interpolated_val_q16, fault_interp_m.committed_output_q16, clean_interp_m.displacement_wrap_mod);

    // 29. Test Harvard 1946 Relay Biquinary Code Parity & Interlock Prover
    printf("[TEST] Testing Harvard 1946 Relay Biquinary Parity (2-out-of-7 Code & Chatter Latch)...\n");
    AuncientHarvard1946BiquinaryMetrics clean_biquin_m = {0};
    bool ok_biquin_clean = auncient_harvard_1946_biquinary_prover(
        7 /* digit = 7 (5+2) */, false /* clean */, 3 /* k=3 */, &clean_biquin_m
    );
    assert(ok_biquin_clean == true && clean_biquin_m.overall_biquinary_sound == true);
    assert(clean_biquin_m.parity_sound == true);
    assert(clean_biquin_m.active_relay_count == 2);
    assert(clean_biquin_m.bi_part == 1 && clean_biquin_m.quin_part == 2);
    assert(clean_biquin_m.committed_output == 7);

    AuncientHarvard1946BiquinaryMetrics fault_biquin_m = {0};
    bool ok_biquin_fault = auncient_harvard_1946_biquinary_prover(
        7 /* digit = 7 */, true /* simulate contact chatter */, 3 /* k=3 */, &fault_biquin_m
    );
    assert(ok_biquin_fault == true && fault_biquin_m.overall_biquinary_sound == true);
    assert(fault_biquin_m.rollback_sound == true);
    assert(fault_biquin_m.committed_output == 7);
    printf("   ✓ Harvard 1946 Biquinary Parity verified (Digit=7, Active_Relays=2, Bi=1, Quin=2, DispMod=%u).\n",
           clean_biquin_m.displacement_wrap_mod);

    // 30. Test Harvard 1946 Double-Precision Subtractive Division Engine Prover
    printf("[TEST] Testing Harvard 1946 Subtractive Divider (Residue Conservation & Zero-Div Clutch)...\n");
    AuncientHarvard1946DividerMetrics clean_div_m = {0};
    bool ok_div_clean = auncient_harvard_1946_divider_prover(
        1000000 /* Saat */, 875, false /* clean */, 3 /* k=3 */, &clean_div_m
    );
    assert(ok_div_clean == true && clean_div_m.overall_divider_sound == true);
    assert(clean_div_m.residue_sound == true);
    assert(clean_div_m.remainder_bound_sound == true);
    assert(clean_div_m.shadow_isolation_sound == true);
    assert(clean_div_m.quotient_q == 1142);
    assert(clean_div_m.remainder_r == 750);
    assert(clean_div_m.committed_output == 1142);

    AuncientHarvard1946DividerMetrics fault_div_m = {0};
    bool ok_div_fault = auncient_harvard_1946_divider_prover(
        1000000 /* Saat */, 0 /* division by zero */, true /* simulate zero-div fault */, 3 /* k=3 */, &fault_div_m
    );
    assert(ok_div_fault == true && fault_div_m.overall_divider_sound == true);
    assert(fault_div_m.rollback_sound == true);
    assert(fault_div_m.committed_output == 1000000);
    printf("   ✓ Harvard 1946 Subtractive Divider verified (Q=%lu, R=%lu, Fault_Rollback=%lu, DispMod=%u).\n",
           clean_div_m.quotient_q, clean_div_m.remainder_r, fault_div_m.committed_output, clean_div_m.displacement_wrap_mod);

    // 31. Test Harvard 1946 Geneva-Drive Multi-Decade Ripple-Carry Prover
    printf("[TEST] Testing Harvard 1946 Geneva-Drive Ripple-Carry (999999 + 1 = 1000000 & Jam Latch)...\n");
    AuncientHarvard1946GenevaCarryMetrics clean_geneva_m = {0};
    bool ok_geneva_clean = auncient_harvard_1946_geneva_carry_prover(
        999999 /* base Saat */, 1 /* inc */, false /* clean */, 3 /* k=3 */, &clean_geneva_m
    );
    assert(ok_geneva_clean == true && clean_geneva_m.overall_geneva_sound == true);
    assert(clean_geneva_m.ripple_carry_sound == true);
    assert(clean_geneva_m.shadow_isolation_sound == true);
    assert(clean_geneva_m.accumulated_sum == 1000000);
    assert(clean_geneva_m.committed_output == 1000000);

    AuncientHarvard1946GenevaCarryMetrics fault_geneva_m = {0};
    bool ok_geneva_fault = auncient_harvard_1946_geneva_carry_prover(
        999999 /* base Saat */, 1 /* inc */, true /* simulate gear jam */, 3 /* k=3 */, &fault_geneva_m
    );
    assert(ok_geneva_fault == true && fault_geneva_m.overall_geneva_sound == true);
    assert(fault_geneva_m.rollback_sound == true);
    assert(fault_geneva_m.committed_output == 999999);
    printf("   ✓ Harvard 1946 Geneva-Drive Carry verified (Sum=%lu, Fault_Rollback=%lu, DispMod=%u).\n",
           clean_geneva_m.accumulated_sum, fault_geneva_m.committed_output, clean_geneva_m.displacement_wrap_mod);

    // 32. Test Harvard Zuo H-Bridge Quadrant Inversion Prover in Open Singularity
    printf("[TEST] Testing Harvard Zuo H-Bridge Quadrant Inversion (FET Commutation & Singularity)...\n");
    AuncientHarvardZuoHBridgeMetrics clean_zuo_m = {0};
    bool ok_zuo_clean = auncient_harvard_zuo_hbridge_quadrant_prover(
        5000 /* V_A mV */, 2172 /* V_B mV */, false /* clean */, 3 /* k=3 */, &clean_zuo_m
    );
    assert(ok_zuo_clean == true && clean_zuo_m.overall_zuo_hbridge_sound == true);
    assert(clean_zuo_m.quadrant_inversion_sound == true);
    assert(clean_zuo_m.gating_clamp_sound == true);
    assert(clean_zuo_m.shadow_isolation_sound == true);
    assert(clean_zuo_m.g_gate_forward >= 875 && clean_zuo_m.g_gate_forward <= 1000);

    AuncientHarvardZuoHBridgeMetrics fault_zuo_m = {0};
    bool ok_zuo_fault = auncient_harvard_zuo_hbridge_quadrant_prover(
        5000 /* V_A mV */, 2172 /* V_B mV */, true /* simulate arm short */, 3 /* k=3 */, &fault_zuo_m
    );
    assert(ok_zuo_fault == true && fault_zuo_m.overall_zuo_hbridge_sound == true);
    assert(fault_zuo_m.rollback_sound == true);
    assert(fault_zuo_m.committed_output == 5000);
    printf("   ✓ Harvard Zuo H-Bridge Quadrant verified (V_diff=2828mV, G_gate=%ld, Out=%ld, DispMod=%u).\n",
           clean_zuo_m.g_gate_forward, clean_zuo_m.committed_output, clean_zuo_m.displacement_wrap_mod);

    // 33. Test Harvard Zuo Dual-Tape Cross-Feed Monotonicity Prover
    printf("[TEST] Testing Harvard Zuo Dual-Tape Sync (Banach Leaf Stride & Skew Latch)...\n");
    AuncientHarvardZuoTapeSyncMetrics clean_tape_m = {0};
    bool ok_tape_clean = auncient_harvard_zuo_tape_sync_prover(
        1000000 /* Leaf Saat */, 16 /* stride */, false /* clean */, 3 /* k=3 */, &clean_tape_m
    );
    assert(ok_tape_clean == true && clean_tape_m.overall_tape_sync_sound == true);
    assert(clean_tape_m.phase_lock_sound == true);
    assert(clean_tape_m.stride_bound_sound == true);
    assert(clean_tape_m.shadow_isolation_sound == true);
    assert(clean_tape_m.argument_index_out == 62500);
    assert(clean_tape_m.committed_output == 62500);

    AuncientHarvardZuoTapeSyncMetrics fault_tape_m = {0};
    bool ok_tape_fault = auncient_harvard_zuo_tape_sync_prover(
        1000000 /* Leaf Saat */, 16 /* stride */, true /* simulate tape skew */, 3 /* k=3 */, &fault_tape_m
    );
    assert(ok_tape_fault == true && fault_tape_m.overall_tape_sync_sound == true);
    assert(fault_tape_m.rollback_sound == true);
    assert(fault_tape_m.committed_output == 1000000);
    printf("   ✓ Harvard Zuo Dual-Tape Sync verified (Arg_Index=%lu, Fault_Rollback=%lu, DispMod=%u).\n",
           clean_tape_m.argument_index_out, fault_tape_m.committed_output, clean_tape_m.displacement_wrap_mod);

    // 34. Test Harvard Zuo Pluggable Patchboard Permutation Automorphism Prover
    printf("[TEST] Testing Harvard Zuo Plugboard Permutation (24-Channel Conservation & Short Latch)...\n");
    AuncientHarvardZuoPlugboardMetrics clean_plug_m = {0};
    bool ok_plug_clean = auncient_harvard_zuo_plugboard_prover(
        1000000 /* Seed Saat */, false /* clean */, 3 /* k=3 */, &clean_plug_m
    );
    assert(ok_plug_clean == true && clean_plug_m.overall_plugboard_sound == true);
    assert(clean_plug_m.bijectivity_sound == true);
    assert(clean_plug_m.shadow_isolation_sound == true);
    assert(clean_plug_m.sum_input_channels == clean_plug_m.sum_permuted_channels);
    assert(clean_plug_m.sum_permuted_channels == 24000276ULL);
    assert(clean_plug_m.committed_output == 24000276ULL);

    AuncientHarvardZuoPlugboardMetrics fault_plug_m = {0};
    bool ok_plug_fault = auncient_harvard_zuo_plugboard_prover(
        1000000 /* Seed Saat */, true /* simulate cross-talk fault */, 3 /* k=3 */, &fault_plug_m
    );
    assert(ok_plug_fault == true && fault_plug_m.overall_plugboard_sound == true);
    assert(fault_plug_m.rollback_sound == true);
    assert(fault_plug_m.committed_output == 1000000ULL);
    printf("   ✓ Harvard Zuo Plugboard Permutation verified (Sum_24_Ch=%lu, Fault_Rollback=%lu, DispMod=%u).\n",
           clean_plug_m.sum_permuted_channels, fault_plug_m.committed_output, clean_plug_m.displacement_wrap_mod);

    // 35. Test Harvard Zuo Modified Airy-Hankel 3-Term Recurrence Prover
    printf("[TEST] Testing Harvard Zuo Modified Airy-Hankel Recurrence (h1/h2 Stability & Singularity Latch)...\n");
    AuncientHarvardZuoHankelMetrics clean_hankel_m = {0};
    bool ok_hankel_clean = auncient_harvard_zuo_hankel_prover(
        65536 /* z = 1.0 in Q16 */, false /* clean */, 3 /* k=3 */, &clean_hankel_m
    );
    assert(ok_hankel_clean == true && clean_hankel_m.overall_hankel_sound == true);
    assert(clean_hankel_m.stability_bound_sound == true);
    assert(clean_hankel_m.gating_clamp_sound == true);
    assert(clean_hankel_m.shadow_isolation_sound == true);
    assert(clean_hankel_m.h3_q16 >= -131072LL && clean_hankel_m.h3_q16 <= 131072LL);

    AuncientHarvardZuoHankelMetrics fault_hankel_m = {0};
    bool ok_hankel_fault = auncient_harvard_zuo_hankel_prover(
        65536 /* z = 1.0 in Q16 */, true /* simulate singularity fault */, 3 /* k=3 */, &fault_hankel_m
    );
    assert(ok_hankel_fault == true && fault_hankel_m.overall_hankel_sound == true);
    assert(fault_hankel_m.rollback_sound == true);
    assert(fault_hankel_m.committed_output == 65536LL);
    printf("   ✓ Harvard Zuo Hankel Recurrence verified (h3_Q16=%ld, G_gate=%ld, Fault_Rollback=%ld, DispMod=%u).\n",
           clean_hankel_m.h3_q16, clean_hankel_m.g_gate_q16, fault_hankel_m.committed_output, clean_hankel_m.displacement_wrap_mod);

    printf("=============================================================\n");
    printf("ALL EDSAC-AUTODIN COMPILER FIREWALL TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}













