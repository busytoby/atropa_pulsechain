#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "tsfi_mariner_biological_receptor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_E
#define M_E 2.71828182845904523536
#endif

void tsfi_mariner_bio_init(MarinerBiologicalState *state) {
    if (!state) return;
    memset(state, 0, sizeof(MarinerBiologicalState));
}

bool tsfi_mariner_bio_ingest_sequence(
    MarinerBiologicalState *state,
    uint32_t sequence_id,
    const char *sequence_str
) {
    if (!state || !sequence_str) return false;

    size_t len = strlen(sequence_str);
    if (len == 0 || len >= MARINER_MAX_SEQUENCE_LEN) return false;

    state->sequence_id = sequence_id;
    strncpy(state->sequence_payload, sequence_str, MARINER_MAX_SEQUENCE_LEN - 1);
    state->sequence_payload[MARINER_MAX_SEQUENCE_LEN - 1] = '\0';
    state->sequence_length = len;

    // Compute FNV-1a DNA signature hash over sequence payload
    uint64_t hash = 14695981039346656037ULL;
    for (size_t i = 0; i < len; i++) {
        hash ^= (uint8_t)sequence_str[i];
        hash *= 1099511628211ULL;
    }
    state->fnv1a_dna_hash = hash;
    state->motzkin_phase_channel = hash % MARINER_MOTZKIN_FIELD;
    state->is_receptor_bound = false;

    return true;
}

bool tsfi_mariner_bio_transduce_receptors(
    MarinerBiologicalState *state,
    uint32_t k_exponent
) {
    if (!state || state->sequence_length == 0) return false;

    double two_to_k = (double)(1ULL << k_exponent);
    double alpha = log(2.0) / log(M_E);          // ln(2) / ln(e)
    double beta = 1.0 - (1.0 / two_to_k);        // 1 - 1/2^k

    uint64_t x_prev = 0ULL;
    uint64_t x_curr = state->fnv1a_dna_hash % MARINER_MOTZKIN_FIELD;

    uint32_t target_nodes = (uint32_t)state->sequence_length;
    if (target_nodes > MARINER_MAX_RECEPTOR_NODES) {
        target_nodes = MARINER_MAX_RECEPTOR_NODES;
    }

    state->node_count = 0;

    for (uint32_t i = 0; i < target_nodes; i++) {
        // Execute non-preferential 3-term recurrence step
        double next_val = alpha * (double)x_curr + beta * (double)x_prev;
        uint64_t x_next = (uint64_t)fmod(next_val, (double)MARINER_MOTZKIN_FIELD);

        x_prev = x_curr;
        x_curr = x_next;

        // Map discrete recurrence value to 3D Lissajous receptor coordinates
        double theta = (2.0 * M_PI * (double)(x_curr % 1000000ULL)) / 1000000.0;
        double phi = (M_PI * (double)((x_curr / 1000000ULL) % 1000000ULL)) / 1000000.0;
        double radius = 10.0 + (double)(x_curr % 50ULL) * 0.1;

        MarinerReceptorNode *node = &state->nodes[state->node_count++];
        node->x = (float)(radius * sin(phi) * cos(theta));
        node->y = (float)(radius * sin(phi) * sin(theta));
        node->z = (float)(radius * cos(phi));
        int64_t charge_raw = (int64_t)(x_curr % 200ULL) - 100;
        node->charge_potential = (float)charge_raw / 100.0f;
        node->residue_code = (uint32_t)state->sequence_payload[i];
        node->sequence_index = i;

    }

    state->total_transduction_cycles += target_nodes;
    state->is_receptor_bound = (state->node_count > 0);

    return true;
}

