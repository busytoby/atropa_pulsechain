#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "libantigravity_interop.h"

#define MAX_WM_LOGS 100

typedef struct {
    int subject_id;
    int relation_id;
    int object_id;
    uint64_t timestamp;
} WMTraceRecord;

int main(void) {
    printf("Initializing WinchesterMQ Working Memory State Evolution Toy...\n");

    // Initialize mock state vectors (dim = 2)
    float base_state[2] = { 1.0f, 0.0f };       // Initial Base (Seed phase)
    float form_relation[2] = { 0.1f, 0.2f };    // Form transition relation vector
    float next_state[2] = { 1.09f, 0.21f };     // Evolved Base state (dist ~ 0.014)

    // Logical constraint graphs
    int edges_src[2] = { 1, 2 };
    int edges_rel[2] = { 10, 10 };
    int edges_dst[2] = { 2, 3 };
    int asymmetric_rels[1] = { 10 };

    // Logging trace buffers
    int log_src[MAX_WM_LOGS] = {0};
    int log_rel[MAX_WM_LOGS] = {0};
    int log_dst[MAX_WM_LOGS] = {0};
    uint64_t log_ts[MAX_WM_LOGS] = {0};
    size_t log_count = 0;

    int verified = -1;

    // Run transition check
    printf("Evaluating transition: Seed state -> Form state...\n");
    int status = interop_wm_transition_verify(
        base_state, form_relation, next_state, 2, 0.05f,
        edges_src, edges_rel, edges_dst, 2,
        asymmetric_rels, 1,
        101, 10, 102, 123456789ULL,
        log_src, log_rel, log_dst, log_ts, &log_count, MAX_WM_LOGS,
        &verified
    );

    if (status != 0 || verified != 1) {
        fprintf(stderr, "Error: WinchesterMQ state transition verification failed (status: %d, verified: %d)\n", status, verified);
        return 1;
    }
    printf("✓ State transition successfully verified and logged.\n");

    // Persist verified traces to wm_trace.dat.bin (adhering to Rule 13)
    const char *out_path = "assets/wm_trace.dat.bin";
    FILE *f = fopen(out_path, "wb");
    if (!f) {
        fprintf(stderr, "Error: Could not open output file %s for writing\n", out_path);
        return 1;
    }

    // Write number of records first
    uint32_t count_val = (uint32_t)log_count;
    fwrite(&count_val, sizeof(uint32_t), 1, f);

    // Write records
    for (size_t i = 0; i < log_count; i++) {
        WMTraceRecord rec;
        rec.subject_id = log_src[i];
        rec.relation_id = log_rel[i];
        rec.object_id = log_dst[i];
        rec.timestamp = log_ts[i];
        fwrite(&rec, sizeof(WMTraceRecord), 1, f);
    }
    fclose(f);

    printf("✓ Successfully persisted %u verified records to %s\n", count_val, out_path);

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

    WMTraceRecord read_rec;
    if (fread(&read_rec, sizeof(WMTraceRecord), 1, f) != 1) {
        fprintf(stderr, "Error: Verification read of record failed\n");
        fclose(f);
        return 1;
    }
    fclose(f);

    printf("✓ Read-back verification complete: Rec[0] = (%d, %d, %d) at timestamp %lu\n",
           read_rec.subject_id, read_rec.relation_id, read_rec.object_id, (unsigned long)read_rec.timestamp);

    return 0;
}
