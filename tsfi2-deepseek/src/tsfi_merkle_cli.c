#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <immintrin.h>
#include "tsfi_helmholtz.h"
#include "tsfi_merkle.h"
#include "tsfi_math.h"
#include "tsfi_io.h"
#include "tsfi_c_math.h"

#define TSFI_MANIFOLD_SIZE_STATE (4 * 1024 * 1024)
#define TSFI_MANIFOLD_SIZE_FULL  (32 * 1024 * 1024)

/**
 * @brief ZMM-Accelerated Hex Printer (Internal Development)
 * Uses ZMM registers to prepare 512-bit chunks for output.
 */
void print_hex_zmm(const char *label, const uint8_t *data, size_t len) {
    printf("%s: ", label);
    if (len == 32) {
        __m256i v = _mm256_loadu_si256((const __m256i*)data);
        uint64_t limbs[4];
        _mm256_storeu_si256((__m256i*)limbs, v);
        for(int i=0; i<4; i++) printf("%016lx", limbs[i]);
    } else {
        for (size_t i = 0; i < len; i++) printf("%02x", data[i]);
    }
    printf("\n");
}

/**
 * @brief Internalized Fresnel C (Kirchhoff Reflection Logic)
 */
static float internal_fresnel_c(float u) {
    // Approximation for internal development rigidity
    float u2 = u * u;
    return u - (u2*u*u2)/10.0f; // Simplified Taylor
}

/**
 * @brief Internalized Fresnel S (Kirchhoff Reflection Logic)
 */
static float internal_fresnel_s(float u) {
    float u2 = u * u;
    return (u2*u)/3.0f - (u2*u2*u)/42.0f; // Simplified Taylor
}

static int handle_inspect(const char *path) {
    void *manifold = malloc(TSFI_MANIFOLD_SIZE_STATE);
    if (tsfi_restore_manifold(path, manifold, TSFI_MANIFOLD_SIZE_STATE) != 0) {
        fprintf(stderr, "Failed to load manifold: %s\n", path);
        free(manifold);
        return 1;
    }
    uint8_t root[32], receipt[32];
    tsfi_helmholtz_init();
    tsfi_helmholtz_reduce_11(root, receipt, NULL, NULL, manifold, 1, 2026, NULL);
    print_hex_zmm("State Root", root, 32);
    print_hex_zmm("Receipt Root", receipt, 32);
    free(manifold);
    return 0;
}

static int handle_inspect12(const char *path) {
    void *manifold = malloc(TSFI_MANIFOLD_SIZE_FULL);
    if (tsfi_restore_manifold(path, manifold, TSFI_MANIFOLD_SIZE_FULL) != 0) {
        fprintf(stderr, "Failed to load Level 12 manifold: %s\n", path);
        free(manifold);
        return 1;
    }
    uint8_t root[32], receipt[32], sheaf[32];
    tsfi_helmholtz_init();
    tsfi_helmholtz_reduce_12(root, receipt, sheaf, NULL, NULL, manifold, 1, 2026, NULL);
    print_hex_zmm("State Root", root, 32);
    print_hex_zmm("Receipt Root", receipt, 32);
    print_hex_zmm("Sheaf Root", sheaf, 32);
    free(manifold);
    return 0;
}

static int handle_post_lore(const char *path, int offset, const char *text) {
    void *manifold = calloc(1, TSFI_MANIFOLD_SIZE_FULL);
    tsfi_restore_manifold(path, manifold, TSFI_MANIFOLD_SIZE_FULL);
    tsfi_helmholtz_init();
    tsfi_helmholtz_tokenize_lore(manifold, text, offset);
    if (tsfi_persist_manifold(path, manifold, TSFI_MANIFOLD_SIZE_FULL) == 0) {
        printf("[PASS] Lore Posted and Manifold Solidified: %s\n", path);
    } else {
        printf("[FAIL] Failed to persist manifold.\n");
    }
    free(manifold);
    return 0;
}

static void usage(const char *prog) {
    printf("Usage: %s <command> [args]\n", prog);
    printf("Commands:\n");
    printf("  inspect <manifold.pos>          Show Level 11 roots (Legacy)\n");
    printf("  inspect12 <manifold.pos>        Show Level 12 roots (Mind/Body/Sheaf)\n");
    printf("  post_lore <manifold.pos> <offset> <text>  Write lore to manifold\n");
    printf("  cornu <epoch>                   Show Cornu Spiral (Internal Development)\n");
}

int main(int argc, char **argv) {
    if (argc < 2) { usage(argv[0]); return 1; }
    const char *cmd = argv[1];

    if (strcmp(cmd, "inspect") == 0) {
        if (argc < 3) { usage(argv[0]); return 1; }
        return handle_inspect(argv[2]);
    } 
    else if (strcmp(cmd, "inspect12") == 0) {
        if (argc < 3) { usage(argv[0]); return 1; }
        return handle_inspect12(argv[2]);
    }
    else if (strcmp(cmd, "post_lore") == 0) {
        if (argc < 5) { usage(argv[0]); return 1; }
        return handle_post_lore(argv[2], atoi(argv[3]), argv[4]);
    }
    else if (strcmp(cmd, "cornu") == 0) {
        if (argc < 3) { usage(argv[0]); return 1; }
        float u = atof(argv[2]);
        printf("Epoch %f -> Cornu(C: %f, S: %f)\n", (double)u, (double)internal_fresnel_c(u), (double)internal_fresnel_s(u));
    }
    else {
        usage(argv[0]);
        return 1;
    }
    return 0;
}
