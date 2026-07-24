#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MOTZKIN_PRIME 953467954114363ULL
#define NUM_PHYSICS_POINTS 50
#define WIDTH 400
#define HEIGHT 400

typedef struct {
    double x, y, z;
} point_3d_t;

typedef struct {
    double x, y, z;
    double px, py, pz; // Previous coordinates for Verlet integration
    double ox, oy, oz; // Original target anchor points
} mass_node_t;


// Spring constraint between adjacent nodes
typedef struct {
    int node_a;
    int node_b;
    double rest_length;
} spring_link_t;

// Simulated Field-Effect Transistor (FET) discharge parameters
typedef struct {
    double charge_level;    // Current electric field charge (degrades during discharge)
    double spring_k;        // Spring stiffness coefficient
    double damping;         // Velocity damping factor
} fet_discharge_state_t;

static double distance_3d(point_3d_t a, point_3d_t b) {
    return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) + (a.z - b.z) * (a.z - b.z));
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT SIGGRAPH VERLET FET DISCHARGE SIMULATOR\n");
    printf("=============================================================\n");
    fflush(stdout);

    // 1. Initialize mass nodes along Lissajous paths (representing low-level transistor layouts)
    mass_node_t nodes[NUM_PHYSICS_POINTS];
    double freq_x = 3.0;
    double freq_y = 2.0;

    for (int i = 0; i < NUM_PHYSICS_POINTS; i++) {
        double t = (double)i * (2.0 * 3.1415926535 / NUM_PHYSICS_POINTS);
        double x = cos(freq_x * t);
        double y = sin(freq_y * t);
        double z = sin(t);

        nodes[i].x = x;
        nodes[i].y = y;
        nodes[i].z = z;
        nodes[i].px = x;
        nodes[i].py = y;
        nodes[i].pz = z;
        nodes[i].ox = x;
        nodes[i].oy = y;
        nodes[i].oz = z;
    }
    printf("   ✓ Physical mass nodes initialized along Lissajous trajectory.\n");
    fflush(stdout);

    // 2. Configure Mass-Spring structural links
    spring_link_t links[NUM_PHYSICS_POINTS];
    for (int i = 0; i < NUM_PHYSICS_POINTS; i++) {
        int next = (i + 1) % NUM_PHYSICS_POINTS;
        point_3d_t pt_a = { .x = nodes[i].x, .y = nodes[i].y, .z = nodes[i].z };
        point_3d_t pt_b = { .x = nodes[next].x, .y = nodes[next].y, .z = nodes[next].z };
        links[i].node_a = i;
        links[i].node_b = next;
        links[i].rest_length = distance_3d(pt_a, pt_b);
    }
    printf("   ✓ structural mass-spring networks configured.\n");
    fflush(stdout);

    // 3. Execute simulated FET discharge cycle (exponential decay of charge envelope)
    fet_discharge_state_t fet = {
        .charge_level = 1.0,  // Initial fully-charged state
        .spring_k = 0.4,       // Elastic spring response
        .damping = 0.98        // Friction coefficient
    };

    double dt = 0.05;
    printf("[SIMULATION] Running 100 steps of Verlet FET discharge cycle...\n");
    fflush(stdout);

    for (int step = 0; step < 100; step++) {
        // Discharge charge decay
        fet.charge_level *= 0.95;

        // Apply Verlet Integration to all nodes
        for (int i = 0; i < NUM_PHYSICS_POINTS; i++) {
            double temp_x = nodes[i].x;
            double temp_y = nodes[i].y;
            double temp_z = nodes[i].z;

            // External forces: electrostatic field pull degrades with charge level decay
            double force_x = (nodes[i].ox - nodes[i].x) * fet.spring_k * fet.charge_level;
            double force_y = (nodes[i].oy - nodes[i].y) * fet.spring_k * fet.charge_level - (1.0 - fet.charge_level) * 0.2; // Gravity pull during discharge
            double force_z = (nodes[i].oz - nodes[i].z) * fet.spring_k * fet.charge_level;

            nodes[i].x = nodes[i].x + (nodes[i].x - nodes[i].px) * fet.damping + force_x * dt * dt;
            nodes[i].y = nodes[i].y + (nodes[i].y - nodes[i].py) * fet.damping + force_y * dt * dt;
            nodes[i].z = nodes[i].z + (nodes[i].z - nodes[i].pz) * fet.damping + force_z * dt * dt;

            nodes[i].px = temp_x;
            nodes[i].py = temp_y;
            nodes[i].pz = temp_z;
        }

        // Enforce structural spring links length constraints (10 iterations)
        for (int iter = 0; iter < 10; iter++) {
            for (int i = 0; i < NUM_PHYSICS_POINTS; i++) {
                int na = links[i].node_a;
                int nb = links[i].node_b;

                double dx = nodes[nb].x - nodes[na].x;
                double dy = nodes[nb].y - nodes[na].y;
                double dz = nodes[nb].z - nodes[na].z;
                double len = sqrt(dx*dx + dy*dy + dz*dz);
                if (len < 0.001) len = 0.001;

                double diff = (links[i].rest_length - len) / len * 0.5;
                double disp_x = dx * diff;
                double disp_y = dy * diff;
                double disp_z = dz * diff;

                nodes[na].x -= disp_x;
                nodes[na].y -= disp_y;
                nodes[na].z -= disp_z;

                nodes[nb].x += disp_x;
                nodes[nb].y += disp_y;
                nodes[nb].z += disp_z;

            }
        }
    }

    printf("   ✓ simulated soft-body deformation finalized. Charge decayed: %.4f.\n", fet.charge_level);
    fflush(stdout);

    // Save final physics projection coordinates to disk
    const char *physics_log_path = "tests/siggraph_physics.dat.bin";
    FILE *out = fopen(physics_log_path, "wb");
    if (out) {
        uint32_t header = 0x50485953; // "PHYS"
        fwrite(&header, sizeof(uint32_t), 1, out);
        for (int i = 0; i < NUM_PHYSICS_POINTS; i++) {
            fwrite(&(nodes[i].x), sizeof(double), 1, out);
            fwrite(&(nodes[i].y), sizeof(double), 1, out);
            fwrite(&(nodes[i].z), sizeof(double), 1, out);
        }
        fclose(out);
    }
    printf("   ✓ Deformed physical coordinates written to: %s\n", physics_log_path);
    fflush(stdout);

    printf("=============================================================\n");
    printf("VERLET DISCHARGE SIMULATOR COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);

    return 0;
}
