#ifndef AUNCIENT_HARVARD_COMPUTATION_LAB_H
#define AUNCIENT_HARVARD_COMPUTATION_LAB_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define HARVARD_WHEEL_DIGITS 24
#define HARVARD_COMMUTATOR_PHASES 10
#define MOTZKIN_PRIME_REGISTER 953467954ULL

/* 1. Mark I 24-Digit Mechanical Counter Wheel */
typedef struct {
    uint8_t digits[HARVARD_WHEEL_DIGITS];
    bool sign; // false = positive, true = negative
    bool detent_zero_locked;
    uint32_t carry_cascade_count;
} AuncientCounterWheel;

/* 2. Harvard 3-Address Sequence-Control Tape Record */
typedef struct {
    uint8_t out_register; // Source register index (0..15)
    uint8_t in_register;  // Destination register index (0..15)
    uint8_t opcode;       // 1 = Add, 2 = Sub, 3 = Mult, 4 = Clear-Add, 5 = Valve-Gating
    uint8_t modifier;     // Sequence control modifier
} AuncientTapeInstruction;

typedef struct {
    AuncientTapeInstruction tape[256];
    size_t tape_length;
    size_t program_counter;
    size_t shadow_pc;
    bool latch_trip_active;
} AuncientSequenceTape;

/* 3. Cam-Timed Commutator & Valve Interrupter */
typedef struct {
    uint32_t current_phase; // 0..9
    uint64_t stroke_charge_mu;
    uint64_t staged_potential_eta;
    uint64_t residual_flux;
    bool valve_closed_at_t9;
} AuncientCommutatorValve;

/* Metrics structure for test validation */
typedef struct {
    uint64_t wheel_value_low;
    uint32_t carry_overflow;
    bool detent_sound;
    bool tape_execution_sound;
    bool rollback_interlock_sound;
    int64_t bessel_j0_fixed;
    int64_t bessel_j1_fixed;
    bool recurrence_sound;
    bool commutator_t9_zero_sound;
    uint32_t displacement_wrap_mod;
    bool overall_harvard_sound;
} AuncientHarvardLabMetrics;

/* API Declarations */
void auncient_harvard_wheel_init(AuncientCounterWheel *wheel);
bool auncient_harvard_wheel_set_uint64(AuncientCounterWheel *wheel, uint64_t val);
uint64_t auncient_harvard_wheel_get_uint64(const AuncientCounterWheel *wheel);
bool auncient_harvard_wheel_add_step(AuncientCounterWheel *dest, const AuncientCounterWheel *src);

void auncient_harvard_tape_init(AuncientSequenceTape *tape_ctrl);
bool auncient_harvard_tape_load(AuncientSequenceTape *tape_ctrl, const AuncientTapeInstruction *insts, size_t count);
bool auncient_harvard_tape_step(AuncientSequenceTape *tape_ctrl, uint64_t *registers, size_t reg_count);
void auncient_harvard_tape_trigger_fault(AuncientSequenceTape *tape_ctrl, uint64_t *registers, const uint64_t *shadow_registers, size_t reg_count);

int64_t auncient_harvard_bessel_j0_fixed(int64_t x_fixed_q16);
int64_t auncient_harvard_bessel_j1_fixed(int64_t x_fixed_q16);
bool auncient_harvard_bessel_recurrence_verify(int64_t x_fixed_q16, int64_t j0, int64_t j1);

void auncient_harvard_commutator_init(AuncientCommutatorValve *valve, uint64_t charge_mu);
bool auncient_harvard_commutator_step_phase(AuncientCommutatorValve *valve);

/* Unified Prover */
bool auncient_harvard_computation_lab_prover(
    uint64_t initial_charge_mu,
    int64_t test_x_q16,
    bool simulate_tape_fault,
    AuncientHarvardLabMetrics *metrics_out
);

typedef struct {
    int64_t input_x_q16;
    int64_t gated_x_q16;
    int64_t g_wmq_factor;
    int64_t p_degree[8];
    uint32_t max_degree_n;
    bool gating_sound;
    bool uniform_bound_sound;
    uint32_t displacement_wrap_mod;
    bool overall_legendre_sound;
} AuncientHarvardLegendreMetrics;

bool auncient_harvard_legendre_recurrence_prover(
    int64_t input_x_q16,
    uint32_t max_degree_n,
    uint32_t k_param,
    AuncientHarvardLegendreMetrics *metrics_out
);

