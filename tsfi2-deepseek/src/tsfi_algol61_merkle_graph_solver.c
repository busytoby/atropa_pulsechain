#define _POSIX_C_SOURCE 200809L
#include "tsfi_algol61_merkle_graph_solver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>

static uint32_t fnv1a_32(const char *str, size_t len) {
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < len; i++) {
        hash ^= (uint8_t)str[i];
        hash *= 16777619u;
    }
    return hash;
}

static TsfiProverDomain classify_domain(const char *filename) {
    if (strstr(filename, "acoustic") || strstr(filename, "edo22") || strstr(filename, "binaural") || strstr(filename, "audio") || strstr(filename, "sound")) {
        return TSFI_SOLVER_DOMAIN_HARMONIC_EDO22;
    }
    if (strstr(filename, "entropy") || strstr(filename, "motzkin") || strstr(filename, "treasury") || strstr(filename, "token") || strstr(filename, "sha") || strstr(filename, "random")) {
        return TSFI_SOLVER_DOMAIN_CRYPTO_ENTROPY;
    }
    if (strstr(filename, "initial_orders") || strstr(filename, "edsac") || strstr(filename, "opcode") || strstr(filename, "compiler") || strstr(filename, "firewall") || strstr(filename, "loader")) {
        return TSFI_SOLVER_DOMAIN_INSTRUCTION_HAL;
    }
    if (strstr(filename, "hogan") || strstr(filename, "ssa") || strstr(filename, "dna") || strstr(filename, "teddy") || strstr(filename, "endowment") || strstr(filename, "citizen")) {
        return TSFI_SOLVER_DOMAIN_SOVEREIGN_ID;
    }
    return TSFI_SOLVER_DOMAIN_PHYSICS_FET;
}

bool tsfi_algol61_graph_init(TsfiAlgol61KnowledgeGraph *graph) {
    if (!graph) return false;
    memset(graph, 0, sizeof(TsfiAlgol61KnowledgeGraph));
    return true;
}

bool tsfi_algol61_graph_ingest_prover(TsfiAlgol61KnowledgeGraph *graph, const char *filepath) {
    if (!graph || !filepath) return false;
    if (graph->node_count >= TSFI_MAX_PROVERS_CAPACITY) return false;

    FILE *f = fopen(filepath, "rb");
    if (!f) return false;

    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (sz <= 0 || sz > 1000000) {
        fclose(f);
        return false;
    }

    char *buf = (char*)malloc((size_t)sz + 1);
    if (!buf) {
        fclose(f);
        return false;
    }
    size_t read_bytes = fread(buf, 1, (size_t)sz, f);
    buf[read_bytes] = '\0';
    fclose(f);

    const char *basename = strrchr(filepath, '/');
    basename = (basename) ? basename + 1 : filepath;

    TsfiAlgol61Node *node = &graph->nodes[graph->node_count];
    strncpy(node->name, basename, sizeof(node->name) - 1);
    node->domain = classify_domain(basename);

    uint32_t ast_hash = fnv1a_32(buf, read_bytes);
    node->ast_hash = ast_hash;

    // Map to canonical 64-hexagram coordinate register
    node->yi_hexagram_index = (uint8_t)(ast_hash % TSFI_YI_HEXAGRAM_COUNT);

    // Count simple syntax tokens for conditions
    uint32_t pre = 0;
    uint32_t post = 0;
    const char *p = buf;
    while ((p = strstr(p, "if ")) != NULL) { pre++; p += 3; }
    p = buf;
    while ((p = strstr(p, ":=")) != NULL) { post++; p += 2; }

    node->precondition_cnt = pre;
    node->postcondition_cnt = post;

    graph->hexagram_buckets[node->yi_hexagram_index]++;
    graph->node_count++;

    free(buf);
    return true;
}

bool tsfi_algol61_graph_compute_merkle_root(TsfiAlgol61KnowledgeGraph *graph) {
    if (!graph || graph->node_count == 0) return false;

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) return false;

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) != 1) {
        EVP_MD_CTX_free(ctx);
        return false;
    }

    for (uint32_t i = 0; i < graph->node_count; i++) {
        EVP_DigestUpdate(ctx, graph->nodes[i].name, strlen(graph->nodes[i].name));
        EVP_DigestUpdate(ctx, &graph->nodes[i].domain, sizeof(graph->nodes[i].domain));
        EVP_DigestUpdate(ctx, &graph->nodes[i].yi_hexagram_index, sizeof(graph->nodes[i].yi_hexagram_index));
        EVP_DigestUpdate(ctx, &graph->nodes[i].ast_hash, sizeof(graph->nodes[i].ast_hash));
    }

    unsigned int out_len = 0;
    int ok = EVP_DigestFinal_ex(ctx, graph->merkle_root, &out_len);
    EVP_MD_CTX_free(ctx);

    return (ok == 1 && out_len == 32);
}

uint32_t tsfi_algol61_graph_query_hexagram(const TsfiAlgol61KnowledgeGraph *graph, uint8_t hexagram_idx, TsfiAlgol61Node *out_nodes, uint32_t max_out) {
    if (!graph || hexagram_idx >= TSFI_YI_HEXAGRAM_COUNT || !out_nodes || max_out == 0) return 0;

    uint32_t found = 0;
    for (uint32_t i = 0; i < graph->node_count && found < max_out; i++) {
        if (graph->nodes[i].yi_hexagram_index == hexagram_idx) {
            out_nodes[found++] = graph->nodes[i];
        }
    }
    return found;
}
