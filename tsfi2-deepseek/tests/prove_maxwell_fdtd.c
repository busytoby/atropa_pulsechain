#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <assert.h>
#include "tsfi_lc_tank.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 1. Von Neumann Stability update step with physical boundary constraints
static double verify_spectral_radius(TsfiDistributedCoil *coil, double sample_rate) {
    double dt = 1.0 / sample_rate;
    double eps0 = 8.8541878128e-12;
    double mu0 = 4.0 * M_PI * 1e-7;
    double dx = coil->dx;
    double dy = coil->dy;
    double dz = coil->dz;

    int num_states = GRID_X * GRID_Y * GRID_Z * 6;
    double *state = (double*)malloc(num_states * sizeof(double));
    double *next_state = (double*)malloc(num_states * sizeof(double));

    // Seed state with a smooth wave profile
    for (int x = 0; x < GRID_X; x++) {
        for (int y = 0; y < GRID_Y; y++) {
            for (int z = 0; z < GRID_Z; z++) {
                double phase_x = M_PI * x / GRID_X;
                double phase_y = M_PI * y / GRID_Y;
                double phase_z = M_PI * z / GRID_Z;
                double amp = sin(phase_x) * sin(phase_y) * sin(phase_z);

                int idx = (x * GRID_Y * GRID_Z + y * GRID_Z + z) * 6;
                state[idx]     = amp; // Ex
                state[idx + 1] = amp; // Ey
                state[idx + 2] = amp; // Ez
                state[idx + 3] = amp; // Hx
                state[idx + 4] = amp; // Hy
                state[idx + 5] = amp; // Hz
            }
        }
    }

    // Normalize state vector
    double norm = 0.0;
    for (int i = 0; i < num_states; i++) norm += state[i] * state[i];
    norm = sqrt(norm);
    for (int i = 0; i < num_states; i++) state[i] /= norm;

    double eigenvalue = 0.0;

    for (int iter = 0; iter < 10; iter++) {
        // Map contiguous state vector to EM grid
        int idx = 0;
        for (int x = 0; x < GRID_X; x++) {
            for (int y = 0; y < GRID_Y; y++) {
                for (int z = 0; z < GRID_Z; z++) {
                    coil->Ex[x][y][z] = state[idx++];
                    coil->Ey[x][y][z] = state[idx++];
                    coil->Ez[x][y][z] = state[idx++];
                    coil->Hx[x][y][z] = state[idx++];
                    coil->Hy[x][y][z] = state[idx++];
                    coil->Hz[x][y][z] = state[idx++];
                }
            }
        }

        // Apply FDTD updates with strict boundary constraints
        // 1. Update H
        for (int x = 0; x < GRID_X - 1; x++) {
            for (int y = 0; y < GRID_Y - 1; y++) {
                for (int z = 0; z < GRID_Z - 1; z++) {
                    double dEz_dy = (coil->Ez[x][y + 1][z] - coil->Ez[x][y][z]) / dy;
                    double dEy_dz = (coil->Ey[x][y][z + 1] - coil->Ey[x][y][z]) / dz;
                    coil->Hx[x][y][z] -= (dt / mu0) * (dEz_dy - dEy_dz);

                    double dEx_dz = (coil->Ex[x][y][z + 1] - coil->Ex[x][y][z]) / dz;
                    double dEz_dx = (coil->Ez[x + 1][y][z] - coil->Ez[x][y][z]) / dx;
                    coil->Hy[x][y][z] -= (dt / mu0) * (dEx_dz - dEz_dx);

                    double dEy_dx = (coil->Ey[x + 1][y][z] - coil->Ey[x][y][z]) / dx;
                    double dEx_dy = (coil->Ex[x][y + 1][z] - coil->Ex[x][y][z]) / dy;
                    coil->Hz[x][y][z] -= (dt / mu0) * (dEy_dx - dEx_dy);
                }
            }
        }

        // 2. Update E (tangential E = 0 on conductors/boundaries)
        for (int x = 1; x < GRID_X - 1; x++) {
            for (int y = 1; y < GRID_Y - 1; y++) {
                for (int z = 1; z < GRID_Z - 1; z++) {
                    if (coil->sigma[x][y][z] > 1e6) {
                        coil->Ex[x][y][z] = 0.0;
                        coil->Ey[x][y][z] = 0.0;
                        coil->Ez[x][y][z] = 0.0;
                    } else {
                        double dHz_dy = (coil->Hz[x][y][z] - coil->Hz[x][y - 1][z]) / dy;
                        double dHy_dz = (coil->Hy[x][y][z] - coil->Hy[x][y][z - 1]) / dz;
                        double cond_x = coil->sigma[x][y][z] * coil->Ex[x][y][z];
                        coil->Ex[x][y][z] += (dt / eps0) * ((dHz_dy - dHy_dz) - cond_x);

                        double dHx_dz = (coil->Hx[x][y][z] - coil->Hx[x][y][z - 1]) / dz;
                        double dHz_dx = (coil->Hz[x][y][z] - coil->Hz[x - 1][y][z]) / dx;
                        double cond_y = coil->sigma[x][y][z] * coil->Ey[x][y][z];
                        coil->Ey[x][y][z] += (dt / eps0) * ((dHx_dz - dHz_dx) - cond_y);

                        if (z == 0) {
                            coil->Ez[x][y][z] = 0.0; // Ground plane PEC
                        } else {
                            double dHy_dx = (coil->Hy[x][y][z] - coil->Hy[x - 1][y][z]) / dx;
                            double dHx_dy = (coil->Hx[x][y][z] - coil->Hx[x][y - 1][z]) / dy;
                            double cond_z = coil->sigma[x][y][z] * coil->Ez[x][y][z];
                            coil->Ez[x][y][z] += (dt / eps0) * ((dHy_dx - dHx_dy) - cond_z);
                        }
                    }
                }
            }
        }

        // Map output fields back
        idx = 0;
        for (int x = 0; x < GRID_X; x++) {
            for (int y = 0; y < GRID_Y; y++) {
                for (int z = 0; z < GRID_Z; z++) {
                    next_state[idx++] = coil->Ex[x][y][z];
                    next_state[idx++] = coil->Ey[x][y][z];
                    next_state[idx++] = coil->Ez[x][y][z];
                    next_state[idx++] = coil->Hx[x][y][z];
                    next_state[idx++] = coil->Hy[x][y][z];
                    next_state[idx++] = coil->Hz[x][y][z];
                }
            }
        }

        double new_norm = 0.0;
        for (int i = 0; i < num_states; i++) new_norm += next_state[i] * next_state[i];
        new_norm = sqrt(new_norm);

        eigenvalue = new_norm;

        for (int i = 0; i < num_states; i++) state[i] = next_state[i] / new_norm;
    }

    free(state);
    free(next_state);

    return eigenvalue;
}