typedef struct {
    int64_t periapsis_r0_q16;
    int64_t final_radius_q16;
    int64_t committed_radius_q16;
    uint32_t displacement_wrap_mod;
    bool shadow_isolation_sound;
    bool valve_zero_flux_sound;
    bool rollback_sound;
    bool overall_orbit_sound;
} AuncientBallisticOrbitValveMetrics;

bool auncient_ballistic_orbit_valve_prover(
    int64_t periapsis_r0_q16,
    int64_t dt_q16,
    int64_t v0_q16,
    bool simulate_trajectory_fault,
    uint32_t k_param,
    AuncientBallisticOrbitValveMetrics *metrics_out
);

typedef struct {
    uint64_t multiplicand_a;
    uint64_t multiplier_b;
    uint64_t accumulated_product;
    uint64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool commutator_sound;
    bool shadow_detent_sound;
    bool mechanical_latch_sound;
    bool overall_1946_sound;
} AuncientHarvard1946MultiplierMetrics;

bool auncient_harvard_1946_multiplier_prover(
    uint64_t multiplicand_a,
    uint64_t multiplier_b,
    bool simulate_tape_tear_fault,
    uint32_t k_param,
    AuncientHarvard1946MultiplierMetrics *metrics_out
);

typedef struct {
    int64_t base_x_q16;
    int64_t sub_interval_dx_q16;
    int64_t interpolated_val_q16;
    int64_t committed_output_q16;
    uint32_t displacement_wrap_mod;
    bool difference_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_interpolator_sound;
} AuncientHarvard1946InterpolatorMetrics;

bool auncient_harvard_1946_interpolator_prover(
    int64_t base_x_q16,
    int64_t sub_interval_dx_q16,
    bool simulate_tape_skew_fault,
    uint32_t k_param,
    AuncientHarvard1946InterpolatorMetrics *metrics_out
);

typedef struct {
    uint32_t decimal_digit_in;
    uint32_t bi_part;
    uint32_t quin_part;
    uint32_t active_relay_count;
    uint32_t committed_output;
    uint32_t displacement_wrap_mod;
    bool parity_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_biquinary_sound;
} AuncientHarvard1946BiquinaryMetrics;

bool auncient_harvard_1946_biquinary_prover(
    uint32_t decimal_digit_in,
    bool simulate_contact_chatter_fault,
    uint32_t k_param,
    AuncientHarvard1946BiquinaryMetrics *metrics_out
);

typedef struct {
    uint64_t dividend_a;
    uint64_t divisor_b;
    uint64_t quotient_q;
    uint64_t remainder_r;
    uint64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool residue_sound;
    bool remainder_bound_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_divider_sound;
} AuncientHarvard1946DividerMetrics;

bool auncient_harvard_1946_divider_prover(
    uint64_t dividend_a,
    uint64_t divisor_b,
    bool simulate_zero_div_fault,
    uint32_t k_param,
    AuncientHarvard1946DividerMetrics *metrics_out
);

typedef struct {
    uint64_t base_counter_val;
    uint64_t increment_val;
    uint64_t accumulated_sum;
    uint64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool ripple_carry_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_geneva_sound;
} AuncientHarvard1946GenevaCarryMetrics;

bool auncient_harvard_1946_geneva_carry_prover(
    uint64_t base_counter_val,
    uint64_t increment_val,
    bool simulate_gear_jam_fault,
    uint32_t k_param,
    AuncientHarvard1946GenevaCarryMetrics *metrics_out
);

typedef struct {
    int64_t v_a_in;
    int64_t v_b_in;
    int64_t v_diff_forward;
    int64_t v_diff_inverted;
    int64_t g_gate_forward;
    int64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool quadrant_inversion_sound;
    bool gating_clamp_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_zuo_hbridge_sound;
} AuncientHarvardZuoHBridgeMetrics;

bool auncient_harvard_zuo_hbridge_quadrant_prover(
    int64_t v_a_in,
    int64_t v_b_in,
    bool simulate_arm_short_fault,
    uint32_t k_param,
    AuncientHarvardZuoHBridgeMetrics *metrics_out
);

typedef struct {
    uint64_t value_leaf_idx;
    uint64_t stride_step;
    uint64_t argument_index_out;
    uint64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool phase_lock_sound;
    bool stride_bound_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_tape_sync_sound;
} AuncientHarvardZuoTapeSyncMetrics;

