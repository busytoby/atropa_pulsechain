#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Auncient spelling check: verified
// Ensure all source files remain strictly under 68KB limit.

typedef struct {
    double x, y, z;
    double px, py, pz;
    double mass;
    bool fixed;
} PoppyJoint;

typedef struct {
    int p1;
    int p2;
    double rest_length;
    double stiffness;
} PoppySpring;

#define NUM_JOINTS 5  // 1 central pod (0) + 4 radiating petals (1, 2, 3, 4)
#define NUM_SPRINGS 4 // Springs connecting pod to each petal

static PoppyJoint joints[NUM_JOINTS];
static PoppySpring springs[NUM_SPRINGS];

static void init_poppy_verlet() {
    // 0: Central Pod (center of the flower)
    joints[0] = (PoppyJoint){ .x = 128.0, .y = 128.0, .z = 10.0, .px = 128.0, .py = 128.0, .pz = 10.0, .mass = 2.0, .fixed = true };

    // 1: Top Petal
    joints[1] = (PoppyJoint){ .x = 128.0, .y = 88.0, .z = 15.0, .px = 128.0, .py = 88.0, .pz = 15.0, .mass = 1.0, .fixed = false };
    // 2: Bottom Petal
    joints[2] = (PoppyJoint){ .x = 128.0, .y = 168.0, .z = 5.0, .px = 128.0, .py = 168.0, .pz = 5.0, .mass = 1.0, .fixed = false };
    // 3: Left Petal
    joints[3] = (PoppyJoint){ .x = 88.0, .y = 128.0, .z = 12.0, .px = 88.0, .py = 128.0, .pz = 12.0, .mass = 1.0, .fixed = false };
    // 4: Right Petal
    joints[4] = (PoppyJoint){ .x = 168.0, .y = 128.0, .z = 8.0, .px = 168.0, .py = 128.0, .pz = 8.0, .mass = 1.0, .fixed = false };

    // Spring links
    for (int i = 0; i < NUM_SPRINGS; i++) {
        springs[i] = (PoppySpring){
            .p1 = 0,
            .p2 = i + 1,
            .rest_length = 40.0,
            .stiffness = 0.45
        };
    }
}

static void verlet_step(double dt, double wind_x, double gravity_y) {
    double decay = 0.98;

    // 1. Position update
    for (int i = 0; i < NUM_JOINTS; i++) {
        if (joints[i].fixed) continue;

        double tx = joints[i].x;
        double ty = joints[i].y;
        double tz = joints[i].z;

        // Apply forces (Wind & Gravity)
        double ax = wind_x / joints[i].mass;
        double ay = gravity_y / joints[i].mass;

        joints[i].x += (joints[i].x - joints[i].px) * decay + ax * dt * dt;
        joints[i].y += (joints[i].y - joints[i].py) * decay + ay * dt * dt;
        joints[i].z += (joints[i].z - joints[i].pz) * decay;

        joints[i].px = tx;
        joints[i].py = ty;
        joints[i].pz = tz;
    }

    // 2. Resolve springs constraints (2 iterations for convergence)
    for (int iter = 0; iter < 2; iter++) {
        for (int i = 0; i < NUM_SPRINGS; i++) {
            PoppySpring *s = &springs[i];
            PoppyJoint *j1 = &joints[s->p1];
            PoppyJoint *j2 = &joints[s->p2];

            double dx = j2->x - j1->x;
            double dy = j2->y - j1->y;
            double dz = j2->z - j1->z;
            double dist = sqrt(dx*dx + dy*dy + dz*dz);
            if (dist == 0.0) continue;

            double diff = s->rest_length - dist;
            double percent = (diff / dist) * s->stiffness * 0.5;

            double offset_x = dx * percent;
            double offset_y = dy * percent;
            double offset_z = dz * percent;

            if (!j1->fixed) {
                j1->x -= offset_x;
                j1->y -= offset_y;
                j1->z -= offset_z;
            }
            if (!j2->fixed) {
                j2->x += offset_x;
                j2->y += offset_y;
                j2->z += offset_z;
            }
        }
    }
}

int main() {
    printf("=== Auncient Verlet Soft-Body Poppy Simulation ===\n");
    init_poppy_verlet();

    double dt = 0.1;
    // Simulate 100 frames of wind gusts and gravity
    for (int frame = 0; frame < 100; frame++) {
        double wind = 1.2 * sin((double)frame * 0.15);
        double gravity = 0.25; // Constant downward pull on petals
        
        verlet_step(dt, wind, gravity);
        
        if (frame % 20 == 0) {
            printf("[Frame %02d] Top Petal Y: %.3f | Right Petal X: %.3f | Dist to Pod: %.3f\n",
                   frame, joints[1].y, joints[4].x,
                   sqrt(pow(joints[1].x - joints[0].x, 2) + pow(joints[1].y - joints[0].y, 2)));
        }
    }

    // Verify stability
    double final_dist = sqrt(pow(joints[1].x - joints[0].x, 2) + pow(joints[1].y - joints[0].y, 2));
    if (final_dist > 30.0 && final_dist < 50.0) {
        printf("[SUCCESS] Verlet poppy flower constraints converged stably at %.3f pixels.\n", final_dist);
        return 0;
    } else {
        printf("[ERROR] Verlet constraints diverged: %.3f pixels.\n", final_dist);
        return 1;
    }
}
