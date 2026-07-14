#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "libantigravity_interop.h"

#define MAX_LOGIC_EDGES 100

typedef struct {
    int src;
    int rel;
    int dst;
} EdgeRecord;

int main(void) {
    printf("Initializing Logical Deduction & Rule Closure Toy...\n");

    // Initialize base relation edges
    int edges_src[MAX_LOGIC_EDGES] = { 1, 2 };
    int edges_rel[MAX_LOGIC_EDGES] = { 10, 20 };
    int edges_dst[MAX_LOGIC_EDGES] = { 2, 3 };
    size_t num_edges = 2;

    // Define rules: 10 + 20 -> 30
    int r1_rules[1] = { 10 };
    int r2_rules[1] = { 20 };
    int r3_rules[1] = { 30 };

    printf("Executing logical deductive closure saturation...\n");
    int status = interop_logic_deductive_closure(
        edges_src, edges_rel, edges_dst, &num_edges, MAX_LOGIC_EDGES,
        r1_rules, r2_rules, r3_rules, 1
    );

    if (status != 0) {
        fprintf(stderr, "Error: Deductive closure saturation failed (status: %d)\n", status);
        return 1;
    }
    printf("✓ Saturated graph contains %zu edges.\n", num_edges);

    // Persist to logic_trace.dat.bin (Rule 13)
    const char *out_path = "../assets/logic_trace.dat.bin";
    FILE *f = fopen(out_path, "wb");
    if (!f) {
        fprintf(stderr, "Error: Could not open output file %s for writing\n", out_path);
        return 1;
    }

    uint32_t count_val = (uint32_t)num_edges;
    fwrite(&count_val, sizeof(uint32_t), 1, f);

    for (size_t i = 0; i < num_edges; i++) {
        EdgeRecord rec;
        rec.src = edges_src[i];
        rec.rel = edges_rel[i];
        rec.dst = edges_dst[i];
        fwrite(&rec, sizeof(EdgeRecord), 1, f);
    }
    fclose(f);

    printf("✓ Successfully persisted %u saturated edges to %s\n", count_val, out_path);

    // Read back verification test
    f = fopen(out_path, "rb");
    if (!f) {
        fprintf(stderr, "Error: Could not open file %s for verification reading\n", out_path);
        return 1;
    }

    uint32_t read_count = 0;
    if (fread(&read_count, sizeof(uint32_t), 1, f) != 1 || read_count != count_val) {
        fprintf(stderr, "Error: Verification read count mismatch\n");
        fclose(f);
        return 1;
    }

    printf("Verifying read-back logical deductive closure edges:\n");
    for (uint32_t i = 0; i < read_count; i++) {
        EdgeRecord rec;
        if (fread(&rec, sizeof(EdgeRecord), 1, f) != 1) {
            fprintf(stderr, "Error: Verification read of edge %u failed\n", i);
            fclose(f);
            return 1;
        }
        printf("  Edge[%u]: Entity %d -Relation %d-> Entity %d\n", i, rec.src, rec.rel, rec.dst);
    }
    fclose(f);

    printf("✓ Read-back verification complete. Saturated logic paths verified successfully.\n");
    return 0;
}
