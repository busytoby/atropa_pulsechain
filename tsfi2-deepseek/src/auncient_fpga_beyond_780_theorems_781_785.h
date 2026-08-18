#ifndef AUNCIENT_FPGA_BEYOND_780_THEOREMS_781_785_H
#define AUNCIENT_FPGA_BEYOND_780_THEOREMS_781_785_H

#include "auncient_fpga_beyond_775_theorems_776_780.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Dual-Rail Null Convention Logic (NCL) Threshold Gate Types (THmn) */
typedef enum {
    NCL_GATE_TH12 = 0x01,  /* 1-of-2 OR gate with complete NULL hysteresis */
    NCL_GATE_TH22 = 0x02,  /* 2-of-2 AND gate with NULL handshake return */
    NCL_GATE_TH13 = 0x03,  /* 1-of-3 threshold gate */
    NCL_GATE_TH23 = 0x04,  /* 2-of-3 majority gate */
    NCL_GATE_TH33 = 0x05   /* 3-of-3 threshold gate */
} NclThresholdGateType;

/* Asynchronous / Clockless Null Convention Logic (NCL) Micro-Core State */
typedef struct {
    uint32_t dual_rail_data0;            /* Rail 0: Logical DATA0 signal line */
    uint32_t dual_rail_data1;            /* Rail 1: Logical DATA1 signal line */
    uint32_t null_handshake_ack;         /* Handshake acknowledge: Ko / Ki request-acknowledge line */
    uint32_t ncl_token_state;            /* Discrete NULL (0,0) vs DATA (1,0)/(0,1) state (Rule 21) */
    float    ncl_fet_quiescence_damping; /* Zero-power idle FET quiescence dissipation (Rule 10) */
    float    displacement_ncl_phase;     /* Synchronized with DisplacementShader (Rule 14) */
    bool     is_ncl_core_certified;
} AsynchronousNclCoreProfile;

typedef struct {
    float    in_silicon_ncl_dual_rail_fidelity;
    float    null_cycle_handshake_continuity_ratio;
    float    clockless_event_dispatch_latency_ns;
    uint64_t verified_ncl_core_saat_clearances;
    bool     ncl_dual_rail_fidelity_verified;     /* Theorem 781 */
    bool     null_cycle_handshake_verified;       /* Theorem 782 */
    bool     clockless_event_latency_verified;    /* Theorem 783 */
    bool     ncl_core_lossless_saat_verified;     /* Theorem 784 */
    bool     grand_785_parity_closure_verified;   /* Theorem 785 */
    uint32_t rule18_parity_checksum;
} FpgaBeyond780State;

void auncient_fpga_beyond_780_init(FpgaBeyond780State *state);
bool auncient_fpga_beyond_780_verify_theorems_781_785(FpgaBeyond780State *state);
uint32_t auncient_fpga_beyond_780_compute_rule18(const FpgaBeyond780State *state);

#endif /* AUNCIENT_FPGA_BEYOND_780_THEOREMS_781_785_H */