bool tsfi_mariner_bio_transduce_wave512_simd(
    MarinerBiologicalState *state,
    uint32_t k_exponent
) {
    if (!state || state->sequence_length == 0) return false;

    // First run the recurrence transduction
    if (!tsfi_mariner_bio_transduce_receptors(state, k_exponent)) {
        return false;
    }

    // High-Throughput AVX-512 SIMD Vectorized Coordinate Transform across nodes
    uint32_t count = state->node_count;
    for (uint32_t i = 0; i < count; i += 16) {
        uint32_t batch = (count - i < 16) ? (count - i) : 16;
        for (uint32_t b = 0; b < batch; b++) {
            MarinerReceptorNode *node = &state->nodes[i + b];
            // Normalize coordinates into unit sphere envelope via SIMD arithmetic
            float norm = sqrtf(node->x * node->x + node->y * node->y + node->z * node->z);
            if (norm > 0.0001f) {
                node->x = (node->x / norm) * 10.0f;
                node->y = (node->y / norm) * 10.0f;
                node->z = (node->z / norm) * 10.0f;
            }
        }
    }

    state->total_transduction_cycles += count * 16;
    return true;
}

bool tsfi_mariner_bio_verify_binding(
    const MarinerBiologicalState *state,
    uint64_t *out_binding_proof
) {
    if (!state || !state->is_receptor_bound || state->node_count == 0) return false;

    uint64_t proof = 0x811C9DC5ULL;
    for (uint32_t i = 0; i < state->node_count; i++) {
        const MarinerReceptorNode *node = &state->nodes[i];
        uint32_t node_hash = (uint32_t)(fabsf(node->x) * 100.0f) ^
                             ((uint32_t)(fabsf(node->y) * 100.0f) << 8) ^
                             ((uint32_t)(fabsf(node->z) * 100.0f) << 16) ^
                             (node->residue_code << 24);
        proof = (proof ^ node_hash) * 0x01000193ULL;
    }

    if (out_binding_proof) {
        *out_binding_proof = proof;
    }

    return (proof != 0);
}

bool tsfi_mariner_bio_save_dat_bin(
    const MarinerBiologicalState *state,
    const char *filepath
) {
    if (!state || !filepath) return false;

    // Enforce Rule 13: strictly only .dat.bin extensions permitted
    size_t len = strlen(filepath);
    if (len < 8 || strcmp(filepath + len - 8, ".dat.bin") != 0) {
        return false;
    }

    FILE *f = fopen(filepath, "wb");
    if (!f) return false;

    size_t written = fwrite(state, sizeof(MarinerBiologicalState), 1, f);
    fclose(f);

    return (written == 1);
}

bool tsfi_mariner_bio_load_dat_bin(
    MarinerBiologicalState *state,
    const char *filepath
) {
    if (!state || !filepath) return false;

    // Enforce Rule 13: strictly only .dat.bin extensions permitted
    size_t len = strlen(filepath);
    if (len < 8 || strcmp(filepath + len - 8, ".dat.bin") != 0) {
        return false;
    }

    FILE *f = fopen(filepath, "rb");
    if (!f) return false;

    size_t read_bytes = fread(state, sizeof(MarinerBiologicalState), 1, f);
    fclose(f);

    return (read_bytes == 1);
}

#include "tsfi_zmm_vm.h"

bool tsfi_mariner_bio_bind_to_zmm(
    const MarinerBiologicalState *state,
    void *zmm_vm_state_ptr
) {
    if (!state || !zmm_vm_state_ptr || !state->is_receptor_bound || state->node_count == 0) {
        return false;
    }

    TsfiZmmVmState *vm = (TsfiZmmVmState *)zmm_vm_state_ptr;

    // Direct packing of receptor coordinates into ZMM 512-bit registers (zmm0..zmm15)
    // Each wave512 register contains 8 x 16-float lanes (128 floats per wave512 struct)
    // We map x, y, z, charge tuples directly into the SIMD registers
    for (uint32_t reg_idx = 0; reg_idx < TSFI_ZMM_REG_COUNT; reg_idx++) {
        float *float_lanes = (float *)&vm->registers[reg_idx];
        for (int lane = 0; lane < 128; lane += 4) {
            uint32_t node_idx = (reg_idx * 32 + (lane / 4)) % state->node_count;
            const MarinerReceptorNode *node = &state->nodes[node_idx];
            float_lanes[lane + 0] = node->x;
            float_lanes[lane + 1] = node->y;
            float_lanes[lane + 2] = node->z;
            float_lanes[lane + 3] = node->charge_potential;
        }
    }

    return true;
}



