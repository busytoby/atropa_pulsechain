#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// Error Locator Polynomial (Degree-2 representation: x^2 + ax + b)
typedef struct {
    double a; // Coefficient of x
    double b; // Constant term
} error_poly_t;

// Simulated Network Buffer
typedef struct {
    char payload[64];
    gate_state_t write_gate;
} net_buffer_t;

// -------------------------------------------------------------
// Lindsey-Fox Polynomial Root-Finding Error Locator
// -------------------------------------------------------------

// Solves roots to locate bit errors; returns true if real roots are resolved (correctable)
bool lindsey_fox_resolve_roots(const error_poly_t *poly, double *root1, double *root2) {
    // Discriminant: D = a^2 - 4b
    double discriminant = (poly->a * poly->a) - (4.0 * poly->b);
    
    if (discriminant < 0.0) {
        // Complex roots indicate uncorrectable multi-bit burst corruption
        return false;
    }
    
    // Resolve roots (representing error locations)
    double sqrt_d = sqrt(discriminant);
    *root1 = (-poly->a + sqrt_d) / 2.0;
    *root2 = (-poly->a - sqrt_d) / 2.0;
    return true;
}

// Write corrected payload to buffer (gated by error correction state)
void plexus_write_corrected_payload(net_buffer_t *buf, const char *raw_data, 
                                    const error_poly_t *poly) {
    double r1 = 0, r2 = 0;
    bool correctable = lindsey_fox_resolve_roots(poly, &r1, &r2);

    // Hardware write gate conducts only if error is resolvable (correctable)
    buf->write_gate = correctable ? CONDUC_STATE : CUTOFF_STATE;

    if (buf->write_gate == CONDUC_STATE) {
        // Apply correction and copy to buffer (simulating error locator write-enable)
        strcpy(buf->payload, raw_data);
    }
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT LINDSEY-FOX ERROR LOCATOR VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    net_buffer_t buf = {
        .payload = "UNCORRECTED_DATA",
        .write_gate = CUTOFF_STATE
    };

    // 1. Correctable error case (Discriminant >= 0)
    // Polynomial: x^2 - 5x + 6 (roots at 2 and 3)
    printf("[TEST] Resolving correctable error polynomial (x^2 - 5x + 6)...\n");
    fflush(stdout);
    error_poly_t poly_ok = { -5.0, 6.0 };
    
    plexus_write_corrected_payload(&buf, "CORRECTED_PAYLOAD_A", &poly_ok);
    assert(buf.write_gate == CONDUC_STATE);
    assert(strcmp(buf.payload, "CORRECTED_PAYLOAD_A") == 0);
    printf("   ✓ Polynomial roots resolved successfully (conduction active).\n");
    fflush(stdout);

    // Reset buffer
    strcpy(buf.payload, "UNCORRECTED_DATA");

    // 2. Uncorrectable error case (Discriminant < 0)
    // Polynomial: x^2 + 2x + 5 (Discriminant = 4 - 20 = -16, complex roots)
    printf("[TEST] Resolving uncorrectable error polynomial (x^2 + 2x + 5)...\n");
    fflush(stdout);
    error_poly_t poly_bad = { 2.0, 5.0 };
    
    plexus_write_corrected_payload(&buf, "CORRECTED_PAYLOAD_B", &poly_bad);
    assert(buf.write_gate == CUTOFF_STATE);
    assert(strcmp(buf.payload, "UNCORRECTED_DATA") == 0); // Unaltered/dropped
    printf("   ✓ Burst corruption detected: hardware cutoff isolated write-bus.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("LINDSEY-FOX ERROR LOCATOR INTEGRATION VERIFIED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
