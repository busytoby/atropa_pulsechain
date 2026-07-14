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
