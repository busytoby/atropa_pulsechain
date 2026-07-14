#include <stddef.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
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

int interop_transm_score(const float *h, const float *t, const float *r, float wr, size_t dim, float *out_score) {
    if (!h || !t || !r || !out_score || dim == 0 || wr <= 0.0f) return -1;
    float sum_sq = 0.0f;
    for (size_t i = 0; i < dim; i++) {
        float diff = h[i] + r[i] - t[i];
        sum_sq += diff * diff;
    }
    *out_score = sqrtf(sum_sq) / wr;
    return 0;
}

int interop_transa_score(const float *h, const float *t, const float *r, const float *w, size_t dim, float *out_score) {
    if (!h || !t || !r || !w || !out_score || dim == 0) return -1;
    float sum = 0.0f;
    for (size_t i = 0; i < dim; i++) {
        float diff = h[i] + r[i] - t[i];
        sum += w[i] * (diff < 0.0f ? -diff : diff);
    }
    *out_score = sum;
    return 0;
}

int interop_transf_score(const float *h, const float *t, const float *r, size_t dim, float *out_score) {
    if (!h || !t || !r || !out_score || dim == 0) return -1;
    float sum = 0.0f;
    for (size_t i = 0; i < dim; i++) {
        sum += (h[i] + r[i]) * t[i] + (t[i] - r[i]) * h[i];
    }
    *out_score = sum;
    return 0;
}

int interop_transg_score(const float *h, const float *t, const float *r_clusters, const float *weights, size_t num_clusters, size_t dim, float *out_score) {
    if (!h || !t || !r_clusters || !weights || !out_score || num_clusters == 0 || dim == 0) return -1;
    float score = 0.0f;
    for (size_t m = 0; m < num_clusters; m++) {
        float sum_sq = 0.0f;
        for (size_t i = 0; i < dim; i++) {
            float diff = h[i] + r_clusters[m * dim + i] - t[i];
            sum_sq += diff * diff;
        }
        score += weights[m] * sqrtf(sum_sq);
    }
    *out_score = score;
    return 0;
}

int interop_transsparse_score(const float *h, const float *t, const float *M_vals, const int *M_cols, const int *M_row_ptrs, size_t dim, float *out_score) {
    if (!h || !t || !M_vals || !M_cols || !M_row_ptrs || !out_score || dim == 0) return -1;
    float *proj_h = calloc(dim, sizeof(float));
    float *proj_t = calloc(dim, sizeof(float));
    if (!proj_h || !proj_t) {
        free(proj_h);
        free(proj_t);
        return -1;
    }
    for (size_t r = 0; r < dim; r++) {
        int start = M_row_ptrs[r];
        int end = M_row_ptrs[r + 1];
        float sum_h = 0.0f;
        float sum_t = 0.0f;
        for (int idx = start; idx < end; idx++) {
            int c = M_cols[idx];
            sum_h += M_vals[idx] * h[c];
            sum_t += M_vals[idx] * t[c];
        }
        proj_h[r] = sum_h;
        proj_t[r] = sum_t;
    }
    float sum_sq = 0.0f;
    for (size_t i = 0; i < dim; i++) {
        float diff = proj_h[i] - proj_t[i];
        sum_sq += diff * diff;
    }
    *out_score = sqrtf(sum_sq);
    free(proj_h);
    free(proj_t);
    return 0;
}

int interop_index_ivf_assign(const float *query, const float *centroids, size_t num_centroids, size_t dim, int *out_centroid_indices, size_t top_c) {
    if (!query || !centroids || !out_centroid_indices || num_centroids == 0 || dim == 0 || top_c == 0 || top_c > num_centroids) return -1;
    float *distances = calloc(num_centroids, sizeof(float));
    int *indices = calloc(num_centroids, sizeof(int));
    if (!distances || !indices) {
        free(distances);
        free(indices);
        return -1;
    }
    for (size_t c = 0; c < num_centroids; c++) {
        float sum_sq = 0.0f;
        for (size_t i = 0; i < dim; i++) {
            float diff = query[i] - centroids[c * dim + i];
            sum_sq += diff * diff;
        }
        distances[c] = sum_sq;
        indices[c] = (int)c;
    }
    for (size_t i = 0; i < num_centroids - 1; i++) {
        for (size_t j = 0; j < num_centroids - i - 1; j++) {
            if (distances[j] > distances[j + 1]) {
                float tmp_dist = distances[j];
                distances[j] = distances[j + 1];
                distances[j + 1] = tmp_dist;
                int tmp_idx = indices[j];
                indices[j] = indices[j + 1];
                indices[j + 1] = tmp_idx;
            }
        }
    }
    for (size_t i = 0; i < top_c; i++) {
        out_centroid_indices[i] = indices[i];
    }
    free(distances);
    free(indices);
    return 0;
}