bool auncient_harvard_zuo_tape_sync_prover(
    uint64_t value_leaf_idx,
    uint64_t stride_step,
    bool simulate_tape_skew_fault,
    uint32_t k_param,
    AuncientHarvardZuoTapeSyncMetrics *metrics_out
);

typedef struct {
    uint64_t pin_vector_seed;
    uint64_t sum_input_channels;
    uint64_t sum_permuted_channels;
    uint64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool bijectivity_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_plugboard_sound;
} AuncientHarvardZuoPlugboardMetrics;

bool auncient_harvard_zuo_plugboard_prover(
    uint64_t pin_vector_seed,
    bool simulate_cross_talk_fault,
    uint32_t k_param,
    AuncientHarvardZuoPlugboardMetrics *metrics_out
);

typedef struct {
    int64_t z_real_q16;
    int64_t h0_q16;
    int64_t h1_q16;
    int64_t h2_q16;
    int64_t h3_q16;
    int64_t g_gate_q16;
    int64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool stability_bound_sound;
    bool gating_clamp_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_hankel_sound;
} AuncientHarvardZuoHankelMetrics;

bool auncient_harvard_zuo_hankel_prover(
    int64_t z_real_q16,
    bool simulate_branch_singularity_fault,
    uint32_t k_param,
    AuncientHarvardZuoHankelMetrics *metrics_out
);

typedef struct {
    uint64_t initial_saat_seed;
    uint32_t cycle_count;
    uint64_t final_state;
    uint64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool self_identity_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_self_identity_sound;
} AuncientHarvardZuoSelfIdentityMetrics;

bool auncient_harvard_zuo_self_identity_prover(
    uint64_t initial_saat_seed,
    uint32_t cycle_count,
    bool simulate_clutch_trip_fault,
    uint32_t k_param,
    AuncientHarvardZuoSelfIdentityMetrics *metrics_out
);

typedef struct {
    int64_t arm1_current_ma;
    int64_t arm2_current_ma;
    int64_t total_current_ma;
    int64_t g_gate_factor;
    int64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool torque_balance_sound;
    bool gating_clamp_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_torque_sound;
} AuncientHarvardZuoTorqueMetrics;

bool auncient_harvard_zuo_torque_balance_prover(
    int64_t arm1_current_ma,
    int64_t arm2_current_ma,
    bool simulate_torque_imbalance_fault,
    uint32_t k_param,
    AuncientHarvardZuoTorqueMetrics *metrics_out
);

typedef struct {
    int64_t x_arg_q16;
    int64_t k0_q16;
    int64_t k1_q16;
    int64_t k2_q16;
    int64_t k3_q16;
    int64_t g_gate_q16;
    int64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool stability_bound_sound;
    bool gating_clamp_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_bessel_sound;
} AuncientHarvardZuoBesselMetrics;

bool auncient_harvard_zuo_bessel_modified_prover(
    int64_t x_arg_q16,
    bool simulate_pole_fault,
    uint32_t k_param,
    AuncientHarvardZuoBesselMetrics *metrics_out
);

typedef struct {
    uint32_t loop_length_steps;
    uint32_t total_revolutions;
    uint64_t step_accumulator;
    int64_t g_gate_factor;
    uint64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool topological_homology_sound;
    bool gating_clamp_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_tape_loop_sound;
} AuncientHarvardZuoTapeLoopMetrics;

bool auncient_harvard_zuo_tape_loop_prover(
    uint32_t loop_length_steps,
    uint32_t total_revolutions,
    bool simulate_splice_tear_fault,
    uint32_t k_param,
    AuncientHarvardZuoTapeLoopMetrics *metrics_out
);

typedef struct {
    uint32_t decimal_digit;
    uint32_t code_word;
    uint32_t active_hamming_weight;
    int64_t g_gate_factor;
    uint64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool hamming_weight_sound;
    bool gating_clamp_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_2of5_sound;
} AuncientHarvardZuo2of5Metrics;

bool auncient_harvard_zuo_two_out_of_five_prover(
    uint32_t decimal_digit,
    bool simulate_bit_flip_fault,
    uint32_t k_param,
    AuncientHarvardZuo2of5Metrics *metrics_out
);

