#ifndef AUNCIENT_FPGA_QUADTREE_OS_THEOREMS_366_370_H
#define AUNCIENT_FPGA_QUADTREE_OS_THEOREMS_366_370_H

#include "auncient_fpga_mckeeman_quadrature_theorems_361_365.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    uint32_t cpm_tpa_base_alignment_bytes;
    float    quadtree_dat_bin_slice_integrity;
    float    jubilee_overdrive_clamping_margin_v;
    uint64_t verified_quadtree_os_cycles;
    bool     quadtree_dat_bin_os_boot_verified;   /* Theorem 366 */
    bool     jubilee_quadtree_overdrive_verified; /* Theorem 367 */
    bool     wmq_scsi_cpm_bdos_verified;          /* Theorem 368 */
    bool     ankh_cpm_lossless_saat_verified;     /* Theorem 369 */
    bool     quadtree_os_grand_parity_verified;   /* Theorem 370 */
    uint32_t rule18_parity_checksum;
} FpgaQuadtreeOsState;

void auncient_fpga_quadtree_os_init(FpgaQuadtreeOsState *state);
bool auncient_fpga_quadtree_os_verify_theorems_366_370(FpgaQuadtreeOsState *state);
uint32_t auncient_fpga_quadtree_os_compute_rule18(const FpgaQuadtreeOsState *state);

#endif /* AUNCIENT_FPGA_QUADTREE_OS_THEOREMS_366_370_H */