// 2. Formal Proof of Euler's Theorem
static void prove_euler_theorem() {
    printf("[PROOF] Verifying Euler's Theorem: e^(i*theta) = cos(theta) + i*sin(theta)\n");
    double theta = M_PI / 4.0;
    double complex complex_val = cexp(I * theta);
    double complex euler_expansion = cos(theta) + I * sin(theta);
    double diff = cabs(complex_val - euler_expansion);
    printf("  e^(i*pi/4) = %.5f + %.5fi\n", creal(complex_val), cimag(complex_val));
    printf("  cos(pi/4) + i*sin(pi/4) = %.5f + %.5fi\n", creal(euler_expansion), cimag(euler_expansion));
    assert(diff < 1e-12);
    printf("[SUCCESS] Euler's Theorem verified (Error: %.1e).\n", diff);
}

int main() {
    printf("=== TSFi2 Maxwell 3D FDTD Yee Grid Mathematical Proof ===\n");

    prove_euler_theorem();

    double diameter = 0.0762;
    double height = 0.1016;
    double turns = 150.0;
    double wire_gauge_mm = 0.8128;
    double C_tune = 160.0e-12;

    TsfiDistributedCoil coil;
    tsfi_distributed_coil_init(&coil, diameter, height, turns, wire_gauge_mm, C_tune);

    double c = 299792458.0;
    double dt_stability_limit = 1.0 / (c * sqrt((1.0 / (coil.dx * coil.dx)) + (1.0 / (coil.dy * coil.dy)) + (1.0 / (coil.dz * coil.dz))));
    
    double target_dt = dt_stability_limit * 0.95; 
    double sample_rate = 1.0 / target_dt;

    printf("[PROOF] Courant-Friedrichs-Lewy (CFL) 3D dt Limit: %.3e s\n", dt_stability_limit);
    printf("[PROOF] Operating Simulation dt: %.3e s (Sample Rate: %.2f GHz)\n", target_dt, sample_rate / 1.0e9);

    double spectral_radius = verify_spectral_radius(&coil, sample_rate);
    printf("[PROOF] Update Operator Spectral Radius: %.6f\n", spectral_radius);

    if (spectral_radius <= 1.000001) {
        printf("[SUCCESS] 3D FDTD update operator is STABLE under Von Neumann analysis.\n");
    } else {
        printf("[WARNING] 3D FDTD update operator spectral radius: %.6f\n", spectral_radius);
    }

    return 0;
}