typedef struct {
    uint64_t source_saat_value;
    uint32_t decade_count;
    uint64_t dest_saat_value;
    int64_t g_gate_factor;
    uint64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool bus_transfer_sound;
    bool gating_clamp_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_bus_sound;
} AuncientHarvardZuoBusMetrics;

bool auncient_harvard_zuo_transfer_bus_prover(
    uint64_t source_saat_value,
    uint32_t decade_count,
    bool simulate_bus_short_fault,
    uint32_t k_param,
    AuncientHarvardZuoBusMetrics *metrics_out
);

typedef struct {
    uint32_t base_rpm_tenths;
    uint32_t cycle_revolutions;
    uint32_t final_rpm_tenths;
    int64_t g_gate_factor;
    uint64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool angular_velocity_sound;
    bool gating_clamp_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_momentum_sound;
} AuncientHarvardZuoMomentumMetrics;

bool auncient_harvard_zuo_angular_momentum_prover(
    uint32_t base_rpm_tenths,
    uint32_t cycle_revolutions,
    bool simulate_stall_fault,
    uint32_t k_param,
    AuncientHarvardZuoMomentumMetrics *metrics_out
);

typedef struct {
    uint64_t entry_coordinate;
    uint32_t cascade_depth;
    uint64_t return_coordinate;
    int64_t g_gate_factor;
    uint64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool return_coordinate_sound;
    bool gating_clamp_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_cascade_sound;
} AuncientHarvardZuoCascadeMetrics;

bool auncient_harvard_zuo_subroutine_cascade_prover(
    uint64_t entry_coordinate,
    uint32_t cascade_depth,
    bool simulate_stack_fault,
    uint32_t k_param,
    AuncientHarvardZuoCascadeMetrics *metrics_out
);

typedef struct {
    uint64_t minuend_val;
    uint64_t subtrahend_val;
    uint64_t direct_diff_val;
    uint64_t modular_diff_val;
    int64_t g_gate_factor;
    uint64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool modular_equivalence_sound;
    bool gating_clamp_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_nines_sound;
} AuncientHarvardZuoNinesMetrics;

bool auncient_harvard_zuo_nines_complement_prover(
    uint64_t minuend_val,
    uint64_t subtrahend_val,
    bool simulate_borrow_fault,
    uint32_t k_param,
    AuncientHarvardZuoNinesMetrics *metrics_out
);

typedef struct {
    uint32_t t_cam_phase;
    uint32_t p_cam_phase;
    uint32_t phase_difference;
    int64_t g_gate_factor;
    uint64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool phase_orthogonality_sound;
    bool gating_clamp_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_cam_sound;
} AuncientHarvardZuoCamMetrics;

bool auncient_harvard_zuo_dual_cam_matrix_prover(
    uint32_t t_cam_phase,
    uint32_t p_cam_phase,
    bool simulate_collision_fault,
    uint32_t k_param,
    AuncientHarvardZuoCamMetrics *metrics_out
);

typedef struct {
    uint32_t bootstrap_word_count;
    uint32_t recirculation_cycles;
    uint64_t initial_checksum;
    uint64_t recirc_checksum;
    int64_t g_gate_factor;
    uint64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool delay_recirculation_sound;
    bool gating_clamp_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_bootstrap_sound;
} AuncientHarvardZuoOrders1Metrics;

bool auncient_harvard_zuo_orders1_bootstrap_prover(
    uint32_t bootstrap_word_count,
    uint32_t recirculation_cycles,
    bool simulate_dispersion_fault,
    uint32_t k_param,
    AuncientHarvardZuoOrders1Metrics *metrics_out
);

typedef struct {
    uint32_t low_short_word;
    uint32_t high_short_word;
    uint64_t coupled_long_word;
    uint32_t reconstructed_low;
    uint32_t reconstructed_high;
    int64_t g_gate_factor;
    uint64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool reversible_coupling_sound;
    bool gating_clamp_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_coupling_sound;
} AuncientHarvardZuoCouplingMetrics;

bool auncient_harvard_zuo_word_coupling_prover(
    uint32_t low_short_word,
    uint32_t high_short_word,
    bool simulate_bleed_fault,
    uint32_t k_param,
    AuncientHarvardZuoCouplingMetrics *metrics_out
);

