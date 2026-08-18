#ifndef AUNCIENT_CROSS_VM_POLYMORPHIC_PROVING_THEOREMS_91_95_H
#define AUNCIENT_CROSS_VM_POLYMORPHIC_PROVING_THEOREMS_91_95_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define CROSS_VM_TPA_BASE 0x0100

typedef struct {
    uint32_t local_vm_id;
    uint32_t remote_vm_id;
    uint32_t imported_remote_witness_seal;
    float    local_efficiency_score;
    float    remote_efficiency_score;
    float    hybrid_efficiency_score;
    bool     remote_proof_imported;
    bool     manifold_intersection_non_empty;
    bool     hybrid_monotonic_gain_verified;
    bool     byzantine_tamper_resistance_verified;
    bool     cross_vm_scsi_closure_verified;
    uint32_t rule18_parity_checksum;
} CrossVmPolymorphicState;

void auncient_cross_vm_init(CrossVmPolymorphicState *state, uint32_t local_id, uint32_t remote_id);
bool auncient_cross_vm_import_remote_proof(CrossVmPolymorphicState *state, uint32_t remote_witness);
bool auncient_cross_vm_synthesize_hybrid_strategy(CrossVmPolymorphicState *state);
bool auncient_cross_vm_verify_theorems_91_95(CrossVmPolymorphicState *state);
uint32_t auncient_cross_vm_compute_rule18(const CrossVmPolymorphicState *state);

#endif /* AUNCIENT_CROSS_VM_POLYMORPHIC_PROVING_THEOREMS_91_95_H */
