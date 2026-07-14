#include <stddef.h>
#include <math.h>
#include "libantigravity_interop.h"

float interop_transr_score(const float *h, const float *M, const float *r, const float *t, size_t ent_dim, size_t rel_dim, int norm_type) {
    if (!h || !M || !r || !t || ent_dim == 0 || rel_dim == 0) return 0.0f;
    float score = 0.0f;
    for (size_t i = 0; i < rel_dim; i++) {
        float h_proj = 0.0f;
        float t_proj = 0.0f;
        for (size_t j = 0; j < ent_dim; j++) {
            h_proj += M[i * ent_dim + j] * h[j];
            t_proj += M[i * ent_dim + j] * t[j];
        }
        float diff = h_proj + r[i] - t_proj;
        if (norm_type == 1) {
            score += fabsf(diff);
        } else {
            score += diff * diff;
        }
    }
    return score;
}

int interop_transr_matrix_norm_constraint(float *M, size_t ent_dim, size_t rel_dim) {
    if (!M || ent_dim == 0 || rel_dim == 0) return -1;
    for (size_t i = 0; i < rel_dim; i++) {
        float sum_sq = 0.0f;
        for (size_t j = 0; j < ent_dim; j++) {
            sum_sq += M[i * ent_dim + j] * M[i * ent_dim + j];
        }
        float norm = sqrtf(sum_sq);
        if (norm > 1.0f) {
            for (size_t j = 0; j < ent_dim; j++) {
                M[i * ent_dim + j] /= norm;
            }
        }
    }
    return 0;
}

float interop_ctransr_score(const float *h, const float *M, const float *r_cluster, const float *t, size_t ent_dim, size_t rel_dim, int norm_type) {
    return interop_transr_score(h, M, r_cluster, t, ent_dim, rel_dim, norm_type);
}

float interop_transr_alignment_penalty(const float *M, const float *r, size_t ent_dim, size_t rel_dim) {
    if (!M || !r || ent_dim == 0 || rel_dim == 0) return 0.0f;
    float sum_sq = 0.0f;
    for (size_t j = 0; j < ent_dim; j++) {
        float dot = 0.0f;
        for (size_t i = 0; i < rel_dim; i++) {
            dot += M[i * ent_dim + j] * r[i];
        }
        sum_sq += dot * dot;
    }
    return sum_sq;
}