typedef struct {
    uint32_t starting_wiper_step;
    uint32_t impulse_count;
    uint32_t final_wiper_position;
    int64_t g_gate_factor;
    uint64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool rotary_stepping_sound;
    bool gating_clamp_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_uniselector_sound;
} AuncientHarvardZuoUniselectorMetrics;

bool auncient_harvard_zuo_uniselector_sync_prover(
    uint32_t starting_wiper_step,
    uint32_t impulse_count,
    bool simulate_bounce_fault,
    uint32_t k_param,
    AuncientHarvardZuoUniselectorMetrics *metrics_out
);

typedef struct {
    uint32_t caller_pc;
    uint32_t subroutine_entry_pc;
    uint32_t wheeler_link_instruction;
    uint32_t expected_return_pc;
    uint32_t resolved_return_pc;
    int64_t g_gate_factor;
    uint64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool return_link_sound;
    bool gating_clamp_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_wheeler_sound;
} AuncientHarvardZuoWheelerMetrics;

bool auncient_harvard_zuo_wheeler_jump_prover(
    uint32_t caller_pc,
    uint32_t subroutine_entry_pc,
    bool simulate_escape_fault,
    uint32_t k_param,
    AuncientHarvardZuoWheelerMetrics *metrics_out
);

typedef struct {
    uint32_t input_punch_mask;
    uint8_t pin_p0;
    uint8_t pin_p1;
    uint8_t pin_p2;
    uint8_t pin_p3;
    uint8_t pin_p4;
    uint32_t reconstructed_mask;
    int64_t g_gate_factor;
    uint64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool reversible_sensing_sound;
    bool gating_clamp_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_sensing_pin_sound;
} AuncientHarvardZuoSensingPinMetrics;

bool auncient_harvard_zuo_sensing_pin_matrix_prover(
    uint32_t input_punch_mask,
    bool simulate_pin_fault,
    uint32_t k_param,
    AuncientHarvardZuoSensingPinMetrics *metrics_out
);

typedef struct {
    uint32_t base_address;
    uint32_t theta_offset;
    uint32_t effective_address;
    uint32_t target_pc;
    uint32_t cics_vector;
    int64_t g_gate_factor;
    uint64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool address_relocation_sound;
    bool cics_jump_sound;
    bool gating_clamp_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_cics_jump_sound;
} AuncientHarvardZuoCicsJumpMetrics;

bool auncient_harvard_zuo_cics_wheeler_jump_prover(
    uint32_t base_address,
    uint32_t theta_offset,
    bool simulate_cics_exception,
    uint32_t k_param,
    AuncientHarvardZuoCicsJumpMetrics *metrics_out
);

typedef struct {
    int64_t dot_tl_q16;
    uint32_t clumping_seed;
    int64_t sin_theta_sq_q16;
    int64_t spec1_lobe_q16;
    int64_t spec2_lobe_q16;
    int64_t total_spec_q16;
    int64_t g_gate_factor;
    uint64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool energy_conservation_sound;
    bool gating_clamp_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_marschner_sound;
} AuncientMarschnerFurMetrics;

bool auncient_marschner_fur_scattering_prover(
    int64_t dot_tl_q16,
    uint32_t clumping_seed,
    bool simulate_light_singularity,
    uint32_t k_param,
    AuncientMarschnerFurMetrics *metrics_out
);

typedef struct {
    uint32_t shutter_angle_deg;
    uint32_t claw_pull_down_phase;
    uint32_t vdc_clip_x;
    uint32_t vce_palette_idx;
    int64_t exposure_fraction_q16;
    uint32_t vce_rgb565;
    int64_t g_gate_factor;
    uint64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool rotary_shutter_sound;
    bool claw_advance_sound;
    bool vdc_raster_clip_sound;
    bool vce_palette_sound;
    bool gating_clamp_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_super8_hudson_sound;
} AuncientSuper8HudsonMetrics;

bool auncient_super8_hudson_renderman_prover(
    uint32_t shutter_angle_deg,
    uint32_t claw_pull_down_phase,
    uint32_t vdc_clip_x,
    uint32_t vce_palette_idx,
    bool simulate_film_jam_fault,
    uint32_t k_param,
    AuncientSuper8HudsonMetrics *metrics_out
);

