#ifndef AUNCIENT_FPGA_BIST_ICAP_THEOREMS_451_455_H
#define AUNCIENT_FPGA_BIST_ICAP_THEOREMS_451_455_H

#include "auncient_fpga_self_cert_theorems_446_450.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    float    bist_lut_coverage_ratio;
    float    icap_frame_scrub_fidelity;
    float    internal_ila_trace_bandwidth_gbps;
    uint64_t verified_bist_saat_clearances;
    bool     bist_lut_coverage_verified;          /* Theorem 451 */
    bool     icap_frame_scrub_verified;           /* Theorem 452 */
    bool     internal_ila_trace_verified;         /* Theorem 453 */
    bool     bist_lossless_saat_verified;         /* Theorem 454 */
    bool     bist_icap_grand_parity_verified;     /* Theorem 455 */
    uint32_t rule18_parity_checksum;
} FpgaBistIcapState;

void auncient_fpga_bist_icap_init(FpgaBistIcapState *state);
bool auncient_fpga_bist_icap_verify_theorems_451_455(FpgaBistIcapState *state);
uint32_t auncient_fpga_bist_icap_compute_rule18(const FpgaBistIcapState *state);

#endif /* AUNCIENT_FPGA_BIST_ICAP_THEOREMS_451_455_H */
