#include "tsfi_mann_controller.h"
#include <math.h>
#include <string.h>

void tsfi_mann_controller_init(TSFiMANNController *mann) {
    if (!mann) return;
    memset(mann->memory_matrix, 0, sizeof(mann->memory_matrix));
    memset(mann->usage_vector, 0, sizeof(mann->usage_vector));
    memset(mann->temporal_link, 0, sizeof(mann->temporal_link));
    
    // Set default gate weights
    mann->write_gate = 0.5f;
    mann->read_gate = 0.5f;
    mann->allocation_gate = 0.5f;
}

static float compute_cosine_similarity(const float *u, const float *v, int len) {
    float dot = 0.0f;
    float norm_u = 0.0f;
    float norm_v = 0.0f;
    for (int i = 0; i < len; i++) {
        dot += u[i] * v[i];
        norm_u += u[i] * u[i];
        norm_v += v[i] * v[i];
    }
    if (norm_u <= 1e-8f || norm_v <= 1e-8f) return 0.0f;
    return dot / (sqrtf(norm_u) * sqrtf(norm_v));
}

void tsfi_mann_controller_write(TSFiMANNController *mann, 
                               const float *key_vector, 
                               float strength, 
                               const float *erase_vector, 
                               const float *write_vector) {
    if (!mann || !key_vector || !erase_vector || !write_vector) return;

    // 1. Content addressing weights using cosine similarity and softmax
    float content_weights[MANN_MEMORY_SLOTS];
    float sum_exp = 0.0f;
    for (int i = 0; i < MANN_MEMORY_SLOTS; i++) {
        float sim = compute_cosine_similarity(key_vector, mann->memory_matrix[i], MANN_SLOT_WIDTH);
        content_weights[i] = expf(strength * sim);
        sum_exp += content_weights[i];
    }
    if (sum_exp > 1e-8f) {
        for (int i = 0; i < MANN_MEMORY_SLOTS; i++) {
            content_weights[i] /= sum_exp;
        }
    } else {
        for (int i = 0; i < MANN_MEMORY_SLOTS; i++) {
            content_weights[i] = 1.0f / MANN_MEMORY_SLOTS;
        }
    }

    // 2. Allocation addressing weights based on least used slots
    float allocation_weights[MANN_MEMORY_SLOTS];
    float sum_alloc_exp = 0.0f;
    for (int i = 0; i < MANN_MEMORY_SLOTS; i++) {
        // Higher weight to lower usage
        allocation_weights[i] = expf(-mann->usage_vector[i]);
        sum_alloc_exp += allocation_weights[i];
    }
    if (sum_alloc_exp > 1e-8f) {
        for (int i = 0; i < MANN_MEMORY_SLOTS; i++) {
            allocation_weights[i] /= sum_alloc_exp;
        }
    } else {
        for (int i = 0; i < MANN_MEMORY_SLOTS; i++) {
            allocation_weights[i] = 1.0f / MANN_MEMORY_SLOTS;
        }
    }

    // 3. Blend write weights using allocation gate
    float write_weights[MANN_MEMORY_SLOTS];
    for (int i = 0; i < MANN_MEMORY_SLOTS; i++) {
        write_weights[i] = mann->allocation_gate * allocation_weights[i] + 
                           (1.0f - mann->allocation_gate) * content_weights[i];
    }

    // 4. Update memory matrix
    for (int i = 0; i < MANN_MEMORY_SLOTS; i++) {
        float w = mann->write_gate * write_weights[i];
        for (int j = 0; j < MANN_SLOT_WIDTH; j++) {
            mann->memory_matrix[i][j] = mann->memory_matrix[i][j] * (1.0f - w * erase_vector[j]) + 
                                        w * write_vector[j];
        }
        
        // Update usage: write access increments usage
        mann->usage_vector[i] += w;
    }
}

void tsfi_mann_controller_read(TSFiMANNController *mann, 
                              const float *key_vector, 
                              float strength, 
                              float *out_read_vector) {
    if (!mann || !key_vector || !out_read_vector) return;

    // 1. Content addressing weights using cosine similarity and softmax
    float read_weights[MANN_MEMORY_SLOTS];
    float sum_exp = 0.0f;
    for (int i = 0; i < MANN_MEMORY_SLOTS; i++) {
        float sim = compute_cosine_similarity(key_vector, mann->memory_matrix[i], MANN_SLOT_WIDTH);
        read_weights[i] = expf(strength * sim);
        sum_exp += read_weights[i];
    }
    if (sum_exp > 1e-8f) {
        for (int i = 0; i < MANN_MEMORY_SLOTS; i++) {
            read_weights[i] /= sum_exp;
        }
    } else {
        for (int i = 0; i < MANN_MEMORY_SLOTS; i++) {
            read_weights[i] = 1.0f / MANN_MEMORY_SLOTS;
        }
    }

    // 2. Read out weighted sum of memory matrix rows
    memset(out_read_vector, 0, MANN_SLOT_WIDTH * sizeof(float));
    for (int i = 0; i < MANN_MEMORY_SLOTS; i++) {
        float w = mann->read_gate * read_weights[i];
        for (int j = 0; j < MANN_SLOT_WIDTH; j++) {
            out_read_vector[j] += w * mann->memory_matrix[i][j];
        }
        
        // Update usage: read access increments usage slightly
        mann->usage_vector[i] += w * 0.1f;
    }
}

void tsfi_mann_controller_load_usd_gates(TSFiMANNController *mann, const usd_auncient_mann_api_t *mann_api) {
    if (!mann || !mann_api) return;
    mann->write_gate = usd_mann_api_get_write_gate(mann_api);
    mann->read_gate = usd_mann_api_get_read_gate(mann_api);
    mann->allocation_gate = usd_mann_api_get_allocation_gate(mann_api);
}