int interop_index_pq_quantize(const float *sub_vec, const float *codebook, size_t num_codes, size_t sub_dim, int *out_code) {
    if (!sub_vec || !codebook || !out_code || num_codes == 0 || sub_dim == 0) return -1;
    float min_dist = 1e30f;
    int best_code = -1;
    for (size_t c = 0; c < num_codes; c++) {
        float sum_sq = 0.0f;
        for (size_t i = 0; i < sub_dim; i++) {
            float diff = sub_vec[i] - codebook[c * sub_dim + i];
            sum_sq += diff * diff;
        }
        if (sum_sq < min_dist) {
            min_dist = sum_sq;
            best_code = (int)c;
        }
    }
    *out_code = best_code;
    return 0;
}

int interop_index_lsh_hash(const float *v, const float *projection_planes, size_t dim, size_t num_planes, uint32_t *out_hash) {
    if (!v || !projection_planes || !out_hash || dim == 0 || num_planes == 0 || num_planes > 32) return -1;
    uint32_t hash = 0;
    for (size_t p = 0; p < num_planes; p++) {
        float dot = 0.0f;
        for (size_t i = 0; i < dim; i++) {
            dot += v[i] * projection_planes[p * dim + i];
        }
        if (dot >= 0.0f) {
            hash |= (1U << p);
        }
    }
    *out_hash = hash;
    return 0;
}

int interop_zmm_parse_hex_address(const char *address_hex, uint8_t *out_address) {
    if (!address_hex || !out_address) return -1;
    size_t start = 0;
    if (address_hex[0] == '0' && (address_hex[1] == 'x' || address_hex[1] == 'X')) {
        start = 2;
    }
    for (size_t i = 0; i < 20; i++) {
        char high = address_hex[start + i * 2];
        char low = address_hex[start + i * 2 + 1];
        if (high == '\0' || low == '\0') return -1;
        uint8_t byte_val = 0;
        if (high >= '0' && high <= '9') byte_val += (high - '0') << 4;
        else if (high >= 'a' && high <= 'f') byte_val += (high - 'a' + 10) << 4;
        else if (high >= 'A' && high <= 'F') byte_val += (high - 'A' + 10) << 4;
        else return -1;
        if (low >= '0' && low <= '9') byte_val += (low - '0');
        else if (low >= 'a' && low <= 'f') byte_val += (low - 'a' + 10);
        else if (low >= 'A' && low <= 'F') byte_val += (low - 'A' + 10);
        else return -1;
        out_address[i] = byte_val;
    }
    return 0;
}

int interop_zmm_resolve_contract_address(const uint8_t *address, const uint8_t *registry_addresses, const uint64_t *page_indices, size_t count, uint64_t *out_page_idx) {
    if (!address || !registry_addresses || !page_indices || !out_page_idx || count == 0) return -1;
    for (size_t idx = 0; idx < count; idx++) {
        int match = 1;
        for (size_t b = 0; b < 20; b++) {
            if (registry_addresses[idx * 20 + b] != address[b]) {
                match = 0;
                break;
            }
        }
        if (match) {
            *out_page_idx = page_indices[idx];
            return 0;
        }
    }
    return -2;
}

int interop_zmm_verify_contract_state(const uint8_t *address, const uint8_t *state_payload, size_t size, uint32_t expected_checksum) {
    if (!address || !state_payload || size == 0) return -1;
    uint32_t checksum = 5381;
    for (size_t i = 0; i < 20; i++) {
        checksum = ((checksum << 5) + checksum) + address[i];
    }
    for (size_t i = 0; i < size; i++) {
        checksum = ((checksum << 5) + checksum) + state_payload[i];
    }
    return (checksum == expected_checksum) ? 1 : 0;
}

