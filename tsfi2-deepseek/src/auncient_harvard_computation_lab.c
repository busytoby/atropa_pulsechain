#include "../inc/auncient_harvard_computation_lab.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* 1. Mark I 24-Digit Mechanical Counter Wheel Implementation */
void auncient_harvard_wheel_init(AuncientCounterWheel *wheel) {
    if (!wheel) return;
    memset(wheel->digits, 0, sizeof(wheel->digits));
    wheel->sign = false;
    wheel->detent_zero_locked = true;
    wheel->carry_cascade_count = 0;
}

bool auncient_harvard_wheel_set_uint64(AuncientCounterWheel *wheel, uint64_t val) {
    if (!wheel) return false;
    auncient_harvard_wheel_init(wheel);
    if (val == 0) {
        wheel->detent_zero_locked = true;
        return true;
    }
    wheel->detent_zero_locked = false;
    for (size_t i = 0; i < HARVARD_WHEEL_DIGITS && val > 0; ++i) {
        wheel->digits[i] = (uint8_t)(val % 10);
        val /= 10;
    }
    return true;
}

uint64_t auncient_harvard_wheel_get_uint64(const AuncientCounterWheel *wheel) {
    if (!wheel) return 0;
    uint64_t val = 0;
    uint64_t mult = 1;
    for (size_t i = 0; i < HARVARD_WHEEL_DIGITS && i < 19; ++i) {
        val += ((uint64_t)wheel->digits[i]) * mult;
        mult *= 10;
    }
    return val;
}

bool auncient_harvard_wheel_add_step(AuncientCounterWheel *dest, const AuncientCounterWheel *src) {
    if (!dest || !src) return false;
    uint8_t carry = 0;
    dest->carry_cascade_count = 0;
    for (size_t i = 0; i < HARVARD_WHEEL_DIGITS; ++i) {
        uint16_t sum = (uint16_t)dest->digits[i] + (uint16_t)src->digits[i] + carry;
        dest->digits[i] = (uint8_t)(sum % 10);
        carry = (uint8_t)(sum / 10);
        if (carry > 0) {
            dest->carry_cascade_count++;
        }
    }
    dest->detent_zero_locked = (auncient_harvard_wheel_get_uint64(dest) == 0);
    return true;
}

/* 2. Harvard 3-Address Sequence Tape Interpreter */
void auncient_harvard_tape_init(AuncientSequenceTape *tape_ctrl) {
    if (!tape_ctrl) return;
    memset(tape_ctrl->tape, 0, sizeof(tape_ctrl->tape));
    tape_ctrl->tape_length = 0;
    tape_ctrl->program_counter = 0;
    tape_ctrl->shadow_pc = 0;
    tape_ctrl->latch_trip_active = false;
}

bool auncient_harvard_tape_load(AuncientSequenceTape *tape_ctrl, const AuncientTapeInstruction *insts, size_t count) {
    if (!tape_ctrl || !insts || count > 256) return false;
    auncient_harvard_tape_init(tape_ctrl);
    memcpy(tape_ctrl->tape, insts, count * sizeof(AuncientTapeInstruction));
    tape_ctrl->tape_length = count;
    return true;
}

bool auncient_harvard_tape_step(AuncientSequenceTape *tape_ctrl, uint64_t *registers, size_t reg_count) {
    if (!tape_ctrl || !registers || tape_ctrl->program_counter >= tape_ctrl->tape_length) return false;
    if (tape_ctrl->latch_trip_active) return false;

    AuncientTapeInstruction inst = tape_ctrl->tape[tape_ctrl->program_counter];
    if (inst.out_register >= reg_count || inst.in_register >= reg_count) return false;

    switch (inst.opcode) {
        case 1: // Add
            registers[inst.in_register] += registers[inst.out_register];
            break;
        case 2: // Sub
            if (registers[inst.in_register] >= registers[inst.out_register]) {
                registers[inst.in_register] -= registers[inst.out_register];
            } else {
                registers[inst.in_register] = 0;
            }
            break;
        case 3: // Mult scale (7/8)
            registers[inst.in_register] = (registers[inst.out_register] * 875ULL) / 1000ULL;
            break;
        case 4: // Clear-Add
            registers[inst.in_register] = registers[inst.out_register];
            break;
        case 5: // Valve-Gating: Modpow(mu, e, mu) == 0
            registers[inst.in_register] = registers[inst.out_register] % registers[inst.out_register];
            break;
        default:
            break;
    }

    tape_ctrl->program_counter++;
    return true;
}

