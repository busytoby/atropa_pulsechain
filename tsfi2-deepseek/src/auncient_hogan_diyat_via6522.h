#ifndef AUNCIENT_HOGAN_DIYAT_VIA6522_H
#define AUNCIENT_HOGAN_DIYAT_VIA6522_H

#include "auncient_teddy_bear_heart_usda.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define VIA6522_TPA_BASE 0x0100
#define DIYAT_CAPSTAN_SEEK_FEE_SAAT   25ULL
#define DIYAT_VIA_TIMER_INT_FEE_SAAT  10ULL
#define DIYAT_PORT_IO_FEE_SAAT         5ULL

typedef struct {
    uint8_t ora;  /* Output Register A */
    uint8_t orb;  /* Output Register B */
    uint8_t ddra; /* Data Direction Register A */
    uint8_t ddrb; /* Data Direction Register B */
    uint16_t t1c; /* Timer 1 Counter */
    uint16_t t2c; /* Timer 2 Counter */
    uint8_t sr;   /* Shift Register */
    uint8_t acr;  /* Auxiliary Control Register */
    uint8_t pcr;  /* Peripheral Control Register */
    uint8_t ifr;  /* Interrupt Flag Register */
    uint8_t ier;  /* Interrupt Enable Register */
} Via6522State;

typedef struct {
    float angular_velocity_rad_s;
    float tape_position_meters;
    uint32_t total_seeks;
    bool motor_engaged;
} CapstanShaftState;

typedef struct {
    TeddyBearHeartUsdaAsset *teddy_bear;
    Via6522State via;
    CapstanShaftState capstan;
    uint64_t total_diyat_fees_charged_saat;
    bool acid_atomicity_verified;
    bool capstan_inertia_diyat_verified;
    bool via_timer_diyat_verified;
    bool insipient_balance_revert_verified;
    bool rule18_closure_verified;
    uint32_t rule18_parity_checksum;
} HoganDiyatVia6522Engine;

void auncient_hogan_diyat_init(HoganDiyatVia6522Engine *engine, TeddyBearHeartUsdaAsset *teddy_bear);
bool auncient_hogan_diyat_charge_fee(HoganDiyatVia6522Engine *engine, uint64_t fee_saat);
bool auncient_via6522_execute_port_io(HoganDiyatVia6522Engine *engine, uint8_t port_a_val, uint8_t port_b_val);
bool auncient_capstan_seek(HoganDiyatVia6522Engine *engine, float distance_meters);
bool auncient_hogan_diyat_verify_theorems_56_60(HoganDiyatVia6522Engine *engine);
uint32_t auncient_hogan_diyat_compute_rule18(const HoganDiyatVia6522Engine *engine);

#endif /* AUNCIENT_HOGAN_DIYAT_VIA6522_H */
