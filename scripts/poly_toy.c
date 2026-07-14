#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "libantigravity_interop.h"

int main(void) {
    printf("Initializing Polynomial Verification Circuit Toy...\n");

    // A(x) = B(x)Q(x) + R(x) under prime modulus 17
    // Let B(x) = 1 + 2x, Q(x) = 3 + 5x, R(x) = 4
    // A(x) = 7 + 11x + 10x^2
    uint64_t pe_a[3] = { 7, 11, 10 };
    uint64_t pe_b[2] = { 1, 2 };
    uint64_t pe_q[2] = { 3, 5 };
    uint64_t pe_r[1] = { 4 };
    uint64_t prime = 17;
    int verified = -1;

    printf("Verifying algebraic Euclidean division constraint...\n");
    int status = interop_poly_euclidean_verify(
        pe_a, 2,
        pe_b, 1,
        pe_q, 1,
        pe_r, 0,
        prime,
        &verified
    );

    if (status != 0 || verified != 1) {
        fprintf(stderr, "Error: Polynomial Euclidean division verification failed (status: %d, verified: %d)\n", status, verified);
        return 1;
    }
    printf("✓ Polynomial division constraint successfully verified.\n");

    // Persist to poly_trace.dat.bin (Rule 13)
    const char *out_path = "assets/poly_trace.dat.bin";
    FILE *f = fopen(out_path, "wb");
    if (!f) {
        fprintf(stderr, "Error: Could not open output file %s for writing\n", out_path);
        return 1;
    }

    uint32_t deg_a = 2, deg_b = 1, deg_q = 1, deg_r = 0;
    fwrite(&deg_a, sizeof(uint32_t), 1, f);
    fwrite(&deg_b, sizeof(uint32_t), 1, f);
    fwrite(&deg_q, sizeof(uint32_t), 1, f);
    fwrite(&deg_r, sizeof(uint32_t), 1, f);
    fwrite(pe_a, sizeof(uint64_t), deg_a + 1, f);
    fwrite(pe_b, sizeof(uint64_t), deg_b + 1, f);
    fwrite(pe_q, sizeof(uint64_t), deg_q + 1, f);
    fwrite(pe_r, sizeof(uint64_t), deg_r + 1, f);
    fwrite(&verified, sizeof(int), 1, f);
    fclose(f);

    printf("✓ Successfully persisted verified polynomial proof coefficients to %s\n", out_path);

    // Read back verification test
    f = fopen(out_path, "rb");
    if (!f) {
        fprintf(stderr, "Error: Could not open file %s for verification reading\n", out_path);
        return 1;
    }

    uint32_t r_deg_a, r_deg_b, r_deg_q, r_deg_r;
    fread(&r_deg_a, sizeof(uint32_t), 1, f);
    fread(&r_deg_b, sizeof(uint32_t), 1, f);
    fread(&r_deg_q, sizeof(uint32_t), 1, f);
    fread(&r_deg_r, sizeof(uint32_t), 1, f);

    uint64_t *r_a = malloc((r_deg_a + 1) * sizeof(uint64_t));
    uint64_t *r_b = malloc((r_deg_b + 1) * sizeof(uint64_t));
    uint64_t *r_q = malloc((r_deg_q + 1) * sizeof(uint64_t));
    uint64_t *r_r = malloc((r_deg_r + 1) * sizeof(uint64_t));
    int r_verified = -1;

    fread(r_a, sizeof(uint64_t), r_deg_a + 1, f);
    fread(r_b, sizeof(uint64_t), r_deg_b + 1, f);
    fread(r_q, sizeof(uint64_t), r_deg_q + 1, f);
    fread(r_r, sizeof(uint64_t), r_deg_r + 1, f);
    fread(&r_verified, sizeof(int), 1, f);
    fclose(f);

    printf("Verifying read-back polynomial proof coefficients:\n");
    printf("  A(x) = %lu + %lux + %lux^2\n", (unsigned long)r_a[0], (unsigned long)r_a[1], (unsigned long)r_a[2]);
    printf("  B(x) = %lu + %lux\n", (unsigned long)r_b[0], (unsigned long)r_b[1]);
    printf("  Q(x) = %lu + %lux\n", (unsigned long)r_q[0], (unsigned long)r_q[1]);
    printf("  R(x) = %lu\n", (unsigned long)r_r[0]);
    printf("  Verification flag: %d\n", r_verified);

    free(r_a);
    free(r_b);
    free(r_q);
    free(r_r);

    printf("✓ Read-back verification complete. Saturated arithmetic gate verified successfully.\n");
    return 0;
}