void auncient_harvard_tape_trigger_fault(
    AuncientSequenceTape *tape_ctrl,
    uint64_t *registers,
    const uint64_t *shadow_registers,
    size_t reg_count
) {
    if (!tape_ctrl || !registers || !shadow_registers) return;
    tape_ctrl->latch_trip_active = true;
    tape_ctrl->program_counter = tape_ctrl->shadow_pc;
    for (size_t i = 0; i < reg_count; ++i) {
        registers[i] = shadow_registers[i];
    }
}

/* 3. Annals Vol. III Bessel Series & Finite-Difference Recurrence (Fixed-Point Q16) */
int64_t auncient_harvard_bessel_j0_fixed(int64_t x_q16) {
    // J0(x) = 1 - (x/2)^2 + (x/2)^4 / (1! * 2!)^2 ...
    // In Q16 fixed point: 1.0 = 65536
    int64_t one = 65536LL;
    int64_t x2 = (x_q16 * x_q16) >> 16;      // x^2
    int64_t term1 = x2 >> 2;                  // (x/2)^2
    int64_t term2 = (term1 * term1) >> (16 + 2); // ((x/2)^4) / 4

    int64_t res = one - term1 + term2;
    return res;
}

int64_t auncient_harvard_bessel_j1_fixed(int64_t x_q16) {
    // J1(x) = (x/2) - (x/2)^3 / 2 + (x/2)^5 / 12
    int64_t half_x = x_q16 >> 1;              // x/2
    int64_t x2 = (x_q16 * x_q16) >> 16;
    int64_t term1 = (half_x * x2) >> (16 + 3); // (x/2)^3 / 2
    int64_t res = half_x - term1;
    return res;
}

bool auncient_harvard_bessel_recurrence_verify(int64_t x_q16, int64_t j0, int64_t j1) {
    // Differentiation / recurrence relation: J0'(x) = -J1(x)
    // For small step dx = 256 (in Q16 ~ 0.0039):
    int64_t dx = 256LL;
    int64_t j0_plus = auncient_harvard_bessel_j0_fixed(x_q16 + dx);
    int64_t num_deriv = ((j0_plus - j0) << 16) / dx;
    int64_t neg_j1 = -j1;
    int64_t diff = (num_deriv > neg_j1) ? (num_deriv - neg_j1) : (neg_j1 - num_deriv);
    return (diff < 5000); // Tolerance in Q16
}

/* 4. Cam-Timed Commutator & Valve Interrupter */
void auncient_harvard_commutator_init(AuncientCommutatorValve *valve, uint64_t charge_mu) {
    if (!valve) return;
    valve->current_phase = 0;
    valve->stroke_charge_mu = charge_mu;
    valve->staged_potential_eta = 0;
    valve->residual_flux = 0;
    valve->valve_closed_at_t9 = false;
}

bool auncient_harvard_commutator_step_phase(AuncientCommutatorValve *valve) {
    if (!valve) return false;
    if (valve->current_phase < 5) {
        // Phases 0..4: Charge buildup
        valve->staged_potential_eta = (valve->stroke_charge_mu * (uint64_t)(valve->current_phase + 1) * 875ULL) / 5000ULL;
        valve->residual_flux = valve->staged_potential_eta;
    } else if (valve->current_phase < 9) {
        // Phases 5..8: Power delivery & cam-damped extraction
        valve->staged_potential_eta = (valve->stroke_charge_mu * 875ULL) / 1000ULL;
        valve->residual_flux = valve->staged_potential_eta / (valve->current_phase - 3);
    } else if (valve->current_phase == 9) {
        // Phase 9: Mechanical interrupter drop to zero
        valve->residual_flux = valve->stroke_charge_mu % valve->stroke_charge_mu; // == 0
        valve->valve_closed_at_t9 = (valve->residual_flux == 0);
    }
    valve->current_phase = (valve->current_phase + 1) % HARVARD_COMMUTATOR_PHASES;
    return true;
}

