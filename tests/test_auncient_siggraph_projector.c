#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MOTZKIN_PRIME 953467954114363ULL
#define NUM_POINTS 200

// Tripartite Register Layout of SHA Fa structure
typedef struct {
    uint64_t base_reg;       // Base reference alignment register
    uint64_t signal_reg;     // Core wave carrier signal register
    uint64_t channel_reg;    // X-frequency control channel register
    uint64_t dynamo_reg;     // Y-frequency control dynamo register
    uint64_t foundation_reg; // Z-frequency control foundation register
    uint64_t identity_reg;   // Symmetry scaling boundary register
    uint64_t element_reg;    // Density element layout register
    uint64_t chin_reg;       // Lower boundary asymmetry register
} sha_fa_registers_t;

// Projected 3D coordinate point
typedef struct {
    double x;
    double y;
    double z;
} point_3d_t;

// Quadtree node structure for spatial partition representation
typedef struct quadtree_node {
    double min_x, min_y;
    double max_x, max_y;
    point_3d_t point;
    bool has_point;
    struct quadtree_node *children[4];
} quadtree_node_t;

// Modular exponentiation helper
static uint64_t mod_exp(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t res = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1) {
            res = (__uint128_t)res * base % mod;
        }
        base = (__uint128_t)base * base % mod;
        exp /= 2;
    }
    return res;
}

// Initialize SHA Fa registers mapping
static void init_sha_fa_registers(sha_fa_registers_t *regs) {
    regs->base_reg = 2; // Seed Base
    regs->signal_reg = 13; // Wave carrier signal strength
    regs->identity_reg = 7; // EDO-22 octave boundary

    // Derived via modular mathematical equations:
    regs->channel_reg = mod_exp(regs->base_reg, regs->signal_reg, MOTZKIN_PRIME); // Tune: Base^Signal mod Prime
    regs->foundation_reg = mod_exp(regs->base_reg, regs->identity_reg, MOTZKIN_PRIME); // Conify: Base^Identity mod Prime
    
    regs->element_reg = 22; // Octave spacing density
    regs->dynamo_reg = mod_exp(regs->base_reg, regs->signal_reg, regs->element_reg); // Bond: Base^Signal mod Element
    
    // Chin asymmetry computation: Beta + Eta mod MotzkinPrime
    uint64_t beta = 500;
    uint64_t eta = 120;
    regs->chin_reg = (beta + eta) % MOTZKIN_PRIME;
}

// Coordinate mapping generator implementing Lissajous transformations
static void generate_projected_trajectory(const sha_fa_registers_t *regs, point_3d_t *points, int count) {
    double freq_x = (double)(regs->channel_reg % 10) + 1.0;
    double freq_y = (double)(regs->dynamo_reg % 10) + 1.0;
    double freq_z = (double)(regs->foundation_reg % 10) + 1.0;

    // Phase angle derived from Base reference
    double phase_w = (double)(regs->base_reg % 100) * 0.0628;

    // Chin hemispheric factor
    double chin_factor = (double)(regs->chin_reg) / (double)MOTZKIN_PRIME;

    for (int i = 0; i < count; i++) {
        double t = (double)i * 0.0628; // Step increment

        // 1. Compute coordinate trajectories
        points[i].x = sin(freq_x * t + phase_w);
        points[i].y = sin(freq_y * t);
        points[i].z = sin(freq_z * t);

        // 2. Modulate lower boundary coordinate layout using Chin asymmetry parameter
        if (points[i].y < 0.0) {
            points[i].y *= (1.0 + chin_factor * 0.5); // Warp Y negative coordinates to shape floor
        }
    }
}

// Allocates a fresh quadtree node representing screen mapping layout
static quadtree_node_t *create_quadtree_node(double min_x, double min_y, double max_x, double max_y) {
    quadtree_node_t *node = (quadtree_node_t *)malloc(sizeof(quadtree_node_t));
    node->min_x = min_x;
    node->min_y = min_y;
    node->max_x = max_x;
    node->max_y = max_y;
    node->has_point = false;
    for (int i = 0; i < 4; i++) {
        node->children[i] = NULL;
    }
    return node;
}

