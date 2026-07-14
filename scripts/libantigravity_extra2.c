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

int interop_coaxial_map_row_to_entity(const int *table_data, size_t row_stride, size_t row_idx, size_t col_idx, int *out_entity_id) {
    if (!table_data || !out_entity_id) return -1;
    *out_entity_id = table_data[row_idx * row_stride + col_idx];
    return 0;
}

int interop_coaxial_semantic_join(const int *h_ids, const int *t_ids, size_t row_count, const float *embeddings, size_t dim, const float *r, float margin, int *out_matched_flags) {
    if (!h_ids || !t_ids || row_count == 0 || !embeddings || dim == 0 || !r || !out_matched_flags) return -1;
    int matched_count = 0;
    for (size_t i = 0; i < row_count; i++) {
        const float *h = embeddings + h_ids[i] * dim;
        const float *t = embeddings + t_ids[i] * dim;
        float dist_sq = 0.0f;
        for (size_t j = 0; j < dim; j++) {
            float diff = h[j] + r[j] - t[j];
            dist_sq += diff * diff;
        }
        if (dist_sq <= margin) {
            out_matched_flags[i] = 1;
            matched_count++;
        } else {
            out_matched_flags[i] = 0;
        }
    }
    return matched_count;
}

float interop_ctransr_cluster_constraint(const float *M1, const float *M2, size_t dim) {
    if (!M1 || !M2 || dim == 0) return 0.0f;
    float sum_sq = 0.0f;
    for (size_t i = 0; i < dim; i++) {
        float diff = M1[i] - M2[i];
        sum_sq += diff * diff;
    }
    return sum_sq;
}

int interop_transr_scale_translation(float *r, size_t rel_dim, size_t ent_dim) {
    if (!r || ent_dim == 0 || rel_dim == 0) return -1;
    float scale = sqrtf((float)rel_dim / (float)ent_dim);
    for (size_t i = 0; i < rel_dim; i++) {
        r[i] *= scale;
    }
    return 0;
}

float interop_transr_mrr_eval(const int *ranks, size_t count) {
    if (!ranks || count == 0) return 0.0f;
    float sum = 0.0f;
    for (size_t i = 0; i < count; i++) {
        if (ranks[i] > 0) {
            sum += 1.0f / (float)ranks[i];
        }
    }
    return sum / (float)count;
}