int interop_graph_path_walk(const int *edges_src, const int *edges_rel, const int *edges_dst, size_t num_edges, int seed_entity, const int *relation_path, size_t path_len, int *out_dst, size_t max_dst, size_t *out_count) {
    if (!edges_src || !edges_rel || !edges_dst || num_edges == 0 || !relation_path || path_len == 0 || !out_dst || max_dst == 0 || !out_count) return -1;
    int *current_states = calloc(num_edges + 1, sizeof(int));
    int *next_states = calloc(num_edges + 1, sizeof(int));
    if (!current_states || !next_states) {
        free(current_states);
        free(next_states);
        return -1;
    }
    size_t current_count = 0;
    current_states[current_count++] = seed_entity;
    for (size_t p = 0; p < path_len; p++) {
        int target_rel = relation_path[p];
        size_t next_count = 0;
        for (size_t s = 0; s < current_count; s++) {
            int u = current_states[s];
            for (size_t e = 0; e < num_edges; e++) {
                if (edges_src[e] == u && edges_rel[e] == target_rel) {
                    int v = edges_dst[e];
                    int exists = 0;
                    for (size_t n = 0; n < next_count; n++) {
                        if (next_states[n] == v) {
                            exists = 1;
                            break;
                        }
                    }
                    if (!exists && next_count < num_edges) {
                        next_states[next_count++] = v;
                    }
                }
            }
        }
        if (next_count == 0) {
            *out_count = 0;
            free(current_states);
            free(next_states);
            return 0;
        }
        int *temp = current_states;
        current_states = next_states;
        next_states = temp;
        current_count = next_count;
    }
    size_t copy_limit = (current_count < max_dst) ? current_count : max_dst;
    for (size_t i = 0; i < copy_limit; i++) {
        out_dst[i] = current_states[i];
    }
    *out_count = current_count;
    free(current_states);
    free(next_states);
    return 0;
}

int interop_graph_subgraph_extract(const int *edges_src, const int *edges_rel, const int *edges_dst, size_t num_edges, int seed_entity, size_t hops, int *out_subgraph_edges, size_t max_subgraph, size_t *out_count) {
    if (!edges_src || !edges_rel || !edges_dst || num_edges == 0 || !out_subgraph_edges || max_subgraph == 0 || !out_count) return -1;
    int *visited_entities = calloc(num_edges + 1, sizeof(int));
    int *new_entities = calloc(num_edges + 1, sizeof(int));
    if (!visited_entities || !new_entities) {
        free(visited_entities);
        free(new_entities);
        return -1;
    }
    size_t visited_count = 0;
    visited_entities[visited_count++] = seed_entity;
    size_t subgraph_count = 0;
    for (size_t h = 0; h < hops; h++) {
        size_t new_count = 0;
        for (size_t s = 0; s < visited_count; s++) {
            int u = visited_entities[s];
            for (size_t e = 0; e < num_edges; e++) {
                if (edges_src[e] == u) {
                    int v = edges_dst[e];
                    int edge_already_added = 0;
                    for (size_t idx = 0; idx < subgraph_count; idx++) {
                        if (out_subgraph_edges[idx] == (int)e) {
                            edge_already_added = 1;
                            break;
                        }
                    }
                    if (!edge_already_added && subgraph_count < max_subgraph) {
                        out_subgraph_edges[subgraph_count++] = (int)e;
                    }
                    int already_visited = 0;
                    for (size_t idx = 0; idx < visited_count; idx++) {
                        if (visited_entities[idx] == v) {
                            already_visited = 1;
                            break;
                        }
                    }
                    int already_new = 0;
                    for (size_t idx = 0; idx < new_count; idx++) {
                        if (new_entities[idx] == v) {
                            already_new = 1;
                            break;
                        }
                    }
                    if (!already_visited && !already_new && new_count < num_edges) {
                        new_entities[new_count++] = v;
                    }
                }
            }
        }
        if (new_count == 0) break;
        for (size_t i = 0; i < new_count; i++) {
            if (visited_count < num_edges + 1) {
                visited_entities[visited_count++] = new_entities[i];
            }
        }
    }
    *out_count = subgraph_count;
    free(visited_entities);
    free(new_entities);
    return 0;
}

int interop_graph_semantic_select(const float *h, const float *r, const float *all_entities, size_t num_entities, size_t dim, float threshold, int *out_entity_indices, size_t max_out, size_t *out_count) {
    if (!h || !r || !all_entities || num_entities == 0 || dim == 0 || !out_entity_indices || max_out == 0 || !out_count) return -1;
    size_t count = 0;
    for (size_t idx = 0; idx < num_entities; idx++) {
        float sum_sq = 0.0f;
        for (size_t i = 0; i < dim; i++) {
            float expected_t_i = h[i] + r[i];
            float diff = expected_t_i - all_entities[idx * dim + i];
            sum_sq += diff * diff;
        }
        float distance = sqrtf(sum_sq);
        if (distance < threshold) {
            if (count < max_out) {
                out_entity_indices[count++] = (int)idx;
            }
        }
    }
    *out_count = count;
    return 0;
}

int interop_conv_update_context(int *context_history, size_t *history_len, size_t max_history, int new_entity_id) {
    if (!context_history || !history_len || max_history == 0) return -1;
    size_t curr_len = *history_len;
    size_t new_len = (curr_len < max_history) ? (curr_len + 1) : max_history;
    for (size_t i = new_len - 1; i > 0; i--) {
        context_history[i] = context_history[i - 1];
    }
    context_history[0] = new_entity_id;
    *history_len = new_len;
    return 0;
}

