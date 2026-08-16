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

#endif // AUNCIENT_HARVARD_COMPUTATION_LAB_H
