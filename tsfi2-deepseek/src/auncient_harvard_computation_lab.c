#include "../inc/auncient_harvard_computation_lab.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void auncient_harvard_wheel_init(AuncientCounterWheel *w) {
    if (w) { memset(w->digits, 0, sizeof(w->digits)); w->sign = false; w->detent_zero_locked = true; w->carry_cascade_count = 0; }
}

bool auncient_harvard_wheel_set_uint64(AuncientCounterWheel *w, uint64_t v) {
    if (!w) return false;
    auncient_harvard_wheel_init(w);
    if (v == 0) { w->detent_zero_locked = true; return true; }
    w->detent_zero_locked = false;
    for (size_t i = 0; i < HARVARD_WHEEL_DIGITS && v > 0; ++i) { w->digits[i] = (uint8_t)(v % 10); v /= 10; }
    return true;
}

uint64_t auncient_harvard_wheel_get_uint64(const AuncientCounterWheel *w) {
    if (!w) return 0;
    uint64_t v = 0, m = 1;
    for (size_t i = 0; i < HARVARD_WHEEL_DIGITS && i < 19; ++i) { v += ((uint64_t)w->digits[i]) * m; m *= 10; }
    return v;
}

bool auncient_harvard_wheel_add_step(AuncientCounterWheel *d, const AuncientCounterWheel *s) {
    if (!d || !s) return false;
    uint8_t c = 0; d->carry_cascade_count = 0;
    for (size_t i = 0; i < HARVARD_WHEEL_DIGITS; ++i) {
        uint16_t sum = (uint16_t)d->digits[i] + (uint16_t)s->digits[i] + c;
        d->digits[i] = (uint8_t)(sum % 10); c = (uint8_t)(sum / 10);
        if (c > 0) d->carry_cascade_count++;
    }
    d->detent_zero_locked = (auncient_harvard_wheel_get_uint64(d) == 0);
    return true;
}

void auncient_harvard_tape_init(AuncientSequenceTape *t) {
    if (t) { memset(t->tape, 0, sizeof(t->tape)); t->tape_length = 0; t->program_counter = 0; t->shadow_pc = 0; t->latch_trip_active = false; }
}

bool auncient_harvard_tape_load(AuncientSequenceTape *t, const AuncientTapeInstruction *insts, size_t count) {
    if (!t || !insts || count > 256) return false;
    auncient_harvard_tape_init(t);
    memcpy(t->tape, insts, count * sizeof(AuncientTapeInstruction));
    t->tape_length = count;
    return true;
}

bool auncient_harvard_tape_step(AuncientSequenceTape *t, uint64_t *r, size_t rc) {
    if (!t || !r || t->program_counter >= t->tape_length || t->latch_trip_active) return false;
    AuncientTapeInstruction inst = t->tape[t->program_counter];
    if (inst.out_register >= rc || inst.in_register >= rc) return false;
    r[inst.in_register] = r[inst.out_register];
    t->program_counter++;
    return true;
}

void auncient_harvard_tape_trigger_fault(AuncientSequenceTape *t, uint64_t *r, const uint64_t *sr, size_t rc) {
    if (!t || !r || !sr) return;
    t->latch_trip_active = true;
    t->program_counter = t->shadow_pc;
    for (size_t i = 0; i < rc; ++i) r[i] = sr[i];
}

int64_t auncient_harvard_bessel_j0_fixed(int64_t x_q16) {
    int64_t one = 65536LL, x2 = (x_q16 * x_q16) >> 16, term1 = x2 >> 2, term2 = (term1 * term1) >> (16 + 2);
    return one - term1 + term2;
}

int64_t auncient_harvard_bessel_j1_fixed(int64_t x_q16) {
    int64_t half_x = x_q16 >> 1, x2 = (x_q16 * x_q16) >> 16, term1 = (half_x * x2) >> (16 + 3);
    return half_x - term1;
}

bool auncient_harvard_bessel_recurrence_verify(int64_t x_q16, int64_t j0, int64_t j1) {
    int64_t dx = 256LL, j0_plus = auncient_harvard_bessel_j0_fixed(x_q16 + dx);
    int64_t num_deriv = ((j0_plus - j0) << 16) / dx, neg_j1 = -j1, diff = (num_deriv > neg_j1) ? (num_deriv - neg_j1) : (neg_j1 - num_deriv);
    return (diff < 5000);
}

void auncient_harvard_commutator_init(AuncientCommutatorValve *v, uint64_t c) {
    if (v) { v->current_phase = 0; v->stroke_charge_mu = c; v->staged_potential_eta = 0; v->residual_flux = 0; v->valve_closed_at_t9 = false; }
}

bool auncient_harvard_commutator_step_phase(AuncientCommutatorValve *v) {
    if (!v) return false;
    v->current_phase = (v->current_phase + 1) % HARVARD_COMMUTATOR_PHASES;
    return true;
}