int interop_conv_link_entity(const char *query_tokens, const char *entity_names, const int *entity_ids, size_t num_entities, int *out_entity_id) {
    if (!query_tokens || !entity_names || !entity_ids || !out_entity_id || num_entities == 0) return -1;
    const char *p = entity_names;
    for (size_t idx = 0; idx < num_entities; idx++) {
        const char *end = p;
        while (*end != '\0' && *end != ',') {
            end++;
        }
        size_t len = end - p;
        if (len > 0) {
            int match = 0;
            const char *q = query_tokens;
            while (*q != '\0') {
                int possible_match = 1;
                for (size_t i = 0; i < len; i++) {
                    if (q[i] == '\0' || p[i] != q[i]) {
                        possible_match = 0;
                        break;
                    }
                }
                if (possible_match) {
                    match = 1;
                    break;
                }
                q++;
            }
            if (match) {
                *out_entity_id = entity_ids[idx];
                return 0;
            }
        }
        if (*end == '\0') break;
        p = end + 1;
    }
    return -2;
}

int interop_conv_serialize_path(int src_id, int rel_id, int dst_id, char *out_buffer, size_t max_len) {
    if (!out_buffer || max_len == 0) return -1;
    int written = snprintf(out_buffer, max_len, "Entity_%d via relation_%d maps to entity_%d.", src_id, rel_id, dst_id);
    return (written > 0 && (size_t)written < max_len) ? 0 : -2;
}

int interop_conv_search_query(const char *query_text, const char *entity_names, const int *entity_ids, size_t num_entities, const int *edges_src, const int *edges_rel, const int *edges_dst, size_t num_edges, int *context_history, size_t *history_len, size_t max_history, int target_rel, char *out_response, size_t max_resp_len) {
    if (!query_text || !entity_names || !entity_ids || num_entities == 0 || !edges_src || !edges_rel || !edges_dst || num_edges == 0 || !context_history || !history_len || max_history == 0 || !out_response || max_resp_len == 0) return -1;
    int start_entity_id = -1;
    int link_res = interop_conv_link_entity(query_text, entity_names, entity_ids, num_entities, &start_entity_id);
    if (link_res != 0) {
        if (*history_len > 0) {
            start_entity_id = context_history[0];
        } else {
            return -2;
        }
    } else {
        interop_conv_update_context(context_history, history_len, max_history, start_entity_id);
    }
    int match_dst_id = -1;
    for (size_t e = 0; e < num_edges; e++) {
        if (edges_src[e] == start_entity_id && edges_rel[e] == target_rel) {
            match_dst_id = edges_dst[e];
            break;
        }
    }
    if (match_dst_id == -1) {
        return -3;
    }
    interop_conv_update_context(context_history, history_len, max_history, match_dst_id);
    return interop_conv_serialize_path(start_entity_id, target_rel, match_dst_id, out_response, max_resp_len);
}

int interop_qa_verify_fact(const int *edges_src, const int *edges_rel, const int *edges_dst, size_t num_edges, int s, int p, int o, int *out_exists) {
    if (!edges_src || !edges_rel || !edges_dst || num_edges == 0 || !out_exists) return -1;
    *out_exists = 0;
    for (size_t e = 0; e < num_edges; e++) {
        if (edges_src[e] == s && edges_rel[e] == p && edges_dst[e] == o) {
            *out_exists = 1;
            break;
        }
    }
    return 0;
}

int interop_qa_multi_hop_query(const int *edges_src, const int *edges_rel, const int *edges_dst, size_t num_edges, int seed, const int *path_rels, size_t path_len, int *out_dsts, size_t max_dsts, size_t *out_count) {
    return interop_graph_path_walk(edges_src, edges_rel, edges_dst, num_edges, seed, path_rels, path_len, out_dsts, max_dsts, out_count);
}

int interop_qa_get_contract_attribute(const uint8_t *state_payload, size_t payload_size, size_t offset, uint64_t *out_attribute) {
    if (!state_payload || payload_size == 0 || !out_attribute) return -1;
    if (offset + 8 > payload_size) return -2;
    uint64_t val = 0;
    for (int i = 0; i < 8; i++) {
        val |= ((uint64_t)state_payload[offset + i]) << (i * 8);
    }
    *out_attribute = val;
    return 0;
}

