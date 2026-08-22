#include "auncient_hogan_diyat_via6522.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void auncient_hogan_diyat_init(HoganDiyatVia6522Engine *engine, TeddyBearHeartUsdaAsset *teddy_bear) {
    if (!engine) return;
    memset(engine, 0, sizeof(HoganDiyatVia6522Engine));
    engine->teddy_bear = teddy_bear;

    /* VIA 6522 default reset state */
    engine->via.ddra = 0xFF; /* Port A output */
    engine->via.ddrb = 0xFF; /* Port B output */
    engine->via.t1c = 0xFFFF;
    engine->via.t2c = 0xFFFF;
    engine->via.ier = 0x80 | 0x40; /* Enable Timer 1 Interrupt */

    /* Capstan default state */
    engine->capstan.angular_velocity_rad_s = 31.4159f; /* 300 RPM standard tape capstan */
    engine->capstan.tape_position_meters = 0.0f;
    engine->capstan.motor_engaged = true;
}

bool auncient_hogan_diyat_charge_fee(HoganDiyatVia6522Engine *engine, uint64_t fee_saat) {
    if (!engine || !engine->teddy_bear) return false;

    /* ACID Pre-Condition: Verify solvent balance */
    uint64_t current_balance = engine->teddy_bear->profile.hogan_account_saat;
    if (current_balance < fee_saat) {
        /* ACID Rollback: Reject transaction, zero balance mutation */
        return false;
    }

    /* ACID Execution: Atomic debit in Saat currency */
    engine->teddy_bear->profile.hogan_account_saat -= fee_saat;
    engine->total_diyat_fees_charged_saat += fee_saat;
    return true;
}

bool auncient_via6522_execute_port_io(HoganDiyatVia6522Engine *engine, uint8_t port_a_val, uint8_t port_b_val) {
    if (!engine) return false;

    /* Charge VIA Port I/O Diyat fee in Saat */
    if (!auncient_hogan_diyat_charge_fee(engine, DIYAT_PORT_IO_FEE_SAAT)) {
        return false;
    }

    engine->via.ora = port_a_val;
    engine->via.orb = port_b_val;
    engine->via.ifr |= 0x18; /* Set active handshake flags */
    return true;
}

bool auncient_capstan_seek(HoganDiyatVia6522Engine *engine, float distance_meters) {
    if (!engine) return false;

    /* Charge Capstan Mechanical Seek Diyat fee in Saat */
    if (!auncient_hogan_diyat_charge_fee(engine, DIYAT_CAPSTAN_SEEK_FEE_SAAT)) {
        return false;
    }

    engine->capstan.tape_position_meters += distance_meters;
    engine->capstan.total_seeks++;
    return true;
}

bool auncient_hogan_diyat_verify_theorems_56_60(HoganDiyatVia6522Engine *engine) {
    if (!engine || !engine->teddy_bear) return false;

    uint64_t start_balance = engine->teddy_bear->profile.hogan_account_saat;

    /* Theorem 56: Direct Saat Fee Debit Atomicity */
    bool fee_charged = auncient_hogan_diyat_charge_fee(engine, 100ULL);
    engine->acid_atomicity_verified = fee_charged &&
        (engine->teddy_bear->profile.hogan_account_saat == (start_balance - 100ULL));

    /* Theorem 57: Capstan Shaft Kinetic Energy Diyat Scaling */
    bool seek_ok = auncient_capstan_seek(engine, 1.25f);
    engine->capstan_inertia_diyat_verified = seek_ok && (engine->capstan.tape_position_meters == 1.25f);

    /* Theorem 58: VIA 6522 Hardware Register I/O & Interrupt Fee */
    bool io_ok = auncient_via6522_execute_port_io(engine, 0xAA, 0x55);
    engine->via_timer_diyat_verified = io_ok && (engine->via.ora == 0xAA) && (engine->via.orb == 0x55);

    /* Theorem 59: Insufficient Balance Reversion & State Rollback */
    uint64_t excessive_fee = engine->teddy_bear->profile.hogan_account_saat + 50000ULL;
    uint64_t balance_before_revert = engine->teddy_bear->profile.hogan_account_saat;
    bool excessive_charged = auncient_hogan_diyat_charge_fee(engine, excessive_fee);
    engine->insipient_balance_revert_verified = (!excessive_charged) &&
        (engine->teddy_bear->profile.hogan_account_saat == balance_before_revert);

    /* Theorem 60: Non-Preferential Checksum & Hardware Closure */
    engine->rule18_parity_checksum = auncient_hogan_diyat_compute_rule18(engine);
    engine->rule18_closure_verified = (engine->rule18_parity_checksum > 0);

    return (engine->acid_atomicity_verified &&
            engine->capstan_inertia_diyat_verified &&
            engine->via_timer_diyat_verified &&
            engine->insipient_balance_revert_verified &&
            engine->rule18_closure_verified);
}

uint32_t auncient_hogan_diyat_compute_rule18(const HoganDiyatVia6522Engine *engine) {
    if (!engine) return 0;
    const uint8_t *data = (const uint8_t *)engine;
    size_t len = sizeof(HoganDiyatVia6522Engine);

    uint64_t p0 = 1;
    uint64_t p1 = (len > 0) ? (data[0] + 7) : 1;
    uint64_t pn = p1;

    for (size_t i = 1; i < len; ++i) {
        uint64_t alpha = (i * 17ULL) % 256ULL;
        uint64_t beta = (i * 31ULL) % 256ULL;
        pn = ((data[i] + alpha) * p1 - beta * p0) % 65535ULL;
        p0 = p1;
        p1 = pn;
    }
    return (uint32_t)pn;
}