// Insert coordinate projection point into the quadtree structure
static bool insert_quadtree(quadtree_node_t *node, point_3d_t pt) {
    if (pt.x < node->min_x || pt.x > node->max_x || pt.y < node->min_y || pt.y > node->max_y) {
        return false;
    }

    if (!node->has_point) {
        node->point = pt;
        node->has_point = true;
        return true;
    }

    double mid_x = (node->min_x + node->max_x) / 2.0;
    double mid_y = (node->min_y + node->max_y) / 2.0;

    // Quadrants definition
    if (node->children[0] == NULL) {
        node->children[0] = create_quadtree_node(node->min_x, node->min_y, mid_x, mid_y);
        node->children[1] = create_quadtree_node(mid_x, node->min_y, node->max_x, mid_y);
        node->children[2] = create_quadtree_node(node->min_x, mid_y, mid_x, node->max_y);
        node->children[3] = create_quadtree_node(mid_x, mid_y, node->max_x, node->max_y);
    }

    for (int i = 0; i < 4; i++) {
        if (insert_quadtree(node->children[i], pt)) {
            return true;
        }
    }

    return false;
}

// Recursively write quadtree node layout directly to binary .dat.bin stream
static void serialize_quadtree_node(const quadtree_node_t *node, FILE *f) {
    if (!node) {
        uint8_t null_flag = 0;
        fwrite(&null_flag, sizeof(uint8_t), 1, f);
        return;
    }

    uint8_t active_flag = 1;
    fwrite(&active_flag, sizeof(uint8_t), 1, f);
    fwrite(&(node->min_x), sizeof(double), 1, f);
    fwrite(&(node->min_y), sizeof(double), 1, f);
    fwrite(&(node->max_x), sizeof(double), 1, f);
    fwrite(&(node->max_y), sizeof(double), 1, f);

    uint8_t point_flag = node->has_point ? 1 : 0;
    fwrite(&point_flag, sizeof(uint8_t), 1, f);
    if (node->has_point) {
        fwrite(&(node->point.x), sizeof(double), 1, f);
        fwrite(&(node->point.y), sizeof(double), 1, f);
        fwrite(&(node->point.z), sizeof(double), 1, f);
    }

    for (int i = 0; i < 4; i++) {
        serialize_quadtree_node(node->children[i], f);
    }
}

// Clean up quadtree memory allocations
static void free_quadtree(quadtree_node_t *node) {
    if (!node) return;
    for (int i = 0; i < 4; i++) {
        free_quadtree(node->children[i]);
    }
    free(node);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT SIGGRAPH 3D REGISTRY PROJECTION GENERATOR\n");
    printf("=============================================================\n");
    fflush(stdout);

    // 1. Initialize VM register configurations
    sha_fa_registers_t regs;
    init_sha_fa_registers(&regs);
    printf("[GEOMETRY] Registers configured:\n");
    printf("   Channel Frequency (X-Axis):    %lu\n", regs.channel_reg);
    printf("   Dynamo Frequency (Y-Axis):     %lu\n", regs.dynamo_reg);
    printf("   Foundation Frequency (Z-Axis): %lu\n", regs.foundation_reg);
    printf("   Chin Lower Asymmetry Boundary: %lu\n", regs.chin_reg);
    fflush(stdout);

    // 2. Generate point cloud sequence
    point_3d_t points[NUM_POINTS];
    generate_projected_trajectory(&regs, points, NUM_POINTS);
    printf("   ✓ 3D Coordinate trajectory computed (%d points).\n", NUM_POINTS);
    fflush(stdout);

    // 3. Build spatial quadtree representation
    quadtree_node_t *root = create_quadtree_node(-2.0, -2.0, 2.0, 2.0);
    for (int i = 0; i < NUM_POINTS; i++) {
        insert_quadtree(root, points[i]);
    }
    printf("   ✓ Spatial Quadtree successfully mapped.\n");
    fflush(stdout);

    // 4. Serialize to .dat.bin file formatting (Rule 13 compliance)
    const char *out_path = "tests/siggraph_geometry.dat.bin";
    FILE *out = fopen(out_path, "wb");
    if (!out) {
        free_quadtree(root);
        return 1;
    }

    uint32_t header_sig = 0x51545245; // "QTRE"
    fwrite(&header_sig, sizeof(uint32_t), 1, out);
    serialize_quadtree_node(root, out);
    fclose(out);

    printf("   ✓ Quadtree media data written directly to: %s\n", out_path);
    fflush(stdout);

    free_quadtree(root);
    printf("=============================================================\n");
    printf("SIGGRAPH PROJECTION GENERATION COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);

    return 0;
}