int interop_transh_project_hyperplane(const float *h, const float *w_r, size_t dim, float *out_h_proj) {
    if (!h || !w_r || !out_h_proj || dim == 0) return -1;
    float dot = 0.0f;
    for (size_t i = 0; i < dim; i++) {
        dot += w_r[i] * h[i];
    }
    for (size_t i = 0; i < dim; i++) {
        out_h_proj[i] = h[i] - dot * w_r[i];
    }
    return 0;
}

int interop_qa_classify_triple(const float *h, const float *r, const float *t, size_t dim, float threshold, int *out_valid) {
    if (!h || !r || !t || !out_valid || dim == 0) return -1;
    float sum_sq = 0.0f;
    for (size_t i = 0; i < dim; i++) {
        float diff = h[i] + r[i] - t[i];
        sum_sq += diff * diff;
    }
    float dist = sqrtf(sum_sq);
    *out_valid = (dist < threshold) ? 1 : 0;
    return 0;
}

int interop_transd_project_matrix(const float *h, const float *h_p, const float *r_p, size_t dim, float *out_h_proj) {
    if (!h || !h_p || !r_p || !out_h_proj || dim == 0) return -1;
    float dot = 0.0f;
    for (size_t i = 0; i < dim; i++) {
        dot += h_p[i] * h[i];
    }
    for (size_t i = 0; i < dim; i++) {
        out_h_proj[i] = r_p[i] * dot + h[i];
    }
    return 0;
}

int interop_conv_parse_relation(const char *query_tokens, const char *relation_names, const int *relation_ids, size_t num_relations, int *out_relation_id) {
    if (!query_tokens || !relation_names || !relation_ids || !out_relation_id || num_relations == 0) return -1;
    const char *p = relation_names;
    for (size_t idx = 0; idx < num_relations; idx++) {
        const char *end = p;
        while (*end != '\0' && *end != ',') {
            end++;
        }
        size_t len = end - p;
        if (len > 0) {
            int match = 0;
            const char *q = query_tokens;
            while (*q != '\0') {
                int possible_match = 1;
                for (size_t i = 0; i < len; i++) {
                    if (q[i] == '\0' || p[i] != q[i]) {
                        possible_match = 0;
                        break;
                    }
                }
                if (possible_match) {
                    match = 1;
                    break;
                }
                q++;
            }
            if (match) {
                *out_relation_id = relation_ids[idx];
                return 0;
            }
        }
        if (*end == '\0') break;
        p = end + 1;
    }
    return -2;
}

int interop_conv_resolve_coref(const int *context_history, size_t history_len, const int *entity_ids, const int *entity_types, size_t num_entities, int expected_type, int *out_resolved_id) {
    if (!context_history || history_len == 0 || !entity_ids || !entity_types || !out_resolved_id || num_entities == 0) return -1;
    for (size_t h = 0; h < history_len; h++) {
        int ent_id = context_history[h];
        for (size_t idx = 0; idx < num_entities; idx++) {
            if (entity_ids[idx] == ent_id && entity_types[idx] == expected_type) {
                *out_resolved_id = ent_id;
                return 0;
            }
        }
    }
    return -2;
}

int interop_conv_link_entity_fuzzy(const char *query_tokens, const char *entity_names, const int *entity_ids, size_t num_entities, int *out_entity_id) {
    if (!query_tokens || !entity_names || !entity_ids || !out_entity_id || num_entities == 0) return -1;
    const char *p = entity_names;
    for (size_t idx = 0; idx < num_entities; idx++) {
        const char *end = p;
        while (*end != '\0' && *end != ',') {
            end++;
        }
        size_t len = end - p;
        if (len >= 3) {
            size_t match_count = 0;
            const char *q = query_tokens;
            while (*q != '\0') {
                if (*q == p[match_count]) {
                    match_count++;
                    if (match_count == len || match_count >= 4) {
                        *out_entity_id = entity_ids[idx];
                        return 0;
                    }
                }
                q++;
            }
        }
        if (*end == '\0') break;
        p = end + 1;
    }
    return -2;
}

int interop_poly_add_sub_mod(const uint64_t *a, const uint64_t *b, size_t deg_a, size_t deg_b, uint64_t prime, int is_sub, uint64_t *out_c, size_t *out_deg) {
    if (!a || !b || !out_c || !out_deg || prime == 0) return -1;
    size_t max_deg = (deg_a > deg_b) ? deg_a : deg_b;
    size_t final_deg = 0;
    for (size_t i = 0; i <= max_deg; i++) {
        uint64_t val_a = (i <= deg_a) ? a[i] : 0;
        uint64_t val_b = (i <= deg_b) ? b[i] : 0;
        uint64_t val_c;
        if (is_sub) {
            if (val_a >= val_b) {
                val_c = (val_a - val_b) % prime;
            } else {
                val_c = (prime - ((val_b - val_a) % prime)) % prime;
            }
        } else {
            val_c = (val_a + val_b) % prime;
        }
        out_c[i] = val_c;
        if (val_c > 0) {
            final_deg = i;
        }
    }
    *out_deg = final_deg;
    return 0;
}

