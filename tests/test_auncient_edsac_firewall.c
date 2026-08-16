#include "../tsfi2-deepseek/src/auncient_edsac_firewall.h"
#include "../tsfi2-deepseek/src/auncient_timeline_autodin.h"
#include "../tsfi2-deepseek/inc/auncient_harvard_computation_lab.h"
#include "../tsfi2-deepseek/inc/tsfi_displacementshader.h"
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

    // 1-5. Verify firewall authorization, PKI, analyzer, and packet evaluation
    assert(auncient_firewall_init(firewall_rules_tape, 500, valid_pki, 4, NULL) == false);
    auncient_autodin_edsac_authorize(true);
    assert(auncient_firewall_init(firewall_rules_tape, 500, invalid_pki, 3, NULL) == false);

    AuncientAnalyzer analyzer;
    auncient_analyzer_init(&analyzer, 1 << ('S' - 'A'));
    assert(auncient_firewall_init(firewall_rules_tape, 500, valid_pki, 4, &analyzer) == false);

    uint32_t sample_instruction = (('F' & 0xFF) << 24) | (80 << 2);
    assert(auncient_analyzer_classify(&analyzer, &sample_instruction, 1) == true);
    assert(auncient_firewall_init(firewall_rules_tape, 500, valid_pki, 4, NULL) == true);

    AuncientPacket p1 = { .source_ip = 0x0A000001, .dest_port = 80, .payload_checksum = 0xAA55, .key_count = 4 };
    AuncientPacket p2 = { .source_ip = 0x0A000001, .dest_port = 22, .payload_checksum = 0xBB66, .key_count = 4 };
    AuncientPacket p3 = { .source_ip = 0x0A000001, .dest_port = 8080, .payload_checksum = 0xCC77, .key_count = 4 }; 
    AuncientPacket p4 = { .source_ip = 0x0A000001, .dest_port = 443, .payload_checksum = 0xDD88, .key_count = 4 };
    assert(auncient_firewall_eval_packet(&p1) == true && auncient_firewall_eval_packet(&p2) == false);
    assert(auncient_firewall_eval_packet(&p3) == false && auncient_firewall_eval_packet(&p4) == true);

    // 6. Relocate rules using Initial Orders 2 coordinate shifts
    auncient_firewall_relocate_rules(1000);
    AuncientPacket p4_old = { .source_ip = 0x0A000001, .dest_port = 443, .payload_checksum = 0xDD88, .key_count = 4 };
    AuncientPacket p4_new = { .source_ip = 0x0A000001, .dest_port = 1443, .payload_checksum = 0xDD88, .key_count = 4 };
    assert(auncient_firewall_eval_packet(&p4_old) == false && auncient_firewall_eval_packet(&p4_new) == true && auncient_firewall_eval_packet(&p1) == true);
    printf("   ✓ Firewall, PKI, and Initial Orders 2 relocation verified.\n");

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

    AuncientInitialOrders1GateContext forged_ctx = valid_ctx;
    forged_ctx.autodin_receipt = 12345;
    assert(auncient_initial_orders_1_verify_autodin_prerequisite(&forged_ctx, test_insts, 2, &ruling) == false && ruling == 1);

    AuncientInitialOrders1GateContext broken_rec_ctx = valid_ctx;
    broken_rec_ctx.previous_nonce = 999999;
    assert(auncient_initial_orders_1_verify_autodin_prerequisite(&broken_rec_ctx, test_insts, 2, &ruling) == false && ruling == 3);

    uint32_t bad_insts[2] = { 0x41000140, 0x58000010 /* Op='X' */ };
    assert(auncient_initial_orders_1_verify_autodin_prerequisite(&valid_ctx, bad_insts, 2, &ruling) == false && ruling == 2);
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
    AuncientGlmZorseMetrics glm_zorse_m = {0};
    assert(auncient_glm_zorse_multitask_prover(7, 875, 128, 80, &glm_zorse_m) == true && glm_zorse_m.overall_zorse_multitask_sound == true);

    // 16. Test GLM 2D Positional Encoding Prover for Zorse
    AuncientGlmZorse2DPosMetrics glm_pos2d_m = {0};
    uint32_t test_linear_pos = (2 * 64) + 17;
    assert(auncient_glm_zorse_2d_position_prover(test_linear_pos, 2, 64, 80, &glm_pos2d_m) == true && glm_pos2d_m.overall_2d_position_sound == true);

    // 17. Test GLM FET Link Dynamics Prover
    AuncientGlmFetLinkMetrics glm_fet_m = {0};
    assert(auncient_glm_fet_link_dynamics_prover(1000000, 50, 5, 3, &glm_fet_m) == true && glm_fet_m.overall_fet_link_sound == true);

    // 18. Test Accumulator Valves upon Zero TOTIENT ACID Compliance & Rollback
    AuncientTotientValveAcidMetrics clean_valve_m = {0}, fault_valve_m = {0};
    assert(auncient_glm_totient_valve_acid_prover(0, 1000000, 991220, false, 3, &clean_valve_m) == true && clean_valve_m.overall_valve_acid_sound == true);
    assert(auncient_glm_totient_valve_acid_prover(0, 1000000, 991220, true, 3, &fault_valve_m) == true && fault_valve_m.durability_rollback_verified == true);
    printf("   ✓ GLM Multi-Task, 2D Positional, FET Link Dynamics, and Valve ACID verified.\n");

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
    AuncientSecondaryAccumulatorMetrics clean_synth_m = {0}, fault_synth_m = {0};
    bool ok_synth_clean = auncient_glm_secondary_accumulator_synthesis_prover(1000000, 1000, 2000, 991220, false, 3, &clean_synth_m);
    assert(ok_synth_clean == true && clean_synth_m.overall_synthesis_sound == true && clean_synth_m.primary_root_immutable_ok == true && clean_synth_m.secondary_mu_valve == 0);
    bool ok_synth_fault = auncient_glm_secondary_accumulator_synthesis_prover(1000000, 1000, 2000, 991220, true, 3, &fault_synth_m);
    assert(ok_synth_fault == true && fault_synth_m.overall_synthesis_sound == true && fault_synth_m.primary_charge_mu0 == 1000000 && fault_synth_m.committed_secondary_rms == 0);
    printf("   ✓ Primary-Secondary Accumulator Synthesis verified (Primary Root=1M Saat, Sec_RMS=%lu, Fault Rollback=0, DispMod=%u).\n",
           clean_synth_m.secondary_mu_rms, clean_synth_m.displacement_wrap_mod);

    // 22. Test Transitive Secondary Accumulator Chain & Multi-Depth Rollback Prover
    printf("[TEST] Testing Transitive Secondary Chain (S1->S2->S3 Multi-Depth Rollback)...\n");
    AuncientTransitiveSecondaryMetrics clean_chain_m = {0}, fault_chain_m = {0};
    bool ok_chain_clean = auncient_glm_transitive_secondary_chain_prover(1000000, 1000, 2000, 64, 0, 3, &clean_chain_m);
    assert(ok_chain_clean == true && clean_chain_m.overall_chain_sound == true && clean_chain_m.root_preserved_sound == true && clean_chain_m.comm_s3 == (64 * 256));
    bool ok_chain_fault = auncient_glm_transitive_secondary_chain_prover(1000000, 1000, 2000, 64, 3, 3, &fault_chain_m);
    assert(ok_chain_fault == true && fault_chain_m.overall_chain_sound == true && fault_chain_m.root_charge_mu0 == 1000000 && fault_chain_m.comm_s1 == 0 && fault_chain_m.comm_s3 == 0);
    printf("   ✓ Transitive Secondary Chain verified (Root=1M Saat, S1_RMS=%lu, S2_Valve=0, S3_SVDAG=%lu, Multi-Depth Rollback=0, DispMod=%u).\n",
           clean_chain_m.s1_rms, clean_chain_m.s3_svdag, clean_chain_m.displacement_wrap_mod);

    // 23. Test Universal Accumulator ACID Compliance & Transactional Rollback Prover
    printf("[TEST] Testing Universal Accumulator ACID Compliance (Shadow Isolation & Rollback)...\n");
    AuncientUniversalAccumulatorAcidMetrics clean_acid_m = {0}, fault_acid_m = {0};
    bool ok_acid_clean = auncient_glm_universal_accumulator_acid_prover(1000000, 50, false, 3, &clean_acid_m);
    assert(ok_acid_clean == true && clean_acid_m.overall_acid_sound == true && clean_acid_m.committed_mu == clean_acid_m.staged_mu);
    bool ok_acid_fault = auncient_glm_universal_accumulator_acid_prover(1000000, 50, true, 3, &fault_acid_m);
    assert(ok_acid_fault == true && fault_acid_m.overall_acid_sound == true && fault_acid_m.committed_mu == 1000000);
    printf("   ✓ Universal Accumulator ACID verified (Init=1M Saat, Staged=%lu, Rec=%lu, Fault Rollback=%lu, DispMod=%u).\n",
           clean_acid_m.staged_mu, clean_acid_m.reconstructed_mu, fault_acid_m.committed_mu, clean_acid_m.displacement_wrap_mod);

    // 24. Test Harvard Computation Laboratory Suite (Mark I Wheel, Tape Ctrl, Bessel Recurrence, Cam Commutator)
    printf("[TEST] Testing Harvard Computation Laboratory (Mark I Wheels, Tape Latch, Bessel J0/J1, Commutator)...\n");
    AuncientHarvardLabMetrics harvard_clean_m = {0};
    bool ok_harvard_clean = auncient_harvard_computation_lab_prover(1000000, 32768, false, &harvard_clean_m);
    assert(ok_harvard_clean == true && harvard_clean_m.overall_harvard_sound == true && harvard_clean_m.recurrence_sound == true);
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
    AuncientHarvard1946MultiplierMetrics clean_mult_m = {0}, fault_mult_m = {0};
    bool ok_mult_clean = auncient_harvard_1946_multiplier_prover(1000000, 875, false, 3, &clean_mult_m);
    assert(ok_mult_clean == true && clean_mult_m.overall_1946_sound == true && clean_mult_m.accumulated_product == 875000000ULL);
    bool ok_mult_fault = auncient_harvard_1946_multiplier_prover(1000000, 875, true, 3, &fault_mult_m);
    assert(ok_mult_fault == true && fault_mult_m.overall_1946_sound == true && fault_mult_m.committed_output == 1000000ULL);
    printf("   ✓ Harvard 1946 Multiplier verified (Product=%lu, Fault_Rollback=%lu, DispMod=%u).\n",
           clean_mult_m.accumulated_product, fault_mult_m.committed_output, clean_mult_m.displacement_wrap_mod);

    // 28. Test Harvard 1946 Functional Table Interpolator Tape Prover
    printf("[TEST] Testing Harvard 1946 Functional Interpolator (Forward Differences & Rollback)...\n");
    AuncientHarvard1946InterpolatorMetrics clean_interp_m = {0}, fault_interp_m = {0};
    bool ok_interp_clean = auncient_harvard_1946_interpolator_prover(0, 32768, false, 3, &clean_interp_m);
    assert(ok_interp_clean == true && clean_interp_m.overall_interpolator_sound == true && clean_interp_m.committed_output_q16 == clean_interp_m.interpolated_val_q16);
    bool ok_interp_fault = auncient_harvard_1946_interpolator_prover(0, 32768, true, 3, &fault_interp_m);
    assert(ok_interp_fault == true && fault_interp_m.overall_interpolator_sound == true && fault_interp_m.committed_output_q16 == 65536);
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


    // 30. Test Harvard 1946 Subtractive Divider & Zero-Division Clutch Prover
    printf("[TEST] Testing Harvard 1946 Subtractive Divider (Residue Conservation & Zero-Div Clutch)...\n");
    AuncientHarvard1946DividerMetrics clean_div_m = {0}, fault_div_m = {0};
    bool ok_div_clean = auncient_harvard_1946_divider_prover(1000000, 875, false, 3, &clean_div_m);
    assert(ok_div_clean == true && clean_div_m.overall_divider_sound == true && clean_div_m.quotient_q == 1142 && clean_div_m.remainder_r == 750);
    bool ok_div_fault = auncient_harvard_1946_divider_prover(1000000, 0, true, 3, &fault_div_m);
    assert(ok_div_fault == true && fault_div_m.overall_divider_sound == true && fault_div_m.committed_output == 1000000);
    printf("   ✓ Harvard 1946 Subtractive Divider verified (Q=%lu, R=%lu, Fault_Rollback=%lu, DispMod=%u).\n",
           clean_div_m.quotient_q, clean_div_m.remainder_r, fault_div_m.committed_output, clean_div_m.displacement_wrap_mod);

    // 31. Test Harvard 1946 Geneva-Drive Multi-Decade Ripple-Carry Prover
    printf("[TEST] Testing Harvard 1946 Geneva-Drive Ripple-Carry (999999 + 1 = 1000000 & Jam Latch)...\n");
    AuncientHarvard1946GenevaCarryMetrics clean_geneva_m = {0}, fault_geneva_m = {0};
    bool ok_geneva_clean = auncient_harvard_1946_geneva_carry_prover(999999, 1, false, 3, &clean_geneva_m);
    assert(ok_geneva_clean == true && clean_geneva_m.overall_geneva_sound == true && clean_geneva_m.accumulated_sum == 1000000);
    bool ok_geneva_fault = auncient_harvard_1946_geneva_carry_prover(999999, 1, true, 3, &fault_geneva_m);
    assert(ok_geneva_fault == true && fault_geneva_m.overall_geneva_sound == true && fault_geneva_m.committed_output == 999999);
    printf("   ✓ Harvard 1946 Geneva-Drive Carry verified (Sum=%lu, Fault_Rollback=%lu, DispMod=%u).\n",
           clean_geneva_m.accumulated_sum, fault_geneva_m.committed_output, clean_geneva_m.displacement_wrap_mod);

    // 32. Test Harvard Zuo H-Bridge Quadrant Inversion Prover in Open Singularity
    printf("[TEST] Testing Harvard Zuo H-Bridge Quadrant Inversion (FET Commutation & Singularity)...\n");
    AuncientHarvardZuoHBridgeMetrics clean_zuo_m = {0}, fault_zuo_m = {0};
    bool ok_zuo_clean = auncient_harvard_zuo_hbridge_quadrant_prover(5000, 2172, false, 3, &clean_zuo_m);
    assert(ok_zuo_clean == true && clean_zuo_m.overall_zuo_hbridge_sound == true);
    bool ok_zuo_fault = auncient_harvard_zuo_hbridge_quadrant_prover(5000, 2172, true, 3, &fault_zuo_m);
    assert(ok_zuo_fault == true && fault_zuo_m.overall_zuo_hbridge_sound == true && fault_zuo_m.committed_output == 5000);
    printf("   ✓ Harvard Zuo H-Bridge Quadrant verified (V_diff=%ldmV, G_gate=%ld, Out=%ld, DispMod=%u).\n",
           clean_zuo_m.v_diff_forward, clean_zuo_m.g_gate_forward, clean_zuo_m.committed_output, clean_zuo_m.displacement_wrap_mod);



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

    // 36. Test Harvard Zuo Self-Referential Commutator Identity Prover
    printf("[TEST] Testing Harvard Zuo Self-Identity (Zero-Flux Persistence & Trip Latch)...\n");
    AuncientHarvardZuoSelfIdentityMetrics clean_self_m = {0};
    bool ok_self_clean = auncient_harvard_zuo_self_identity_prover(
        1000000 /* Saat seed */, 10 /* cycles */, false /* clean */, 3 /* k=3 */, &clean_self_m
    );
    assert(ok_self_clean == true && clean_self_m.overall_self_identity_sound == true);
    assert(clean_self_m.self_identity_sound == true);
    assert(clean_self_m.shadow_isolation_sound == true);
    assert(clean_self_m.final_state == 1000000ULL);
    assert(clean_self_m.committed_output == 1000000ULL);

    AuncientHarvardZuoSelfIdentityMetrics fault_self_m = {0};
    bool ok_self_fault = auncient_harvard_zuo_self_identity_prover(
        1000000 /* Saat seed */, 10 /* cycles */, true /* simulate clutch trip fault */, 3 /* k=3 */, &fault_self_m
    );
    assert(ok_self_fault == true && fault_self_m.overall_self_identity_sound == true);
    assert(fault_self_m.rollback_sound == true);
    assert(fault_self_m.committed_output == 1000000ULL);
    printf("   ✓ Harvard Zuo Self-Identity verified (Final_State=%lu, Fault_Rollback=%lu, DispMod=%u).\n",
           clean_self_m.final_state, fault_self_m.committed_output, clean_self_m.displacement_wrap_mod);

    // 37. Test Harvard Zuo Multi-Tier Torque Balance Prover
    printf("[TEST] Testing Harvard Zuo Multi-Tier Torque Balance (Push-Pull Equilibrium & Imbalance Latch)...\n");
    AuncientHarvardZuoTorqueMetrics clean_torque_m = {0};
    bool ok_torque_clean = auncient_harvard_zuo_torque_balance_prover(
        4000 /* Arm1 mA */, 4000 /* Arm2 mA */, false /* clean */, 3 /* k=3 */, &clean_torque_m
    );
    assert(ok_torque_clean == true && clean_torque_m.overall_torque_sound == true);
    assert(clean_torque_m.torque_balance_sound == true);
    assert(clean_torque_m.gating_clamp_sound == true);
    assert(clean_torque_m.shadow_isolation_sound == true);
    assert(clean_torque_m.g_gate_factor >= 875 && clean_torque_m.g_gate_factor <= 1000);

    AuncientHarvardZuoTorqueMetrics fault_torque_m = {0};
    bool ok_torque_fault = auncient_harvard_zuo_torque_balance_prover(
        4000 /* Arm1 mA */, 4000 /* Arm2 mA */, true /* simulate imbalance fault */, 3 /* k=3 */, &fault_torque_m
    );
    assert(ok_torque_fault == true && fault_torque_m.overall_torque_sound == true);
    assert(fault_torque_m.rollback_sound == true);
    printf("   ✓ Harvard Zuo Torque Balance verified (Total_I=%ldmA, G_gate=%ld, Out=%ld, DispMod=%u).\n",
           clean_torque_m.total_current_ma, clean_torque_m.g_gate_factor, clean_torque_m.committed_output, clean_torque_m.displacement_wrap_mod);

    // 38. Test Harvard Zuo Modified Cylindrical Bessel Recurrence Prover
    printf("[TEST] Testing Harvard Zuo Modified Cylindrical Bessel (K3 Bound & Pole Singularity Latch)...\n");
    AuncientHarvardZuoBesselMetrics clean_bess_m = {0};
    bool ok_bess_clean = auncient_harvard_zuo_bessel_modified_prover(
        65536 /* x = 1.0 in Q16 */, false /* clean */, 3 /* k=3 */, &clean_bess_m
    );
    assert(ok_bess_clean == true && clean_bess_m.overall_bessel_sound == true);
    assert(clean_bess_m.stability_bound_sound == true);
    assert(clean_bess_m.gating_clamp_sound == true);
    assert(clean_bess_m.shadow_isolation_sound == true);
    assert(clean_bess_m.k3_q16 >= -524288LL && clean_bess_m.k3_q16 <= 524288LL);

    AuncientHarvardZuoBesselMetrics fault_bess_m = {0};
    bool ok_bess_fault = auncient_harvard_zuo_bessel_modified_prover(
        65536 /* x = 1.0 in Q16 */, true /* simulate pole fault */, 3 /* k=3 */, &fault_bess_m
    );
    assert(ok_bess_fault == true && fault_bess_m.overall_bessel_sound == true);
    assert(fault_bess_m.rollback_sound == true);
    assert(fault_bess_m.committed_output == 65536LL);
    printf("   ✓ Harvard Zuo Modified Bessel verified (K3_Q16=%ld, G_gate=%ld, Fault_Rollback=%ld, DispMod=%u).\n",
           clean_bess_m.k3_q16, clean_bess_m.g_gate_q16, fault_bess_m.committed_output, clean_bess_m.displacement_wrap_mod);

    // 39. Test Harvard Zuo Continuous Tape Loop Topology Invariance Prover
    printf("[TEST] Testing Harvard Zuo Tape Loop Topology (Winding Number Invariance & Splice Latch)...\n");
    AuncientHarvardZuoTapeLoopMetrics clean_loop_m = {0};
    bool ok_loop_clean = auncient_harvard_zuo_tape_loop_prover(
        64 /* loop steps */, 16 /* revs */, false /* clean */, 3 /* k=3 */, &clean_loop_m
    );
    assert(ok_loop_clean == true && clean_loop_m.overall_tape_loop_sound == true);
    assert(clean_loop_m.topological_homology_sound == true);
    assert(clean_loop_m.gating_clamp_sound == true);
    assert(clean_loop_m.shadow_isolation_sound == true);
    assert(clean_loop_m.step_accumulator == 1024ULL);

    AuncientHarvardZuoTapeLoopMetrics fault_loop_m = {0};
    bool ok_loop_fault = auncient_harvard_zuo_tape_loop_prover(
        64 /* loop steps */, 16 /* revs */, true /* simulate splice fault */, 3 /* k=3 */, &fault_loop_m
    );
    assert(ok_loop_fault == true && fault_loop_m.overall_tape_loop_sound == true);
    assert(fault_loop_m.rollback_sound == true);
    assert(fault_loop_m.committed_output == 64ULL);
    printf("   ✓ Harvard Zuo Tape Loop Topology verified (Step_Acc=%lu, G_gate=%ld, Out=%lu, DispMod=%u).\n",
           clean_loop_m.step_accumulator, clean_loop_m.g_gate_factor, clean_loop_m.committed_output, clean_loop_m.displacement_wrap_mod);

    // 40. Test Harvard Zuo 2-out-of-5 Teleprinter Code Parity Prover
    printf("[TEST] Testing Harvard Zuo 2-out-of-5 Parity (Constant Weight 2 & Bit-Flip Latch)...\n");
    AuncientHarvardZuo2of5Metrics clean_2of5_m = {0};
    bool ok_2of5_clean = auncient_harvard_zuo_two_out_of_five_prover(
        7 /* digit */, false /* clean */, 3 /* k=3 */, &clean_2of5_m
    );
    assert(ok_2of5_clean == true && clean_2of5_m.overall_2of5_sound == true);
    assert(clean_2of5_m.hamming_weight_sound == true);
    assert(clean_2of5_m.gating_clamp_sound == true);
    assert(clean_2of5_m.shadow_isolation_sound == true);
    assert(clean_2of5_m.code_word == 18);
    assert(clean_2of5_m.active_hamming_weight == 2);

    AuncientHarvardZuo2of5Metrics fault_2of5_m = {0};
    bool ok_2of5_fault = auncient_harvard_zuo_two_out_of_five_prover(
        7 /* digit */, true /* simulate bit-flip fault */, 3 /* k=3 */, &fault_2of5_m
    );
    assert(ok_2of5_fault == true && fault_2of5_m.overall_2of5_sound == true);
    assert(fault_2of5_m.rollback_sound == true);
    printf("   ✓ Harvard Zuo 2-out-of-5 Parity verified (Code=%u, Weight=%u, Out=%lu, DispMod=%u).\n",
           clean_2of5_m.code_word, clean_2of5_m.active_hamming_weight, clean_2of5_m.committed_output, clean_2of5_m.displacement_wrap_mod);

    // 41. Test Harvard Zuo 24-Decade Universal Transfer Bus Invariance Prover
    printf("[TEST] Testing Harvard Zuo 24-Decade Transfer Bus (64-bit Potential & Short Latch)...\n");
    AuncientHarvardZuoBusMetrics clean_bus_m = {0};
    bool ok_bus_clean = auncient_harvard_zuo_transfer_bus_prover(
        1000000 /* Saat source */, 24 /* decades */, false /* clean */, 3 /* k=3 */, &clean_bus_m
    );
    assert(ok_bus_clean == true && clean_bus_m.overall_bus_sound == true);
    assert(clean_bus_m.bus_transfer_sound == true);
    assert(clean_bus_m.gating_clamp_sound == true);
    assert(clean_bus_m.shadow_isolation_sound == true);
    assert(clean_bus_m.dest_saat_value == 1000000ULL);
    assert(clean_bus_m.g_gate_factor == 1000);

    AuncientHarvardZuoBusMetrics fault_bus_m = {0};
    bool ok_bus_fault = auncient_harvard_zuo_transfer_bus_prover(
        1000000 /* Saat source */, 24 /* decades */, true /* simulate bus short fault */, 3 /* k=3 */, &fault_bus_m
    );
    assert(ok_bus_fault == true && fault_bus_m.overall_bus_sound == true);
    assert(fault_bus_m.rollback_sound == true);
    assert(fault_bus_m.committed_output == 1000000ULL);
    printf("   ✓ Harvard Zuo 24-Decade Transfer Bus verified (Dest=%lu, G_gate=%ld, Out=%lu, DispMod=%u).\n",
           clean_bus_m.dest_saat_value, clean_bus_m.g_gate_factor, clean_bus_m.committed_output, clean_bus_m.displacement_wrap_mod);

    // 42. Test Harvard Zuo Main Drive Constant Angular Momentum Invariance Prover
    printf("[TEST] Testing Harvard Zuo Angular Momentum (172.5 RPM & Motor Stall Latch)...\n");
    AuncientHarvardZuoMomentumMetrics clean_mom_m = {0};
    bool ok_mom_clean = auncient_harvard_zuo_angular_momentum_prover(
        1725 /* 172.5 RPM in tenths */, 20 /* revs */, false /* clean */, 3 /* k=3 */, &clean_mom_m
    );
    assert(ok_mom_clean == true && clean_mom_m.overall_momentum_sound == true);
    assert(clean_mom_m.angular_velocity_sound == true);
    assert(clean_mom_m.gating_clamp_sound == true);
    assert(clean_mom_m.shadow_isolation_sound == true);
    assert(clean_mom_m.final_rpm_tenths == 1725);
    assert(clean_mom_m.g_gate_factor == 900);

    AuncientHarvardZuoMomentumMetrics fault_mom_m = {0};
    bool ok_mom_fault = auncient_harvard_zuo_angular_momentum_prover(
        1725 /* 172.5 RPM in tenths */, 20 /* revs */, true /* simulate stall fault */, 3 /* k=3 */, &fault_mom_m
    );
    assert(ok_mom_fault == true && fault_mom_m.overall_momentum_sound == true);
    assert(fault_mom_m.rollback_sound == true);
    assert(fault_mom_m.committed_output == 1725ULL);
    printf("   ✓ Harvard Zuo Angular Momentum verified (RPM_Tenths=%u, G_gate=%ld, Out=%lu, DispMod=%u).\n",
           clean_mom_m.final_rpm_tenths, clean_mom_m.g_gate_factor, clean_mom_m.committed_output, clean_mom_m.displacement_wrap_mod);

    // 43. Test Harvard Zuo Multi-Level Subroutine Cascade Gating Prover
    printf("[TEST] Testing Harvard Zuo Subroutine Cascade (Nested Stack Automorphism & Overflow Latch)...\n");
    AuncientHarvardZuoCascadeMetrics clean_casc_m = {0};
    bool ok_casc_clean = auncient_harvard_zuo_subroutine_cascade_prover(
        43605 /* Entry coordinate */, 4 /* depth */, false /* clean */, 3 /* k=3 */, &clean_casc_m
    );
    assert(ok_casc_clean == true && clean_casc_m.overall_cascade_sound == true);
    assert(clean_casc_m.return_coordinate_sound == true);
    assert(clean_casc_m.gating_clamp_sound == true);
    assert(clean_casc_m.shadow_isolation_sound == true);
    assert(clean_casc_m.return_coordinate == 43605ULL);
    assert(clean_casc_m.g_gate_factor == 937);

    AuncientHarvardZuoCascadeMetrics fault_casc_m = {0};
    bool ok_casc_fault = auncient_harvard_zuo_subroutine_cascade_prover(
        43605 /* Entry coordinate */, 4 /* depth */, true /* simulate stack fault */, 3 /* k=3 */, &fault_casc_m
    );
    assert(ok_casc_fault == true && fault_casc_m.overall_cascade_sound == true);
    assert(fault_casc_m.rollback_sound == true);
    assert(fault_casc_m.committed_output == 43605ULL);
    printf("   ✓ Harvard Zuo Subroutine Cascade verified (Return_Coord=%lu, G_gate=%ld, Out=%lu, DispMod=%u).\n",
           clean_casc_m.return_coordinate, clean_casc_m.g_gate_factor, clean_casc_m.committed_output, clean_casc_m.displacement_wrap_mod);

    // 44. Test Harvard Zuo 24-Decade Complementary Nine's Carry Invariance Prover
    printf("[TEST] Testing Harvard Zuo Nine's Complement (Direct/Modular Subtraction & Borrow Latch)...\n");
    AuncientHarvardZuoNinesMetrics clean_nines_m = {0};
    bool ok_nines_clean = auncient_harvard_zuo_nines_complement_prover(
        1000000 /* Minuend */, 43605 /* Subtrahend */, false /* clean */, 3 /* k=3 */, &clean_nines_m
    );
    assert(ok_nines_clean == true && clean_nines_m.overall_nines_sound == true);
    assert(clean_nines_m.modular_equivalence_sound == true);
    assert(clean_nines_m.gating_clamp_sound == true);
    assert(clean_nines_m.shadow_isolation_sound == true);
    assert(clean_nines_m.direct_diff_val == 956395ULL);
    assert(clean_nines_m.modular_diff_val == 956395ULL);

    AuncientHarvardZuoNinesMetrics fault_nines_m = {0};
    bool ok_nines_fault = auncient_harvard_zuo_nines_complement_prover(
        1000000 /* Minuend */, 43605 /* Subtrahend */, true /* simulate borrow fault */, 3 /* k=3 */, &fault_nines_m
    );
    assert(ok_nines_fault == true && fault_nines_m.overall_nines_sound == true);
    assert(fault_nines_m.rollback_sound == true);
    assert(fault_nines_m.committed_output == 1000000ULL);
    printf("   ✓ Harvard Zuo Nine's Complement verified (Diff=%lu, Modular=%lu, Out=%lu, DispMod=%u).\n",
           clean_nines_m.direct_diff_val, clean_nines_m.modular_diff_val, clean_nines_m.committed_output, clean_nines_m.displacement_wrap_mod);

    // 45. Test Harvard Zuo Dual Cam Timing Matrix Orthogonality Prover
    printf("[TEST] Testing Harvard Zuo Dual Cam Matrix (180° Mechanical Orthogonality & Collision Latch)...\n");
    AuncientHarvardZuoCamMetrics clean_cam_m = {0};
    bool ok_cam_clean = auncient_harvard_zuo_dual_cam_matrix_prover(
        2 /* t2 */, 7 /* p7 */, false /* clean */, 3 /* k=3 */, &clean_cam_m
    );
    assert(ok_cam_clean == true && clean_cam_m.overall_cam_sound == true);
    assert(clean_cam_m.phase_orthogonality_sound == true);
    assert(clean_cam_m.gating_clamp_sound == true);
    assert(clean_cam_m.shadow_isolation_sound == true);
    assert(clean_cam_m.phase_difference == 5);
    assert(clean_cam_m.g_gate_factor == 908);

    AuncientHarvardZuoCamMetrics fault_cam_m = {0};
    bool ok_cam_fault = auncient_harvard_zuo_dual_cam_matrix_prover(
        2 /* t2 */, 7 /* p7 */, true /* simulate collision fault */, 3 /* k=3 */, &fault_cam_m
    );
    assert(ok_cam_fault == true && fault_cam_m.overall_cam_sound == true);
    assert(fault_cam_m.rollback_sound == true);
    assert(fault_cam_m.committed_output == 2ULL);
    printf("   ✓ Harvard Zuo Dual Cam Matrix verified (t=%u, p=%u, Diff=%u, Out=%lu, DispMod=%u).\n",
           clean_cam_m.t_cam_phase, clean_cam_m.p_cam_phase, clean_cam_m.phase_difference, clean_cam_m.committed_output, clean_cam_m.displacement_wrap_mod);

    // 46. Test Harvard Zuo Initial Orders 1 Bootstrap Readiness Prover
    printf("[TEST] Testing Harvard Zuo Initial Orders 1 Bootstrap (31-Word Delay-Line Recirculation & Dispersion Latch)...\n");
    AuncientHarvardZuoOrders1Metrics clean_boot_m = {0};
    bool ok_boot_clean = auncient_harvard_zuo_orders1_bootstrap_prover(
        31 /* words */, 50 /* cycles */, false /* clean */, 3 /* k=3 */, &clean_boot_m
    );
    assert(ok_boot_clean == true && clean_boot_m.overall_bootstrap_sound == true);
    assert(clean_boot_m.delay_recirculation_sound == true);
    assert(clean_boot_m.gating_clamp_sound == true);
    assert(clean_boot_m.shadow_isolation_sound == true);
    assert(clean_boot_m.initial_checksum == 21628080ULL);
    assert(clean_boot_m.recirc_checksum == 21628080ULL);
    assert(clean_boot_m.g_gate_factor == 1000);

    AuncientHarvardZuoOrders1Metrics fault_boot_m = {0};
    bool ok_boot_fault = auncient_harvard_zuo_orders1_bootstrap_prover(
        31 /* words */, 50 /* cycles */, true /* simulate dispersion fault */, 3 /* k=3 */, &fault_boot_m
    );
    assert(ok_boot_fault == true && fault_boot_m.overall_bootstrap_sound == true);
    assert(fault_boot_m.rollback_sound == true);
    assert(fault_boot_m.committed_output == 31ULL);
    printf("   ✓ Harvard Zuo Initial Orders 1 Bootstrap verified (Words=%u, Cycles=%u, Checksum=%lu, Out=%lu, DispMod=%u).\n",
           clean_boot_m.bootstrap_word_count, clean_boot_m.recirculation_cycles, clean_boot_m.initial_checksum, clean_boot_m.committed_output, clean_boot_m.displacement_wrap_mod);

    // 47. Test Harvard Zuo Word Coupling Safety & Space Partition Invariance Prover
    printf("[TEST] Testing Harvard Zuo Word Coupling Safety (17-bit to 35-bit Partition & Bleed Latch)...\n");
    AuncientHarvardZuoCouplingMetrics clean_coupl_m = {0};
    bool ok_coupl_clean = auncient_harvard_zuo_word_coupling_prover(
        43605 /* Low */, 87210 /* High */, false /* clean */, 3 /* k=3 */, &clean_coupl_m
    );
    assert(ok_coupl_clean == true && clean_coupl_m.overall_coupling_sound == true);
    assert(clean_coupl_m.reversible_coupling_sound == true);
    assert(clean_coupl_m.gating_clamp_sound == true);
    assert(clean_coupl_m.shadow_isolation_sound == true);
    assert(clean_coupl_m.reconstructed_low == 43605);
    assert(clean_coupl_m.reconstructed_high == 87210);
    assert(clean_coupl_m.coupled_long_word == 22861621845ULL);

    AuncientHarvardZuoCouplingMetrics fault_coupl_m = {0};
    bool ok_coupl_fault = auncient_harvard_zuo_word_coupling_prover(
        43605 /* Low */, 87210 /* High */, true /* simulate bleed fault */, 3 /* k=3 */, &fault_coupl_m
    );
    assert(ok_coupl_fault == true && fault_coupl_m.overall_coupling_sound == true);
    assert(fault_coupl_m.rollback_sound == true);
    assert(fault_coupl_m.committed_output == 43605ULL);
    printf("   ✓ Harvard Zuo Word Coupling Safety verified (Low=%u, High=%u, LongWord=%lu, Out=%lu, DispMod=%u).\n",
           clean_coupl_m.reconstructed_low, clean_coupl_m.reconstructed_high, clean_coupl_m.coupled_long_word, clean_coupl_m.committed_output, clean_coupl_m.displacement_wrap_mod);

    // 48. Test Harvard Zuo Telephone Uniselector Rotary Stepping Synchronization Prover
    printf("[TEST] Testing Harvard Zuo Telephone Uniselector Sync (0..30 Rotary Step & Bounce Latch)...\n");
    AuncientHarvardZuoUniselectorMetrics clean_uni_m = {0};
    bool ok_uni_clean = auncient_harvard_zuo_uniselector_sync_prover(
        5 /* starting step */, 31 /* impulses */, false /* clean */, 3 /* k=3 */, &clean_uni_m
    );
    assert(ok_uni_clean == true && clean_uni_m.overall_uniselector_sound == true);
    assert(clean_uni_m.rotary_stepping_sound == true);
    assert(clean_uni_m.gating_clamp_sound == true);
    assert(clean_uni_m.shadow_isolation_sound == true);
    assert(clean_uni_m.final_wiper_position == 5);
    assert(clean_uni_m.g_gate_factor == 913);

    AuncientHarvardZuoUniselectorMetrics fault_uni_m = {0};
    bool ok_uni_fault = auncient_harvard_zuo_uniselector_sync_prover(
        5 /* starting step */, 31 /* impulses */, true /* simulate bounce fault */, 3 /* k=3 */, &fault_uni_m
    );
    assert(ok_uni_fault == true && fault_uni_m.overall_uniselector_sound == true);
    assert(fault_uni_m.rollback_sound == true);
    assert(fault_uni_m.committed_output == 5ULL);
    printf("   ✓ Harvard Zuo Telephone Uniselector Sync verified (Start=%u, Impulses=%u, FinalPos=%u, Out=%lu, DispMod=%u).\n",
           clean_uni_m.starting_wiper_step, clean_uni_m.impulse_count, clean_uni_m.final_wiper_position, clean_uni_m.committed_output, clean_uni_m.displacement_wrap_mod);

    // 49. Test Harvard Zuo Wheeler Jump Subroutine Return Link Invariance Prover
    printf("[TEST] Testing Harvard Zuo Wheeler Jump Linkage (Self-Modifying Return Coordinate & Escape Latch)...\n");
    AuncientHarvardZuoWheelerMetrics clean_wh_m = {0};
    bool ok_wh_clean = auncient_harvard_zuo_wheeler_jump_prover(
        100 /* caller PC */, 500 /* entry PC */, false /* clean */, 3 /* k=3 */, &clean_wh_m
    );
    assert(ok_wh_clean == true && clean_wh_m.overall_wheeler_sound == true);
    assert(clean_wh_m.return_link_sound == true);
    assert(clean_wh_m.gating_clamp_sound == true);
    assert(clean_wh_m.shadow_isolation_sound == true);
    assert(clean_wh_m.wheeler_link_instruction == 4653158);
    assert(clean_wh_m.expected_return_pc == 102);
    assert(clean_wh_m.resolved_return_pc == 102);
    assert(clean_wh_m.g_gate_factor == 887);

    AuncientHarvardZuoWheelerMetrics fault_wh_m = {0};
    bool ok_wh_fault = auncient_harvard_zuo_wheeler_jump_prover(
        100 /* caller PC */, 500 /* entry PC */, true /* simulate escape fault */, 3 /* k=3 */, &fault_wh_m
    );
    assert(ok_wh_fault == true && fault_wh_m.overall_wheeler_sound == true);
    assert(fault_wh_m.rollback_sound == true);
    assert(fault_wh_m.committed_output == 100ULL);
    printf("   ✓ Harvard Zuo Wheeler Jump Linkage verified (Caller=%u, Entry=%u, LinkInstr=%u, RetPC=%u, Out=%lu, DispMod=%u).\n",
           clean_wh_m.caller_pc, clean_wh_m.subroutine_entry_pc, clean_wh_m.wheeler_link_instruction, clean_wh_m.resolved_return_pc, clean_wh_m.committed_output, clean_wh_m.displacement_wrap_mod);

    // 50. Test Golden Jubilee Analog Voltage Overdrive & Word Coupling Breakup in DisplacementShader
    printf("[TEST] Testing Golden Jubilee Analog Voltage Overdrive (Pure Non-Accumulating Wave Breakup)...\n");
    TSFiDisplacementShader ds_overdrive;
    tsfi_displacementshader_init(&ds_overdrive, 1.0, 1.0);
    double harmonic_dispersion = 0.0;
    double clean_disp = tsfi_displacementshader_eval_jubilee_overdrive(&ds_overdrive, 50000.0, 131071.0, &harmonic_dispersion);
    assert(harmonic_dispersion == 0.0);
    assert(clean_disp >= 0.0);

    double clipped_disp = tsfi_displacementshader_eval_jubilee_overdrive(&ds_overdrive, 250000.0, 131071.0, &harmonic_dispersion);
    assert(harmonic_dispersion > 0.0); // Overdrive harmonic distortion present
    assert(clipped_disp >= 0.0);
    printf("   ✓ Golden Jubilee Analog Voltage Overdrive verified (CleanDisp=%.4f, ClippedDisp=%.4f, Harmonics=%.2f V).\n",
           clean_disp, clipped_disp, harmonic_dispersion);

    // 51. Test Harvard Zuo 5-Hole Paper Tape Mechanical Sensing Pin Matrix Prover
    printf("[TEST] Testing Harvard Zuo 5-Hole Sensing Pin Matrix (Bijective Mechanical Punch & Gating)...\n");
    AuncientHarvardZuoSensingPinMetrics clean_pin_m = {0};
    bool ok_pin_clean = auncient_harvard_zuo_sensing_pin_matrix_prover(
        19 /* Punch 10011b (19) */, false /* clean */, 3 /* k=3 */, &clean_pin_m
    );
    assert(ok_pin_clean == true && clean_pin_m.overall_sensing_pin_sound == true);
    assert(clean_pin_m.reversible_sensing_sound == true);
    assert(clean_pin_m.gating_clamp_sound == true);
    assert(clean_pin_m.shadow_isolation_sound == true);
    assert(clean_pin_m.pin_p0 == 1 && clean_pin_m.pin_p1 == 1 && clean_pin_m.pin_p2 == 0 && clean_pin_m.pin_p3 == 0 && clean_pin_m.pin_p4 == 1);
    assert(clean_pin_m.reconstructed_mask == 19);
    assert(clean_pin_m.g_gate_factor == 951);

    AuncientHarvardZuoSensingPinMetrics fault_pin_m = {0};
    bool ok_pin_fault = auncient_harvard_zuo_sensing_pin_matrix_prover(
        19 /* Punch 10011b */, true /* simulate pin fault */, 3 /* k=3 */, &fault_pin_m
    );
    assert(ok_pin_fault == true && fault_pin_m.overall_sensing_pin_sound == true);
    assert(fault_pin_m.rollback_sound == true);
    assert(fault_pin_m.committed_output == 19ULL);
    printf("   ✓ Harvard Zuo 5-Hole Sensing Pin Matrix verified (Punch=%u, P0..P4=%u%u%u%u%u, G_gate=%ld, Out=%lu, DispMod=%u).\n",
           clean_pin_m.input_punch_mask, clean_pin_m.pin_p0, clean_pin_m.pin_p1, clean_pin_m.pin_p2, clean_pin_m.pin_p3, clean_pin_m.pin_p4,
           clean_pin_m.g_gate_factor, clean_pin_m.committed_output, clean_pin_m.displacement_wrap_mod);

    // 52. Test Harvard Zuo CICS Operative Jump & Initial Orders 1 Discovery Prover
    printf("[TEST] Testing Harvard Zuo CICS Operative Jump & Initial Orders 1 Discovery Prover...\n");
    AuncientHarvardZuoCicsJumpMetrics clean_cics_m = {0};
    bool ok_cics_clean = auncient_harvard_zuo_cics_wheeler_jump_prover(
        100 /* Base PC */, 50 /* Theta offset */, false /* clean */, 3 /* k=3 */, &clean_cics_m
    );
    assert(ok_cics_clean == true && clean_cics_m.overall_cics_jump_sound == true);
    assert(clean_cics_m.address_relocation_sound == true);
    assert(clean_cics_m.cics_jump_sound == true);
    assert(clean_cics_m.effective_address == 150);
    assert(clean_cics_m.target_pc == 150);
    assert(clean_cics_m.cics_vector == 49605);
    assert(clean_cics_m.g_gate_factor == 893);
    assert(clean_cics_m.committed_output == 172889);

    AuncientHarvardZuoCicsJumpMetrics fault_cics_m = {0};
    bool ok_cics_fault = auncient_harvard_zuo_cics_wheeler_jump_prover(
        100 /* Base PC */, 50 /* Theta offset */, true /* simulate CICS exception */, 3 /* k=3 */, &fault_cics_m
    );
    assert(ok_cics_fault == true && fault_cics_m.overall_cics_jump_sound == true);
    assert(fault_cics_m.rollback_sound == true);
    assert(fault_cics_m.target_pc == 49605);
    assert(fault_cics_m.committed_output == 49605ULL);
    printf("   ✓ Harvard Zuo CICS Initial Orders 1 Jump verified (Base=%u, Theta=%u, Effective=%u, CICS_Vector=0x%X, Out=%lu, DispMod=%u).\n",
           clean_cics_m.base_address, clean_cics_m.theta_offset, clean_cics_m.effective_address, clean_cics_m.cics_vector,
           clean_cics_m.committed_output, clean_cics_m.displacement_wrap_mod);

    // -------------------------------------------------------------
    // TEST 53: Formal Suite 53: Marschner Dual-Layer Fur Anisotropic Scattering Prover
    // -------------------------------------------------------------
    printf("[TEST] Testing Marschner Dual-Layer Fur Anisotropic Scattering (Primary R + Secondary TRT)...\n");
    AuncientMarschnerFurMetrics clean_fur_m = {0};
    bool ok_fur_clean = auncient_marschner_fur_scattering_prover(
        32768 /* dot_tl = 0.5 in Q16 */, 44257 /* clumping seed */, false /* normal */, 3 /* k=3 */, &clean_fur_m
    );
    assert(ok_fur_clean == true && clean_fur_m.overall_marschner_sound == true);
    assert(clean_fur_m.energy_conservation_sound == true);
    assert(clean_fur_m.gating_clamp_sound == true);
    assert(clean_fur_m.shadow_isolation_sound == true);
    assert(clean_fur_m.total_spec_q16 <= 65536);

    AuncientMarschnerFurMetrics fault_fur_m = {0};
    bool ok_fur_fault = auncient_marschner_fur_scattering_prover(
        32768 /* dot_tl */, 44257 /* seed */, true /* simulate light singularity */, 3 /* k=3 */, &fault_fur_m
    );
    assert(ok_fur_fault == true && fault_fur_m.overall_marschner_sound == true);
    assert(fault_fur_m.rollback_sound == true);
    assert(fault_fur_m.committed_output == 32768ULL);
    printf("   ✓ Marschner Fur Anisotropic Scattering verified (dot_tl=0.5, Spec_Total_Q16=%ld, G_gate=%ld, Out=%lu, DispMod=%u).\n",
           clean_fur_m.total_spec_q16, clean_fur_m.g_gate_factor, clean_fur_m.committed_output, clean_fur_m.displacement_wrap_mod);

    // -------------------------------------------------------------
    // TEST 54: Formal Suite 54: Super 8 Mechanical Camera & Hudson HuC6280/VDC/VCE Prover
    // -------------------------------------------------------------
    printf("[TEST] Testing Super 8 Mechanical Camera (180° Shutter, Geneva Claw) & Hudson ICs (HuC6270 VDC, HuC6260 VCE)...\n");
    AuncientSuper8HudsonMetrics clean_s8_m = {0};
    bool ok_s8_clean = auncient_super8_hudson_renderman_prover(
        180 /* 180-deg shutter */, 0 /* Phase 0: locked registration pin */, 320 /* VDC clip X */, 256 /* VCE palette index */,
        false /* normal */, 3 /* k=3 */, &clean_s8_m
    );
    assert(ok_s8_clean == true && clean_s8_m.overall_super8_hudson_sound == true);
    assert(clean_s8_m.rotary_shutter_sound == true);
    assert(clean_s8_m.claw_advance_sound == true);
    assert(clean_s8_m.vdc_raster_clip_sound == true);
    assert(clean_s8_m.vce_palette_sound == true);
    assert(clean_s8_m.gating_clamp_sound == true);
    assert(clean_s8_m.shadow_isolation_sound == true);

    AuncientSuper8HudsonMetrics fault_s8_m = {0};
    bool ok_s8_fault = auncient_super8_hudson_renderman_prover(
        180, 0, 320, 256, true /* simulate film jam fault */, 3 /* k=3 */, &fault_s8_m
    );
    assert(ok_s8_fault == true && fault_s8_m.overall_super8_hudson_sound == true);
    assert(fault_s8_m.rollback_sound == true);
    assert(fault_s8_m.committed_output == 180ULL);
    printf("   ✓ Super 8 & Hudson RenderMan verified (Shutter=180°, VDC_Clip=320, VCE_Palette=256, G_gate=%ld, Out=%lu, DispMod=%u).\n",
           clean_s8_m.g_gate_factor, clean_s8_m.committed_output, clean_s8_m.displacement_wrap_mod);

    // -------------------------------------------------------------
    // TEST 55: Formal Suite 55: Dual-Pass SSS Petal Venation & Fresnel Sky Rim Prover
    // -------------------------------------------------------------
    printf("[TEST] Testing Dual-Pass SSS Petal Venation (Through-Edge + Internal Core) & Fresnel Rim...\n");
    AuncientDualPassSSSMetrics clean_sss_m = {0};
    bool ok_sss_clean = auncient_dual_pass_sss_venation_prover(131072, 16384, false, 3, &clean_sss_m);
    assert(ok_sss_clean == true && clean_sss_m.overall_sss_sound == true);
    assert(clean_sss_m.energy_conservation_sound == true && clean_sss_m.total_radiance_q16 <= 196608);

    AuncientDualPassSSSMetrics fault_sss_m = {0};
    bool ok_sss_fault = auncient_dual_pass_sss_venation_prover(131072, 16384, true, 3, &fault_sss_m);
    assert(ok_sss_fault == true && fault_sss_m.overall_sss_sound == true);
    assert(fault_sss_m.rollback_sound == true && fault_sss_m.committed_output == 131072ULL);
    printf("   ✓ Dual-Pass SSS Petal Venation verified (Density=2.0, Radiance_Total=%ld, G_gate=%ld, Out=%lu, DispMod=%u).\n",
           clean_sss_m.total_radiance_q16, clean_sss_m.g_gate_factor, clean_sss_m.committed_output, clean_sss_m.displacement_wrap_mod);

    // -------------------------------------------------------------
    // TEST 56: Formal Suite 56: Vulkan Vision Scope & Frustum Camera Operation Prover
    // -------------------------------------------------------------
    printf("[TEST] Testing Vulkan Vision Scope (Descriptors Set 0/1/2) & 3D Frustum Camera Operation...\n");
    AuncientVulkanCameraMetrics clean_vulk_m = {0};
    bool ok_vulk_clean = auncient_vulkan_vision_camera_prover(60 /* 60 deg FOV */, 0 /* Set 0 */, 800 /* Depth Z */, false, 3, &clean_vulk_m);
    assert(ok_vulk_clean == true && clean_vulk_m.overall_vulkan_camera_sound == true);
    assert(clean_vulk_m.focal_frustum_sound == true);
    assert(clean_vulk_m.depth_monotonic_sound == true);
    assert(clean_vulk_m.descriptor_layout_sound == true);
    assert(clean_vulk_m.gating_clamp_sound == true);
    assert(clean_vulk_m.shadow_isolation_sound == true);

    AuncientVulkanCameraMetrics fault_vulk_m = {0};
    bool ok_vulk_fault = auncient_vulkan_vision_camera_prover(60, 0, 800, true /* simulate frustum fault */, 3, &fault_vulk_m);
    assert(ok_vulk_fault == true && fault_vulk_m.overall_vulkan_camera_sound == true);
    assert(fault_vulk_m.rollback_sound == true && fault_vulk_m.committed_output == 60ULL);
    printf("   ✓ Vulkan Vision & Camera Operation verified (FOV=60°, Depth=800, G_gate=%ld, Out=%lu, DispMod=%u).\n",
           clean_vulk_m.g_gate_factor, clean_vulk_m.committed_output, clean_vulk_m.displacement_wrap_mod);

    // -------------------------------------------------------------
    // TEST 57: Formal Suite 57: 5-Instrument Overdrive Synthesizer & Interleaved RMSNorm Prover
    // -------------------------------------------------------------
    printf("[TEST] Testing 5-Instrument Overdrive Synthesizer (Sub-Bass, FM, Lead, Kick, Snare) & RMSNorm...\n");
    AuncientBionikaSynthMetrics clean_bio_m = {0};
    bool ok_bio_clean = auncient_bionika_synth_overdrive_prover(1 /* Track 1 Sub-Bass */, 55 /* 55Hz */, 55705 /* 0.85 Q16 */, false, 3, &clean_bio_m);
    assert(ok_bio_clean == true && clean_bio_m.overall_bionika_synth_sound == true);
    assert(clean_bio_m.harmonic_bound_sound == true);
    assert(clean_bio_m.energy_conservation_sound == true);
    assert(clean_bio_m.gating_clamp_sound == true);
    assert(clean_bio_m.shadow_isolation_sound == true);

    AuncientBionikaSynthMetrics fault_bio_m = {0};
    bool ok_bio_fault = auncient_bionika_synth_overdrive_prover(1, 55, 55705, true /* simulate overdrive fault */, 3, &fault_bio_m);
    assert(ok_bio_fault == true && fault_bio_m.overall_bionika_synth_sound == true);
    assert(fault_bio_m.rollback_sound == true && fault_bio_m.committed_output == 55705ULL);
    printf("   ✓ 5-Instrument Overdrive Synth & RMSNorm verified (Track=1, Freq=55Hz, G_gate=%ld, Out=%lu, DispMod=%u).\n",
           clean_bio_m.g_gate_factor, clean_bio_m.committed_output, clean_bio_m.displacement_wrap_mod);

    // -------------------------------------------------------------
    // TEST 58: Formal Suite 58: UsdShade FET Discharge Soft-Body Verlet Prover
    // -------------------------------------------------------------
    printf("[TEST] Testing UsdShade FET Discharge Soft-Body Verlet (Mass-Spring Relaxation & Gating)...\n");
    AuncientUsdShadeVerletMetrics clean_usd_m = {0};
    bool ok_usd_clean = auncient_usdshade_fet_verlet_prover(4 /* 4x4 Grid */, 98304 /* 1.5 Q16 */, 65536 /* 1.0 Q16 */, false, 3, &clean_usd_m);
    assert(ok_usd_clean == true && clean_usd_m.overall_usdshade_sound == true);
    assert(clean_usd_m.verlet_bound_sound == true && clean_usd_m.spring_relaxation_sound == true);
    assert(clean_usd_m.gating_clamp_sound == true && clean_usd_m.shadow_isolation_sound == true);

    AuncientUsdShadeVerletMetrics fault_usd_m = {0};
    bool ok_usd_fault = auncient_usdshade_fet_verlet_prover(4, 98304, 65536, true /* simulate lattice fault */, 3, &fault_usd_m);
    assert(ok_usd_fault == true && fault_usd_m.overall_usdshade_sound == true);
    assert(fault_usd_m.rollback_sound == true && fault_usd_m.committed_output == 65536ULL);
    printf("   ✓ UsdShade FET Verlet verified (Grid=4x4, RestLen=1.5, G_gate=%ld, Out=%lu, DispMod=%u).\n",
           clean_usd_m.g_gate_factor, clean_usd_m.committed_output, clean_usd_m.displacement_wrap_mod);

    // -------------------------------------------------------------
    // TEST 59: Formal Suite 59: CADE IMF NATO Tactical Slot Boundary & Ramp Threshold Prover
    // -------------------------------------------------------------
    printf("[TEST] Testing CADE IMF NATO Tactical Slot Boundary & Ramp Thresholds (Timing & Gating)...\n");
    AuncientCadeImfNatoSlotMetrics clean_nato_m = {0};
    bool ok_nato_clean = auncient_cade_imf_nato_slot_prover(1 /* Slot 1 */, 6 /* 6ms Offset */, 5 /* 5ms Up */, 10 /* 10ms Down */, false, 3, &clean_nato_m);
    assert(ok_nato_clean == true && clean_nato_m.overall_nato_slot_sound == true);
    assert(clean_nato_m.slot_boundary_sound == true && clean_nato_m.ramp_threshold_sound == true);
    assert(clean_nato_m.gating_clamp_sound == true && clean_nato_m.shadow_isolation_sound == true);

    AuncientCadeImfNatoSlotMetrics fault_nato_m = {0};
    bool ok_nato_fault = auncient_cade_imf_nato_slot_prover(1, 6, 5, 10, true /* simulate timing fault */, 3, &fault_nato_m);
    assert(ok_nato_fault == true && fault_nato_m.overall_nato_slot_sound == true);
    assert(fault_nato_m.rollback_sound == true && fault_nato_m.committed_output == 6ULL);
    printf("   ✓ CADE IMF NATO Slot Timing verified (Slot=1, Offset=6ms, Up=5ms, Down=10ms, G_gate=%ld, Out=%lu, DispMod=%u).\n",
           clean_nato_m.g_gate_factor, clean_nato_m.committed_output, clean_nato_m.displacement_wrap_mod);

    // 60. Test Teddy Bear Participant DNA Seed & Regional SSA Endowment Prover
    printf("[TEST] Testing Teddy Bear Participant DNA Seed (FNV-1a Area Code & 1,000,000 Saat Baseline)...\n");
    AuncientTeddyBearSsaMetrics clean_tb_m = {0}, fault_tb_m = {0};
    bool ok_tb_clean = auncient_teddy_bear_ssa_endowment_prover(43605, 1000000ULL, false, 3, &clean_tb_m);
    assert(ok_tb_clean == true && clean_tb_m.overall_teddy_bear_sound == true && clean_tb_m.area_code == 1);
    bool ok_tb_fault = auncient_teddy_bear_ssa_endowment_prover(43605, 1000000ULL, true, 3, &fault_tb_m);
    assert(ok_tb_fault == true && fault_tb_m.overall_teddy_bear_sound == true && fault_tb_m.committed_output == 43605ULL);
    printf("   ✓ Teddy Bear SSA Endowment verified (Area=1, G_gate=%ld, Out=%lu, Fault_Rollback=%lu, DispMod=%u).\n",
           clean_tb_m.g_gate_factor, clean_tb_m.committed_output, fault_tb_m.committed_output, clean_tb_m.displacement_wrap_mod);

    printf("=============================================================\n");
    printf("ALL EDSAC-AUTODIN COMPILER FIREWALL TESTS PASSED SUCCESSFULLY (60/60)\n");
    printf("=============================================================\n");
    return 0;
}
