#include "tsfi_graph_cards.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <vector>

// Note: Using C++ for vector/sort to handle tree hashing efficiently
// but maintaining the C-linkage for the thunk system.

extern "C" {

typedef struct {
    std::vector<uint32_t> adj[MAX_GRAPH_NODES];
    uint32_t degree[MAX_GRAPH_NODES];
    bool on_cycle[MAX_GRAPH_NODES];
    uint64_t tree_hash[MAX_GRAPH_NODES];
} GraphInternal;

static uint64_t splitmix64(uint64_t x) {
    x += 0x9e3779b97f4a7c15;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
    x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
    return x ^ (x >> 31);
}

static uint64_t hash_tree(uint32_t u, uint32_t p, GraphInternal *gi) {
    std::vector<uint64_t> child_hashes;
    for (uint32_t v : gi->adj[u]) {
        if (v != p && !gi->on_cycle[v]) {
            child_hashes.push_back(hash_tree(v, u, gi));
        }
    }
    std::sort(child_hashes.begin(), child_hashes.end());
    uint64_t h = 0x1337BEEF;
    for (uint64_t ch : child_hashes) {
        h ^= splitmix64(ch);
    }
    return h;
}

void tsfi_thunk_solve_graph_isomorphism(TsfiGraphCard *card) {
    if (!card || card->node_count == 0) return;

    GraphInternal gi;
    memset(gi.degree, 0, sizeof(gi.degree));
    for (uint32_t i = 0; i < card->node_count; i++) gi.on_cycle[i] = true;

    for (uint32_t i = 0; i < card->edge_count; i++) {
        uint32_t u = card->edges[i].u - 1;
        uint32_t v = card->edges[i].v - 1;
        printf("[DEBUG] Edge %u: %u-%u\n", i, u+1, v+1);
        gi.adj[u].push_back(v);
        gi.adj[v].push_back(u);
        gi.degree[u]++;
        gi.degree[v]++;
    }

    // 1. Prune leaves to find cycle
    std::vector<uint32_t> q;
    for (uint32_t i = 0; i < card->node_count; i++) {
        if (gi.degree[i] == 1) q.push_back(i);
    }
    printf("[DEBUG] Initial leaves: %zu\n", q.size());

    size_t head = 0;
    while(head < q.size()) {
        uint32_t u = q[head++];
        gi.on_cycle[u] = false;
        for (uint32_t v : gi.adj[u]) {
            if (gi.degree[v] > 1 && --gi.degree[v] == 1) q.push_back(v);
        }
    }

    // 2. Hash trees rooted on cycle
    uint32_t start_node = 0xFFFFFFFF;
    uint32_t cycle_count = 0;
    for (uint32_t i = 0; i < card->node_count; i++) {
        if (gi.on_cycle[i]) {
            gi.tree_hash[i] = hash_tree(i, 0xFFFFFFFF, &gi);
            if (start_node == 0xFFFFFFFF) start_node = i;
            cycle_count++;
        }
    }
    printf("[DEBUG] Nodes on cycle: %u, Start: %u\n", cycle_count, start_node+1);

    // 3. Extract cycle sequence
    uint32_t curr = start_node;
    uint32_t prev = 0xFFFFFFFF;
    std::vector<uint64_t> seq;
    do {
        seq.push_back(gi.tree_hash[curr]);
        uint32_t next = 0xFFFFFFFF;
        for (uint32_t v : gi.adj[curr]) {
            if (gi.on_cycle[v] && v != prev) {
                if (v == start_node && seq.size() > 1) continue;
                next = v;
                break;
            }
        }
        prev = curr;
        curr = next;
    } while (curr != 0xFFFFFFFF && curr != start_node);

    // 4. Canonicalize sequence (Min rotation and its reverse)
    auto get_min_rot = [](std::vector<uint64_t> s) {
        size_t n = s.size();
        std::vector<uint64_t> res = s;
        for (size_t i = 0; i < n; i++) {
            std::vector<uint64_t> rot;
            for (size_t j = 0; j < n; j++) rot.push_back(s[(i + j) % n]);
            if (rot < res) res = rot;
        }
        return res;
    };

    std::vector<uint64_t> min_fwd = get_min_rot(seq);
    std::reverse(seq.begin(), seq.end());
    std::vector<uint64_t> min_rev = get_min_rot(seq);
    std::vector<uint64_t> canonical = (min_fwd < min_rev) ? min_fwd : min_rev;

    // 5. Populate Atom (The Subjective Identity)
    memset(&card->canonical_atom, 0, sizeof(TsfiTasteAtom));
    card->canonical_atom.secrets[0] = (float)canonical.size(); // Cycle Length
    for (size_t i = 0; i < canonical.size() && i < 15; i++) {
        card->canonical_atom.secrets[i+1] = (float)(canonical[i] % 1000000) / 1000.0f;
    }
}

} // extern "C"