int interop_poly_multiply_mod(const uint64_t *a, const uint64_t *b, size_t deg_a, size_t deg_b, uint64_t prime, uint64_t *out_c, size_t *out_deg) {
    if (!a || !b || !out_c || !out_deg || prime == 0) return -1;
    size_t max_out_deg = deg_a + deg_b;
    for (size_t i = 0; i <= max_out_deg; i++) {
        out_c[i] = 0;
    }
    for (size_t i = 0; i <= deg_a; i++) {
        for (size_t j = 0; j <= deg_b; j++) {
            uint64_t term = (a[i] * b[j]) % prime;
            out_c[i + j] = (out_c[i + j] + term) % prime;
        }
    }
    size_t final_deg = 0;
    for (size_t i = 0; i <= max_out_deg; i++) {
        if (out_c[i] > 0) {
            final_deg = i;
        }
    }
    *out_deg = final_deg;
    return 0;
}

int interop_poly_euclidean_verify(const uint64_t *a, size_t deg_a, const uint64_t *b, size_t deg_b, const uint64_t *q, size_t deg_q, const uint64_t *r, size_t deg_r, uint64_t prime, int *out_verified) {
    if (!a || !b || !q || !r || !out_verified || prime == 0) return -1;
    size_t qb_max_deg = deg_q + deg_b;
    uint64_t *qb = calloc(qb_max_deg + 1, sizeof(uint64_t));
    if (!qb) return -1;
    size_t qb_deg = 0;
    if (interop_poly_multiply_mod(q, b, deg_q, deg_b, prime, qb, &qb_deg) != 0) {
        free(qb);
        return -1;
    }
    size_t rhs_max_deg = (qb_deg > deg_r) ? qb_deg : deg_r;
    uint64_t *rhs = calloc(rhs_max_deg + 1, sizeof(uint64_t));
    if (!rhs) {
        free(qb);
        return -1;
    }
    size_t rhs_deg = 0;
    if (interop_poly_add_sub_mod(qb, r, qb_deg, deg_r, prime, 0, rhs, &rhs_deg) != 0) {
        free(qb);
        free(rhs);
        return -1;
    }
    int verified = 1;
    size_t max_compare_deg = (deg_a > rhs_deg) ? deg_a : rhs_deg;
    for (size_t i = 0; i <= max_compare_deg; i++) {
        uint64_t val_a = (i <= deg_a) ? a[i] : 0;
        uint64_t val_rhs = (i <= rhs_deg) ? rhs[i] : 0;
        if (val_a != val_rhs) {
            verified = 0;
            break;
        }
    }
    *out_verified = verified;
    free(qb);
    free(rhs);
    return 0;
}

int interop_logic_infer_rule(const int *edges_src, const int *edges_rel, const int *edges_dst, size_t num_edges, int r1, int r2, int r3, int *out_new_src, int *out_new_dst, size_t max_new, size_t *out_count) {
    if (!edges_src || !edges_rel || !edges_dst || num_edges == 0 || !out_new_src || !out_new_dst || max_new == 0 || !out_count) return -1;
    size_t count = 0;
    for (size_t i = 0; i < num_edges; i++) {
        if (edges_rel[i] != r1) continue;
        int u = edges_src[i];
        int v = edges_dst[i];
        for (size_t j = 0; j < num_edges; j++) {
            if (edges_rel[j] != r2 || edges_src[j] != v) continue;
            int w = edges_dst[j];
            int already_exists = 0;
            for (size_t k = 0; k < num_edges; k++) {
                if (edges_src[k] == u && edges_rel[k] == r3 && edges_dst[k] == w) {
                    already_exists = 1;
                    break;
                }
            }
            if (!already_exists) {
                int in_out = 0;
                for (size_t k = 0; k < count; k++) {
                    if (out_new_src[k] == u && out_new_dst[k] == w) {
                        in_out = 1;
                        break;
                    }
                }
                if (!in_out && count < max_new) {
                    out_new_src[count] = u;
                    out_new_dst[count] = w;
                    count++;
                }
            }
        }
    }
    *out_count = count;
    return 0;
}