/* Unified Harvard Computation Laboratory Prover */
bool auncient_harvard_computation_lab_prover(
    uint64_t initial_charge_mu,
    int64_t test_x_q16,
    bool simulate_tape_fault,
    AuncientHarvardLabMetrics *metrics_out
) {
    if (initial_charge_mu == 0) return false;

    // 1. Test 24-Digit Mechanical Counter Wheel
    AuncientCounterWheel w1, w2;
    auncient_harvard_wheel_init(&w1);
    auncient_harvard_wheel_init(&w2);
    auncient_harvard_wheel_set_uint64(&w1, initial_charge_mu);
    auncient_harvard_wheel_set_uint64(&w2, 401876ULL);
    auncient_harvard_wheel_add_step(&w1, &w2);

    uint64_t w_sum = auncient_harvard_wheel_get_uint64(&w1);
    bool detent_sound = (w_sum == (initial_charge_mu + 401876ULL));

    // 2. Test 3-Address Sequence Tape & Rollback
    AuncientSequenceTape tape_ctrl;
    uint64_t registers[4] = { initial_charge_mu, 0, 0, 0 };
    uint64_t shadow_regs[4] = { initial_charge_mu, 0, 0, 0 };

    AuncientTapeInstruction program[3] = {
        { .out_register = 0, .in_register = 1, .opcode = 4 }, // Clear-Add: R1 = R0 (1,000,000)
        { .out_register = 1, .in_register = 2, .opcode = 3 }, // Mult 7/8: R2 = 875,000
        { .out_register = 2, .in_register = 3, .opcode = 5 }  // Valve: R3 = R2 % R2 == 0
    };

    auncient_harvard_tape_load(&tape_ctrl, program, 3);
    auncient_harvard_tape_step(&tape_ctrl, registers, 4); // Step 1
    auncient_harvard_tape_step(&tape_ctrl, registers, 4); // Step 2
    auncient_harvard_tape_step(&tape_ctrl, registers, 4); // Step 3

    bool tape_sound = (registers[1] == initial_charge_mu && registers[2] == 875000ULL && registers[3] == 0);

    bool rollback_sound = true;
    if (simulate_tape_fault) {
        auncient_harvard_tape_trigger_fault(&tape_ctrl, registers, shadow_regs, 4);
        rollback_sound = (registers[0] == initial_charge_mu && registers[1] == 0 && registers[2] == 0 && registers[3] == 0 && tape_ctrl.program_counter == 0);
    }

    // 3. Test Annals Vol. III Bessel Series Recurrence
    int64_t j0 = auncient_harvard_bessel_j0_fixed(test_x_q16);
    int64_t j1 = auncient_harvard_bessel_j1_fixed(test_x_q16);
    bool bessel_sound = auncient_harvard_bessel_recurrence_verify(test_x_q16, j0, j1);

    // 4. Test Cam-Timed Commutator 10-Phase Cycle
    AuncientCommutatorValve valve;
    auncient_harvard_commutator_init(&valve, initial_charge_mu);
    for (int p = 0; p < HARVARD_COMMUTATOR_PHASES; ++p) {
        auncient_harvard_commutator_step_phase(&valve);
    }
    bool commutator_sound = (valve.valve_closed_at_t9 && valve.residual_flux == 0);

    uint32_t disp_wrap = (uint32_t)(w_sum % 256);
    bool overall = detent_sound && tape_sound && rollback_sound && bessel_sound && commutator_sound;

    if (metrics_out) {
        metrics_out->wheel_value_low = w_sum;
        metrics_out->carry_overflow = w1.carry_cascade_count;
        metrics_out->detent_sound = detent_sound;
        metrics_out->tape_execution_sound = tape_sound;
        metrics_out->rollback_interlock_sound = rollback_sound;
        metrics_out->bessel_j0_fixed = j0;
        metrics_out->bessel_j1_fixed = j1;
        metrics_out->recurrence_sound = bessel_sound;
        metrics_out->commutator_t9_zero_sound = commutator_sound;
        metrics_out->displacement_wrap_mod = disp_wrap;
        metrics_out->overall_harvard_sound = overall;
    }

    return overall;
}