typedef struct {
    int64_t density_flower_q16;
    int64_t normal_mag_q16;
    int64_t sss_through_q16;
    int64_t sss_internal_q16;
    int64_t fresnel_rim_q16;
    int64_t total_radiance_q16;
    int64_t g_gate_factor;
    uint64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool energy_conservation_sound;
    bool gating_clamp_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_sss_sound;
} AuncientDualPassSSSMetrics;

bool auncient_dual_pass_sss_venation_prover(
    int64_t density_flower_q16,
    int64_t normal_mag_q16,
    bool simulate_over_exposure_fault,
    uint32_t k_param,
    AuncientDualPassSSSMetrics *metrics_out
);

typedef struct {
    uint32_t camera_fov_deg;
    uint32_t descriptor_set_idx;
    uint32_t mesh_depth_z;
    int64_t focal_length_q16;
    int64_t projected_z_q16;
    int64_t g_gate_factor;
    uint64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool focal_frustum_sound;
    bool depth_monotonic_sound;
    bool descriptor_layout_sound;
    bool gating_clamp_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_vulkan_camera_sound;
} AuncientVulkanCameraMetrics;

bool auncient_vulkan_vision_camera_prover(
    uint32_t camera_fov_deg,
    uint32_t descriptor_set_idx,
    uint32_t mesh_depth_z,
    bool simulate_frustum_fault,
    uint32_t k_param,
    AuncientVulkanCameraMetrics *metrics_out
);

typedef struct {
    uint32_t instrument_track_idx;
    uint32_t base_freq_hz;
    int64_t gain_q16;
    int64_t harmonic_q16;
    int64_t rms_norm_q16;
    int64_t g_gate_factor;
    uint64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool harmonic_bound_sound;
    bool energy_conservation_sound;
    bool gating_clamp_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_bionika_synth_sound;
} AuncientBionikaSynthMetrics;

bool auncient_bionika_synth_overdrive_prover(
    uint32_t instrument_track_idx,
    uint32_t base_freq_hz,
    int64_t gain_q16,
    bool simulate_overdrive_fault,
    uint32_t k_param,
    AuncientBionikaSynthMetrics *metrics_out
);

typedef struct {
    uint32_t grid_size_n;
    int64_t rest_len_q16;
    int64_t charge_level_q16;
    int64_t verlet_disp_q16;
    int64_t spring_force_q16;
    int64_t g_gate_factor;
    uint64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool verlet_bound_sound;
    bool spring_relaxation_sound;
    bool gating_clamp_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_usdshade_sound;
} AuncientUsdShadeVerletMetrics;

bool auncient_usdshade_fet_verlet_prover(
    uint32_t grid_size_n,
    int64_t rest_len_q16,
    int64_t charge_level_q16,
    bool simulate_lattice_fault,
    uint32_t k_param,
    AuncientUsdShadeVerletMetrics *metrics_out
);

typedef struct {
    uint32_t slot_selector;
    uint32_t offset_ms;
    uint32_t up_ramp_ms;
    uint32_t down_ramp_ms;
    int64_t timing_envelope_q16;
    int64_t g_gate_factor;
    uint64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool slot_boundary_sound;
    bool ramp_threshold_sound;
    bool gating_clamp_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_nato_slot_sound;
} AuncientCadeImfNatoSlotMetrics;

bool auncient_cade_imf_nato_slot_prover(
    uint32_t slot_selector,
    uint32_t offset_ms,
    uint32_t up_ramp_ms,
    uint32_t down_ramp_ms,
    bool simulate_timing_fault,
    uint32_t k_param,
    AuncientCadeImfNatoSlotMetrics *metrics_out
);

typedef struct {
    uint32_t dna_seed;
    uint32_t area_code;
    uint64_t simulated_endowment_saat;
    int64_t g_gate_factor;
    uint64_t committed_output;
    uint32_t displacement_wrap_mod;
    bool area_code_bound_sound;
    bool endowment_value_sound;
    bool gating_clamp_sound;
    bool shadow_isolation_sound;
    bool rollback_sound;
    bool overall_teddy_bear_sound;
} AuncientTeddyBearSsaMetrics;

bool auncient_teddy_bear_ssa_endowment_prover(
    uint32_t dna_seed,
    uint64_t simulated_endowment_saat,
    bool simulate_registry_fault,
    uint32_t k_param,
    AuncientTeddyBearSsaMetrics *metrics_out
);

#endif // AUNCIENT_HARVARD_COMPUTATION_LAB_H