int interop_logic_deductive_closure(int *edges_src, int *edges_rel, int *edges_dst, size_t *num_edges, size_t max_edges, const int *r1_rules, const int *r2_rules, const int *r3_rules, size_t num_rules) {
    if (!edges_src || !edges_rel || !edges_dst || !num_edges || max_edges == 0 || !r1_rules || !r2_rules || !r3_rules || num_rules == 0) return -1;
    int added_any = 1;
    while (added_any) {
        added_any = 0;
        size_t current_num = *num_edges;
        for (size_t r = 0; r < num_rules; r++) {
            int r1 = r1_rules[r];
            int r2 = r2_rules[r];
            int r3 = r3_rules[r];
            int temp_src[100];
            int temp_dst[100];
            size_t temp_cnt = 0;
            if (interop_logic_infer_rule(edges_src, edges_rel, edges_dst, current_num, r1, r2, r3, temp_src, temp_dst, 100, &temp_cnt) == 0) {
                for (size_t i = 0; i < temp_cnt; i++) {
                    if (*num_edges < max_edges) {
                        edges_src[*num_edges] = temp_src[i];
                        edges_rel[*num_edges] = r3;
                        edges_dst[*num_edges] = temp_dst[i];
                        (*num_edges)++;
                        added_any = 1;
                    }
                }
            }
        }
    }
    return 0;
}

int interop_logic_check_consistency(const int *edges_src, const int *edges_rel, const int *edges_dst, size_t num_edges, const int *asymmetric_rels, size_t num_asym, int *out_consistent) {
    if (!edges_src || !edges_rel || !edges_dst || num_edges == 0 || !asymmetric_rels || num_asym == 0 || !out_consistent) return -1;
    *out_consistent = 1;
    for (size_t i = 0; i < num_edges; i++) {
        int r = edges_rel[i];
        int u = edges_src[i];
        int v = edges_dst[i];
        int is_asym = 0;
        for (size_t idx = 0; idx < num_asym; idx++) {
            if (asymmetric_rels[idx] == r) {
                is_asym = 1;
                break;
            }
        }
        if (is_asym) {
            if (u == v) {
                *out_consistent = 0;
                return 0;
            }
            for (size_t j = 0; j < num_edges; j++) {
                if (edges_rel[j] == r && edges_src[j] == v && edges_dst[j] == u) {
                    *out_consistent = 0;
                    return 0;
                }
            }
        }
    }
    return 0;
}

int interop_poly_relation_compose_verify(const float *r1, const float *r2, const float *r3, size_t dim, float threshold, int *out_verified) {
    if (!r1 || !r2 || !r3 || !out_verified || dim == 0) return -1;
    float sum_sq = 0.0f;
    for (size_t i = 0; i < dim; i++) {
        float diff = r1[i] + r2[i] - r3[i];
        sum_sq += diff * diff;
    }
    float dist = sqrtf(sum_sq);
    *out_verified = (dist < threshold) ? 1 : 0;
    return 0;
}

int interop_poly_relation_inverse_verify(const float *r1, const float *r2, size_t dim, float threshold, int *out_verified) {
    if (!r1 || !r2 || !out_verified || dim == 0) return -1;
    float sum_sq = 0.0f;
    for (size_t i = 0; i < dim; i++) {
        float diff = r1[i] + r2[i];
        sum_sq += diff * diff;
    }
    float dist = sqrtf(sum_sq);
    *out_verified = (dist < threshold) ? 1 : 0;
    return 0;
}

int interop_logic_forall_verify(const int *edges_src, const int *edges_rel, const int *edges_dst, size_t num_edges, int s, int p, const int *entity_ids, const int *entity_types, size_t num_entities, int expected_type, int *out_satisfied) {
    if (!edges_src || !edges_rel || !edges_dst || num_edges == 0 || !entity_ids || !entity_types || num_entities == 0 || !out_satisfied) return -1;
    *out_satisfied = 1;
    int found_any = 0;
    for (size_t i = 0; i < num_edges; i++) {
        if (edges_src[i] == s && edges_rel[i] == p) {
            found_any = 1;
            int dst = edges_dst[i];
            int type_match = 0;
            for (size_t idx = 0; idx < num_entities; idx++) {
                if (entity_ids[idx] == dst) {
                    if (entity_types[idx] == expected_type) {
                        type_match = 1;
                    }
                    break;
                }
            }
            if (!type_match) {
                *out_satisfied = 0;
                return 0;
            }
        }
    }
    if (!found_any) {
        *out_satisfied = 1;
    }
    return 0;
}

int interop_logic_exists_verify(const int *edges_src, const int *edges_rel, const int *edges_dst, size_t num_edges, int s, int p, const int *entity_ids, const int *entity_types, size_t num_entities, int expected_type, int *out_satisfied) {
    if (!edges_src || !edges_rel || !edges_dst || num_edges == 0 || !entity_ids || !entity_types || num_entities == 0 || !out_satisfied) return -1;
    *out_satisfied = 0;
    for (size_t i = 0; i < num_edges; i++) {
        if (edges_src[i] == s && edges_rel[i] == p) {
            int dst = edges_dst[i];
            for (size_t idx = 0; idx < num_entities; idx++) {
                if (entity_ids[idx] == dst) {
                    if (entity_types[idx] == expected_type) {
                        *out_satisfied = 1;
                        return 0;
                    }
                    break;
                }
            }
        }
    }
    return 0;
}

