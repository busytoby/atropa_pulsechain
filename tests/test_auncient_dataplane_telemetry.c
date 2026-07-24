#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define LOCK_THRESHOLD 0.05

typedef struct {
    uint64_t timestamp_counter;
    uint64_t sequence_number;
    double phase_error;
    uint8_t resolved_action; // 0 = GATED/BLOCKED, 1 = RESUMED
} pll_blame_record_t;

typedef struct {
    double ref_phase;
    double local_phase;
    double phase_error;
    bool locked;
} telemetry_pll_t;

// Writes compliance blame records directly to .dat.bin ledger (Rule 13 compliance)
static bool log_telemetry_blame(const char *path, const pll_blame_record_t *rec) {
    FILE *f = fopen(path, "ab"); // Append mode binary
    if (!f) return false;
    
    size_t written = fwrite(rec, sizeof(pll_blame_record_t), 1, f);
    fclose(f);
    return (written == 1);
}

// Simulates packet routing over the PLL compliance stack
static bool process_packet_under_telemetry(telemetry_pll_t *pll, uint64_t seq, uint64_t tsc, const char *blame_path) {
    // Phase error tracking
    pll->phase_error = pll->ref_phase - pll->local_phase;
    pll->locked = (fabs(pll->phase_error) < LOCK_THRESHOLD);

    if (!pll->locked) {
        // Generate and log compliance blame event
        pll_blame_record_t record;
        record.timestamp_counter = tsc;
        record.sequence_number = seq;
        record.phase_error = pll->phase_error;
        record.resolved_action = 0; // GATED/BLOCKED due to drift

        log_telemetry_blame(blame_path, &record);
        return false; // Intercepted and blocked
    }
    return true; // Transmitted
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT PLL COMPLIANCE DATAPLANE TELEMETRY SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    const char *blame_path = "tests/drift_blame.dat.bin";
    // Remove previous logs if any
    remove(blame_path);

    telemetry_pll_t pll;
    pll.ref_phase = 1.0;
    pll.local_phase = 1.0;
    pll.phase_error = 0.0;
    pll.locked = true;

    // 1. Send packet under phase lock -> should succeed with no logs
    printf("[TEST] Routing packet under locked phase state...\n");
    fflush(stdout);
    bool ok = process_packet_under_telemetry(&pll, 101, 1000, blame_path);
    assert(ok == true);

    // Verify no blame file created yet
    FILE *chk = fopen(blame_path, "rb");
    assert(chk == NULL);
    printf("   ✓ Packet routed. No telemetry blame generated.\n");
    fflush(stdout);

    // 2. Inject clock phase drift -> should fail and generate blame log
    printf("[TEST] Injecting PLL phase drift and routing packet...\n");
    fflush(stdout);
    pll.local_phase = 1.08; // Exceeds LOCK_THRESHOLD of 0.05
    
    ok = process_packet_under_telemetry(&pll, 102, 1050, blame_path);
    assert(ok == false); // Should block packet
    printf("   ✓ Transmission blocked successfully.\n");
    fflush(stdout);

    // 3. Verify .dat.bin compliance blame ledger structure
    printf("[TEST] Verifying .dat.bin blame ledger contents...\n");
    fflush(stdout);

    FILE *ledger = fopen(blame_path, "rb");
    assert(ledger != NULL);
    
    pll_blame_record_t read_rec;
    size_t count = fread(&read_rec, sizeof(pll_blame_record_t), 1, ledger);
    fclose(ledger);

    assert(count == 1);
    assert(read_rec.sequence_number == 102);
    assert(read_rec.timestamp_counter == 1050);
    assert(fabs(read_rec.phase_error - (-0.08)) < 0.0001);
    assert(read_rec.resolved_action == 0);

    printf("   ✓ Compliance blame log parsed and validated successfully from disk.\n");
    fflush(stdout);

    remove(blame_path);

    printf("=============================================================\n");
    printf("PLL COMPLIANCE TELEMETRY TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
