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

size_t interop_transr_adaptive_dimension(size_t base_dim, size_t n_heads, size_t n_tails, size_t min_dim, size_t max_dim) {
    float factor = sqrtf((float)(n_heads + n_tails) / 2.0f);
    size_t dim = (size_t)(roundf((float)base_dim * factor));
    if (dim < min_dim) return min_dim;
    if (dim > max_dim) return max_dim;
    return dim;
}

int interop_transr_orthogonal_gradient_gate(const float *G, const float *v, float *out_G, size_t rel_dim, size_t ent_dim) {
    if (!G || !v || !out_G || rel_dim == 0 || ent_dim == 0) return -1;
    float v_norm_sq = 0.0f;
    for (size_t j = 0; j < ent_dim; j++) {
        v_norm_sq += v[j] * v[j];
    }
    for (size_t i = 0; i < rel_dim; i++) {
        float dot = 0.0f;
        for (size_t j = 0; j < ent_dim; j++) {
            dot += G[i * ent_dim + j] * v[j];
        }
        for (size_t j = 0; j < ent_dim; j++) {
            if (v_norm_sq > 0.0f) {
                out_G[i * ent_dim + j] = G[i * ent_dim + j] - (dot / v_norm_sq) * v[j];
            } else {
                out_G[i * ent_dim + j] = G[i * ent_dim + j];
            }
        }
    }
    return 0;
}

float interop_transr_regularization(const float *M, size_t ent_dim, size_t rel_dim) {
    if (!M || ent_dim == 0 || rel_dim == 0) return 0.0f;
    float sum_sq = 0.0f;
    size_t total = ent_dim * rel_dim;
    for (size_t i = 0; i < total; i++) {
        sum_sq += M[i] * M[i];
    }
    return sum_sq;
}

int interop_rbgraph_resolve_black_height(const int *node_colors, const int *parent_indices, size_t count, size_t node_idx) {
    if (!node_colors || !parent_indices || node_idx >= count) return -1;
    int is_leaf = 1;
    for (size_t i = 0; i < count; i++) {
        if (parent_indices[i] == (int)node_idx) {
            is_leaf = 0;
            break;
        }
    }
    if (is_leaf) return 0;
    int child_height = -2;
    for (size_t i = 0; i < count; i++) {
        if (parent_indices[i] == (int)node_idx) {
            int h = interop_rbgraph_resolve_black_height(node_colors, parent_indices, count, i);
            if (h < 0) return -1;
            int path_h = h + (node_colors[i] == 0 ? 1 : 0);
            if (child_height == -2) {
                child_height = path_h;
            } else if (child_height != path_h) {
                return -1;
            }
        }
    }
    return child_height;
}

int interop_rbgraph_validate_properties(const int *node_colors, const int *parent_indices, size_t count) {
    if (!node_colors || !parent_indices || count == 0) return -1;
    int root_count = 0;
    int root_idx = -1;
    for (size_t i = 0; i < count; i++) {
        if (parent_indices[i] < 0) {
            root_count++;
            root_idx = (int)i;
        }
    }
    if (root_count != 1) return 0;
    if (node_colors[root_idx] != 0) return 0;
    for (size_t i = 0; i < count; i++) {
        if (node_colors[i] == 1) {
            int parent = parent_indices[i];
            if (parent >= 0 && parent < (int)count) {
                if (node_colors[parent] == 1) return 0;
            }
        }
    }
    int root_bh = interop_rbgraph_resolve_black_height(node_colors, parent_indices, count, (size_t)root_idx);
    if (root_bh < 0) return 0;
    return 1;
}

int interop_ctransr_adaptive_split_check(const float *scores, size_t count, float threshold) {
    if (!scores || count <= 1) return 0;
    float sum = 0.0f;
    for (size_t i = 0; i < count; i++) {
        sum += scores[i];
    }
    float mean = sum / (float)count;
    float var_sum = 0.0f;
    for (size_t i = 0; i < count; i++) {
        float diff = scores[i] - mean;
        var_sum += diff * diff;
    }
    float variance = var_sum / (float)count;
    return (variance > threshold) ? 1 : 0;
}

int interop_transr_project_bounds(float *v, size_t dim, float radius) {
    if (!v || dim == 0 || radius <= 0.0f) return -1;
    float sum_sq = 0.0f;
    for (size_t i = 0; i < dim; i++) {
        sum_sq += v[i] * v[i];
    }
    float norm = sqrtf(sum_sq);
    if (norm > radius) {
        float scale = radius / norm;
        for (size_t i = 0; i < dim; i++) {
            v[i] *= scale;
        }
    }
    return 0;
}

int interop_transd_score(const float *h, const float *hp, const float *t, const float *tp, const float *r, const float *rp, size_t ent_dim, size_t rel_dim, float *out_score) {
    if (!h || !hp || !t || !tp || !r || !rp || !out_score || ent_dim == 0 || rel_dim == 0) return -1;
    float hp_dot_h = 0.0f;
    for (size_t i = 0; i < ent_dim; i++) {
        hp_dot_h += hp[i] * h[i];
    }
    float tp_dot_t = 0.0f;
    for (size_t i = 0; i < ent_dim; i++) {
        tp_dot_t += tp[i] * t[i];
    }
    float sum_sq = 0.0f;
    for (size_t i = 0; i < rel_dim; i++) {
        float h_perp_i = hp_dot_h * rp[i];
        if (i < ent_dim) {
            h_perp_i += h[i];
        }
        float t_perp_i = tp_dot_t * rp[i];
        if (i < ent_dim) {
            t_perp_i += t[i];
        }
        float diff = h_perp_i + r[i] - t_perp_i;
        sum_sq += diff * diff;
    }
    *out_score = sqrtf(sum_sq);
    return 0;
}

int interop_transd_normalize_entity(float *e, float *ep, size_t dim) {
    if (!e || !ep || dim == 0) return -1;
    float sum_e = 0.0f;
    for (size_t i = 0; i < dim; i++) {
        sum_e += e[i] * e[i];
    }
    float norm_e = sqrtf(sum_e);
    if (norm_e > 1.0f) {
        for (size_t i = 0; i < dim; i++) {
            e[i] /= norm_e;
        }
    }
    float sum_ep = 0.0f;
    for (size_t i = 0; i < dim; i++) {
        sum_ep += ep[i] * ep[i];
    }
    float norm_ep = sqrtf(sum_ep);
    if (norm_ep > 1.0f) {
        for (size_t i = 0; i < dim; i++) {
            ep[i] /= norm_ep;
        }
    }
    return 0;
}

int interop_transd_scale_relation(float *r, size_t dim, float radius) {
    if (!r || dim == 0 || radius <= 0.0f) return -1;
    float sum_sq = 0.0f;
    for (size_t i = 0; i < dim; i++) {
        sum_sq += r[i] * r[i];
    }
    float norm = sqrtf(sum_sq);
    if (norm > radius) {
        float scale = radius / norm;
        for (size_t i = 0; i < dim; i++) {
            r[i] *= scale;
        }
    }
    return 0;
}