int interop_logic_not_verify(const int *edges_src, const int *edges_rel, const int *edges_dst, size_t num_edges, int s, int p, int o, int *out_satisfied) {
    if (!edges_src || !edges_rel || !edges_dst || num_edges == 0 || !out_satisfied) return -1;
    int exists = 0;
    for (size_t i = 0; i < num_edges; i++) {
        if (edges_src[i] == s && edges_rel[i] == p && edges_dst[i] == o) {
            exists = 1;
            break;
        }
    }
    *out_satisfied = exists ? 0 : 1;
    return 0;
}

int interop_trace_log_action(int subject_id, int relation_id, int object_id, uint64_t timestamp, int *log_src, int *log_rel, int *log_dst, uint64_t *log_ts, size_t *log_count, size_t max_log) {
    if (!log_src || !log_rel || !log_dst || !log_ts || !log_count || max_log == 0) return -1;
    size_t count = *log_count;
    if (count >= max_log) return -2;
    log_src[count] = subject_id;
    log_rel[count] = relation_id;
    log_dst[count] = object_id;
    log_ts[count] = timestamp;
    *log_count = count + 1;
    return 0;
}

int interop_trace_query_history(const int *log_src, const int *log_rel, const int *log_dst, size_t log_count, int s, int r, int o, int *out_src, int *out_rel, int *out_dst, size_t max_results, size_t *out_count) {
    if (!log_src || !log_rel || !log_dst || log_count == 0 || !out_src || !out_rel || !out_dst || max_results == 0 || !out_count) return -1;
    size_t count = 0;
    for (size_t i = 0; i < log_count; i++) {
        if (s != -1 && log_src[i] != s) continue;
        if (r != -1 && log_rel[i] != r) continue;
        if (o != -1 && log_dst[i] != o) continue;
        if (count < max_results) {
            out_src[count] = log_src[i];
            out_rel[count] = log_rel[i];
            out_dst[count] = log_dst[i];
            count++;
        }
    }
    *out_count = count;
    return 0;
}

int interop_trace_verify_rule(const int *log_src, const int *log_rel, const int *log_dst, size_t log_count, int trigger_rel, int target_rel, int *out_satisfied) {
    if (!log_src || !log_rel || !log_dst || log_count == 0 || !out_satisfied) return -1;
    *out_satisfied = 1;
    for (size_t i = 0; i < log_count; i++) {
        if (log_rel[i] == trigger_rel) {
            int y = log_dst[i];
            int found = 0;
            for (size_t j = 0; j < log_count; j++) {
                if (log_src[j] == y && log_rel[j] == target_rel) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                *out_satisfied = 0;
                return 0;
            }
        }
    }
    return 0;
}

int interop_wm_transition_verify(const float *h, const float *r, const float *t, size_t dim, float sem_threshold, const int *edges_src, const int *edges_rel, const int *edges_dst, size_t num_edges, const int *asymmetric_rels, size_t num_asym, int subject_id, int relation_id, int object_id, uint64_t timestamp, int *log_src, int *log_rel, int *log_dst, uint64_t *log_ts, size_t *log_count, size_t max_log, int *out_verified) {
    if (!h || !r || !t || dim == 0 || !out_verified) return -1;
    *out_verified = 0;
    float sum_sq = 0.0f;
    for (size_t i = 0; i < dim; i++) {
        float diff = h[i] + r[i] - t[i];
        sum_sq += diff * diff;
    }
    float dist = sqrtf(sum_sq);
    if (dist >= sem_threshold) {
        return 0;
    }
    if (edges_src && edges_rel && edges_dst && num_edges > 0 && asymmetric_rels && num_asym > 0) {
        int logic_consistent = -1;
        if (interop_logic_check_consistency(edges_src, edges_rel, edges_dst, num_edges, asymmetric_rels, num_asym, &logic_consistent) != 0 || logic_consistent == 0) {
            return 0;
        }
    }
    if (log_src && log_rel && log_dst && log_ts && log_count && max_log > 0) {
        if (interop_trace_log_action(subject_id, relation_id, object_id, timestamp, log_src, log_rel, log_dst, log_ts, log_count, max_log) != 0) {
            return 0;
        }
    }
    *out_verified = 1;
    return 0;
}
